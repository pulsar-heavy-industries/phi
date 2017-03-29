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
            const bl = state.midi.dev.getBootloader()

            const chunkSent = (chunkSize) => {
                dispatch({
                    type: CHUNK_SENT,
                    chunkSize,
                })
            }
            const done = () => {
                dispatch({
                    type: DONE,
                })
            }

            bl.start(
                state.midi.devInfo.dev_id,
                buf,
            ).then(() => {
                bl.sendNextChunk(chunkSent, done)
            })
        }
        return dispatch({
            type: STARTING,
            fwImg: fwImg,
        })
    }
}
