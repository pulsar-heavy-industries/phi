import React from 'react'
import { connect } from 'react-redux'
import { Progress, Icon, Upload, Button, Modal } from 'antd'

import { cancel, start } from './actions'
import { midiSetup } from '../actions'

const mapStateToProps = (state) => {
    return {
        devInfo: state.midi.devInfo,
        shown: state.fwUpdate.modalShown,
        busy: state.fwUpdate.busy,
        done: state.fwUpdate.done,
        fwImg: state.fwUpdate.fwImg,
        bytesSent: state.fwUpdate.bytesSent,
        statusMsg: state.fwUpdate.statusMsg,
    }
}

const mapDispatchToProps = (dispatch) => {
    return {
        handleStart: (fileName, buf) => dispatch(start(fileName, buf)),
        handleCancel: () => dispatch(cancel()),
        handleContinue: () => {
            dispatch(cancel())
            dispatch(midiSetup())
        }
    }
}

class FwUpdateModal extends React.Component {
    handleUpload(file, fileList) {
        const reader = new FileReader()
        reader.readAsArrayBuffer(file)
        reader.onloadend = () => {
            const buf = Array.from(new Uint8Array(reader.result))
            this.props.handleStart(file.name, buf)
        }

        return false
    }

    render() {
        return <Modal
            title="Firmware Update"
            closable={false}
            visible={this.props.shown}
            width={700}
            footer={[
                <Button key="cancel" size="large" onClick={this.props.handleCancel} disabled={this.props.busy || this.props.done}>Cancel</Button>,
                <Button key="continue" type="primary" size="large" disabled={!this.props.done} loading={this.props.busy} onClick={this.props.handleContinue}>Continue</Button>,
            ]}
        >
            {!this.props.fwImg && <Upload
                beforeUpload={(file, fileList) => this.handleUpload(file, fileList)}
            >
                <Button><Icon type="upload" /> Click to Upload</Button>
            </Upload>}

            {this.props.fwImg && <div>
                <strong>Firmware Image:</strong> { this.props.fwImg.fileName } ({ this.props.bytesSent }/{ this.props.fwImg.blImg.buf.length } bytes)
                {this.props.fwImg.blImg.sw_ver && <div><strong>Software Version:</strong> 0x{ this.props.fwImg.blImg.sw_ver.toString(16) }</div>}
                <Progress percent={Math.floor(this.props.bytesSent * 100 / this.props.fwImg.blImg.buf.length)}/>
                <span>{ this.props.statusMsg }</span>
                {this.busy && <strong>Do not navigate away from the app.</strong>}
            </div>}
        </Modal>
    }
}

export default connect(mapStateToProps, mapDispatchToProps)(FwUpdateModal)
