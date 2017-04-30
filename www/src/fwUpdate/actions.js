import Struct from 'struct'
import { midiSetup } from '../actions'

export const PREPARE = 'FW_UPDATE_PREPARE'
export const prepare = () => {
    return {
        type: PREPARE,
    }
}


export const CANCEL = 'FW_UPDATE_CANCEL'
export const cancel = () => {
    return {
        type: CANCEL,
    }
}

export const UPDATE_STATUS_MSG = 'FW_UPDATE_UPDATE_STATUS_MSG'
export const update_status_msg = (statusMsg) => {
    return {
        type: UPDATE_STATUS_MSG,
        statusMsg,
    }
}

export const STARTING = 'FW_UPDATE_STARTING'
export const CHUNK_SENT = 'FW_UPDATE_CHUNK_SENT'
export const DONE = 'FW_UPDATE_DONE'
export const start = (fileName = null, buf = null) => {
    return (dispatch, getState) => {
        let fwImg = null
        if (fileName && buf) {
            fwImg = {
                fileName,
                buf,
            }

            const state = getState()
            const dev = state.midi.dev

            const chunkSent = (chunkSize) => {
                dispatch(update_status_msg('Sending firmware...'))
                dispatch({
                    type: CHUNK_SENT,
                    chunkSize,
                })
            }
            const done = () => {
                // Hack to give device time to re-enumerate
                // TODO poll
                dispatch(update_status_msg('Waiting for re-enumeration...'))
                setTimeout(() => {
                    dispatch(update_status_msg('Done :)'))
                    dispatch({
                        type: DONE,
                    })
                }, 5000)
            }

            let start_bootloader;
            if (!state.midi.devInfo.is_bootloader) {
                // If we're not a bootloader device, try and send a start midi command.
                // If it works the device will restart in bootloader mode
                dispatch(update_status_msg('Starting bootloader...'))

                dev.callSysExCmd(
                   dev.SYSEX_CMD.USER + 0, // ab_main_bl_midi_sysex_cmd_bl_start
                )

                start_bootloader = new Promise((resolve, reject) => {
                    setTimeout(() => {
                        dispatch(midiSetup())

                        let retries = 0
                        const retry = () => {
                            if (getState().midi.devInfo.is_bootloader) {
                                dispatch(update_status_msg('Bootloader started!'))
                                resolve()
                            }

                            ++retries
                            if (retries === 20) {
                                reject(new Error('Timeout while waiting for a bootloader device'))
                            } else {
                                setTimeout(retry, 100)
                            }
                        }
                        setTimeout(retry, 100)
                    }, 2000) // time for USB re-enumeration
                })
            } else {
                start_bootloader = new Promise((resolve, reject) => { resolve() })
            }

            start_bootloader.then(() => {
                // Get the magic word from the image
                const magic_word_rdr = Struct().word32Ule('magic')
                magic_word_rdr.setBuffer(buf);
                const magic_word = magic_word_rdr.fields.magic;

                let bl;
                switch (magic_word) {
                    // Single bootloader image
                    case 0xc0de1337:
                        bl = getState().midi.dev.getBootloader();
                        break;

                    // Multi-img
                    case 0x4d4c5431:
                        bl = getState().midi.dev.getSerialFlashBootloader({updateSelfWhenDone: true});
                        break;

                    default:
                        dispatch(update_status_msg('Unknown file magic 0x' + magic_word.toString(16)));
                        return;
                }

                bl.start(
                    state.midi.devInfo.dev_id,
                    buf,
                ).then(() => {
                    bl.sendNextChunk(chunkSent, done)
                })
            })
        }
        return dispatch({
            type: STARTING,
            fwImg: fwImg,
        })
    }
}
