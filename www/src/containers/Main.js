import React from 'react'
import { connect } from 'react-redux'
import { Button } from 'antd';

import DevInfoCard from './DevInfoCard'
import { midiSetup } from '../actions'
import { prepare as fwUpdatePrepare, start as fwUpdateStart } from '../fwUpdate/actions'
import { FwUpdateModal } from '../fwUpdate'
import { appShowError } from '../actions'

//import logo from './logo.svg'

const mapStateToProps = (state) => {
    return {
        midi: state.midi,
    }
}
// TODO
import binbin from '../../../cenx4/firmware/app/build/bl-ch.bin'
import binbin2 from '../../../cenx4/firmware/app/build/multi.img'
import { MidiBootloaderImg } from '../midi'

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
                if (!blImg.isCompatibleWithDev(devInfo)) {
                    continue
                }

                return dispatch(fwUpdateStart('server-firmware.img', blImg))
            }

            // Nothing suits us
            dispatch(appShowError('No firmware available for this device on the server :('))
        },
        onUpdateFwClick3: () => {
            // TODO broken, need MidiBootloaderMultiImg
            fetch(binbin2)
                .then((resp) => resp.blob())
                .then(blob => {
                    const reader = new FileReader()
                    reader.addEventListener('loadend', () => {
                        const buf = Array.from(new Uint8Array(reader.result))
                        dispatch(fwUpdateStart('boo', new MidiBootloaderImg(buf)))
                    })
                    reader.readAsArrayBuffer(blob)
                })
        }

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
                <Button onClick={this.props.onUpdateFwClick3}>Update multi-img (server)</Button>
            </div>
        )

//         return (
//             <div>
//                 <div className="App-header">
//                     <img src={logo} className="App-logo" alt="logo" />
//                     <h2>Welcome to React</h2>
//                 </div>
//                 <p className="App-intro">
//                     To get started, edit <code>src/App.js</code> and save to reload. lol!
//                     <Button type="primary" onClick={this.handleClick}>Button</Button>
//                 </p>
//             </div>
//         );
    }
}

export default connect(mapStateToProps, mapDispatchToProps)(Main)
