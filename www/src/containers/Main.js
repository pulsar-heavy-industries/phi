import React from 'react'
import { connect } from 'react-redux'
import { Button } from 'antd';

import DevInfoCard from './DevInfoCard'
import { midiSetup } from '../actions'
import { prepare as fwUpdatePrepare, start as fwUpdateStart } from '../fwUpdate/actions'
import { FwUpdateModal } from '../fwUpdate'
import { appShowError } from '../actions'
import { MidiBootloaderImg, MidiBootloaderMultiImg } from '../midi'

const mapStateToProps = (state) => {
    return {
        midi: state.midi,
    }
}

const mapDispatchToProps = (dispatch) => {
    return {
        onResetClick: (inputName, outputName) => {
            dispatch(midiSetup(inputName, outputName))
        },
        onUpdateFwClick: () => {
            dispatch(fwUpdatePrepare())
        },
        onUpdateFwFromServerClick: (devInfo, blImgs) => {
            // Try and find a firmware that suits us
            for (let blImg of blImgs) {
                console.log('maybe', blImg)
                if (!blImg.isCompatibleWithDev(devInfo) || !(blImg instanceof MidiBootloaderImg)) {
                    continue
                }

                return dispatch(fwUpdateStart('server-firmware.img', blImg))
            }

            // Nothing suits us
            dispatch(appShowError('No firmware available for this device on the server :('))
        },
        onUpdateMultiImgFwFromServerClick: (devInfo, blImgs) => {
            // Try and find a firmware that suits us
            for (let blImg of blImgs) {
                console.log('maybe', blImg)
                if (!blImg.isCompatibleWithDev(devInfo) || !(blImg instanceof MidiBootloaderMultiImg)) {
                    continue
                }

                return dispatch(fwUpdateStart('multi-img.img', blImg))
            }

            // Nothing suits us
            dispatch(appShowError('No firmware available for this device on the server :('))
        },

    }
}



class Main extends React.Component {
    componentDidMount() {
    }

    handleClick = (e) => {
        console.log(e, this);
    }

    render() {
        return (
            <div>
                <DevInfoCard />
                <FwUpdateModal/>
                <Button onClick={() => { this.props.onResetClick(this.props.midi.inputName, this.props.midi.outputName) }}>Reset</Button>
                <Button onClick={this.props.onUpdateFwClick}>Update firmware</Button>
                <Button onClick={() => { this.props.onUpdateFwFromServerClick(this.props.midi.devInfo, this.props.midi.serverBlImgs) }}>Update firmware (server)</Button>
                <Button onClick={() => { this.props.onUpdateMultiImgFwFromServerClick(this.props.midi.devInfo, this.props.midi.serverBlImgs) }}>Update multi-img firmware (server)</Button>
            </div>
        )

    }
}

export default connect(mapStateToProps, mapDispatchToProps)(Main)
