import { openMidiDevice } from '../midi'


/******************************************************************************
 * App
 *****************************************************************************/

export const APP_RESET = 'APP_RESET'
export const APP_ERROR = 'APP_ERROR'

export const appShowError = (error) => {
    return {
        type: APP_ERROR,
        error,
    }
}

export const appHideError = () => {
    return {
        type: APP_ERROR,
        error: null,
    }
}

export const appReset = (resetup = false) => {
    return {
        type: APP_RESET,
        resetup: resetup,
    }
}

/******************************************************************************
 * MIDI
 *****************************************************************************/

export const MIDI_READY = 'MIDI_READY'
export const MIDI_SERVER_BOOTLOADER_IMAGES_LOADED = 'MIDI_SERVER_BOOTLOADER_IMAGES_LOADED'

export const midiReady = (inputName, outputName, dev, devInfo)  => {
    return {
        type: MIDI_READY,
        dev,
        inputName,
        outputName,
        devInfo,
    }
}

export const midiSetup = (inputName, outputName) => {
    return (dispatch, getState) => {
        const midi = getState().midi
        dispatch(appReset())

        inputName = inputName || midi.inputName
        outputName = outputName || midi.outputName

        openMidiDevice(inputName, outputName)
            .then((dev) => {
                dev.open()
                dev.getDevInfo().then((devInfo) => {
                    return dispatch(midiReady(inputName, outputName, dev, devInfo))
                }).catch((err) => {
                    dispatch(appReset(true))
                    dispatch(appShowError(err))
                })
            })
            .catch((err) => {
                dispatch(appReset(true))
                dispatch(appShowError(err))
            })
    }
}

import { BaseMidiBootloaderImg } from '../midi'
import _fw_cenx4 from '../../../cenx4/firmware/app/build/bl-ch.bin'
import _fw_cenx4_multi from '../../../cenx4/firmware/app/build/multi.img'
import _fw_narvi from '../../../narvi/firmware/app/build/bl-ch.bin'
export const midiGetServerBootloaderImages = () => {
    return (dispatch, getState) => {
        Promise.all([
            _fw_cenx4,
            _fw_cenx4_multi,
            _fw_narvi,
        ].map(url => fetch(url)
            .then(resp => resp.blob())
            .then(blob => new Promise((resolve, reject) => {
                const reader = new FileReader()
                reader.addEventListener('loadend', () => {
                    const buf = Array.from(new Uint8Array(reader.result))
                    resolve(buf)
                })
                reader.readAsArrayBuffer(blob)
            }))
            .then(buf => BaseMidiBootloaderImg.from_buf(buf))
        )).then((blImgs) => {
            dispatch({
                type: MIDI_SERVER_BOOTLOADER_IMAGES_LOADED,
                blImgs,
            })
        })
    }
}
