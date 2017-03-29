import { PREPARE, CANCEL, STARTING, CHUNK_SENT, DONE } from './actions'

const INITIAL_STATE = {
    modalShown: false,
    busy: false,
}

export default (state = INITIAL_STATE, action) => {
    switch (action.type) {
        case PREPARE:
            return {
                ...state,
                modalShown: true,
            }

        case CANCEL:
            if (state.busy) {
                return state
            }

            return {
                ...state,
                modalShown: false,
            }

        case STARTING:
            return {
                ...state,
                modalShown: true,
                busy: true,
                fwImg: action.fwImg,
                bytesSent: 0,
                done: false,
            }

        case CHUNK_SENT:
            return {
                ...state,
                bytesSent: state.bytesSent + action.chunkSize,
            }

        case DONE:
            return {
                ...state,
                busy: false,
                done: true,
            }

        default:
            return state
    }
}
