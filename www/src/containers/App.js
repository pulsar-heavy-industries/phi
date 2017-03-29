import React from 'react'
import { connect } from 'react-redux'

import './App.css'
import SetupModal from './SetupModal'
import Main from './Main'
import { MidiManager } from '../midi'
import AppError from './AppError'

const mapStateToProps = (state) => {
    return {
        error: state.app.error,
        setupRequired: state.midi.setupRequired,
        dev: state.midi.dev,
        inputName: state.midi.inputName,
        outputName: state.midi.outputName,
    }
}

class App extends React.Component {
    render() {
        let maybeError = null
        let maybeSetup = null
        let maybeMain = null

        if (this.props.error) {
            maybeError = <AppError/>
        }

        if (this.props.setupRequired) {
            // If we already have input/output names then we don't actually need the dialog box
            // (it means we're doing a reset)
            if (!this.props.inputName || !this.props.outputName) {
                maybeSetup = <SetupModal />
            }
        } else {
            maybeMain = <div>
                <MidiManager/ >
                <Main />
            </div>
        }

        return (
            <div className="App">
                {maybeError}
                {maybeSetup}
                {maybeMain}
            </div>
        )
    }
}
export default connect(mapStateToProps)(App)
