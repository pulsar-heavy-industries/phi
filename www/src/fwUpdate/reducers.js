import { PREPARE, UPDATE_STATUS_MSG, CANCEL, STARTING, CHUNK_SENT, DONE } from './actions'

const INITIAL_STATE = {
    modalShown: false,
    busy: false,
    statusMsg: '',
}

export default (state = INITIAL_STATE, action) => {
    switch (action.type) {
        case PREPARE:
            return {
                ...state,
                modalShown: true,
            }

        case UPDATE_STATUS_MSG:
            return {
                ...state,
                statusMsg: action.statusMsg,
            }

        case CANCEL:
            if (state.busy) {
                return state
            }

            return {
                ...INITIAL_STATE,
            }

        case STARTING:
            return {
                ...state,
                modalShown: true,
                busy: true,
                fwImg: action.fwImg,
                bytesSent: 0,
                done: false,
                statusMsg: 'Starting...'
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
