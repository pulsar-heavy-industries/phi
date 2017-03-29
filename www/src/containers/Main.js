import React from 'react'
import { connect } from 'react-redux'
import { Button } from 'antd';

import DevInfoCard from './DevInfoCard'
import { midiSetup } from '../actions'
import { prepare as fwUpdatePrepare, start as fwUpdateStart } from '../fwUpdate/actions'
import { FwUpdateModal } from '../fwUpdate'

//import logo from './logo.svg'

const mapStateToProps = (state) => {
    return {
        midi: state.midi,
    }
}
// TODO
import binbin from '../fwUpdate/bl-ch.bin'

const mapDispatchToProps = (dispatch) => {
    return {
        onResetClick: (inputName, outputName) => {
            dispatch(midiSetup(inputName, outputName))
        },
        onUpdateFwClick: () => {
            dispatch(fwUpdatePrepare())
        },
        onUpdateFwClick2: () => {
            fetch(binbin)
                .then((resp) => resp.blob())
                .then(blob => {
                    const reader = new FileReader()
                    reader.addEventListener('loadend', () => {
                        const buf = Array.from(new Uint8Array(reader.result))
                        dispatch(fwUpdateStart('boo', buf))
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
                {this.props.midi.devInfo.is_bootloader && <FwUpdateModal/>}
                <Button onClick={() => { this.props.onResetClick(this.props.midi.inputName, this.props.midi.outputName) }}>Reset</Button>
                <Button onClick={this.props.onUpdateFwClick}>Update firmware</Button>
                <Button onClick={this.props.onUpdateFwClick2}>Update firmware (server)</Button>
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
