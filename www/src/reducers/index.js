import { combineReducers } from 'redux'
import { routerReducer as routing } from 'react-router-redux'

import { APP_ERROR, APP_RESET, MIDI_READY, MIDI_SERVER_BOOTLOADER_IMAGES_LOADED } from '../actions'
import { reducers as fwUpdate } from '../fwUpdate'

const INITIAL_MIDI_STATE = {
    setupRequired: true,
}

const midi = (state = INITIAL_MIDI_STATE, action) => {
    switch (action.type) {
        case APP_RESET:
            return {
                ...state,
                setupRequired: true,
                dev: null,
                inputName: action.resetup ? null : state.inputName,
                outputName: action.resetup ? null : state.outputName,
            }

        case MIDI_READY:
            return {
                ...state,
                setupRequired: false,
                inputName: action.inputName,
                outputName: action.outputName,
                dev: action.dev,
                devInfo: action.devInfo,
            }

        case MIDI_SERVER_BOOTLOADER_IMAGES_LOADED:
            return {
                ...state,
                serverBlImgs: action.blImgs,
            }

        default:
          return state
    }
}



const INITIAL_APP_STATE = {
    setupRequired: true,
}

const app = (state = INITIAL_APP_STATE, action) => {
    switch (action.type) {
        case APP_ERROR:
            return {
                ...state,
                error: action.error,
            }

        default:
            return state
    }
}


const rootReducer = combineReducers({
    midi,
    app,
    fwUpdate,
    routing
})

export default rootReducer
