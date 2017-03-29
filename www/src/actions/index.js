import { openMidiDevice } from '../midi'


export const APP_RESET = 'APP_RESET'
export const APP_ERROR = 'APP_ERROR'

export const MIDI_READY = 'MIDI_READY'

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


