import React from 'react'
import { connect } from 'react-redux'
import { Card, Menu, Dropdown } from 'antd';
import { appReset, midiSetup } from '../actions'

const mapStateToProps = (state) => {
    return {
        midi: state.midi,
        devInfo: state.midi.devInfo,
    }
}

const mapDispatchToProps = (dispatch) => {
    return {
        onActionClick: (e, midi) => {
            switch (e.key) {
                case 'reset':
                    return dispatch(midiSetup())

                case 'reset-full':
                    return dispatch(appReset(true))

                default:
                    break
            }
        }
    }
}


class DevInfoCard extends React.Component {
    render() {
        const actions = <Menu onClick={(e) => this.props.onActionClick(e, this.props.midi) }>
            <Menu.Item key="reset">Reset</Menu.Item>
            <Menu.Item key="reset-full">Reset and reconfigure</Menu.Item>
        </Menu>

        return <Card
            title="Device Information"
            style={{ width: 500 }}
            extra={
                <Dropdown.Button overlay={actions}>
                    Actions
                </Dropdown.Button>
            }
            >
            <div>
                <ul>
                    <li>
                        <strong>Device Id</strong>:
                        0x{ this.props.devInfo.dev_id.toString(16) }
                        ({ this.props.devInfo.dev_name }{ this.props.devInfo.is_bootloader ? ' - Bootloader' : ''})
                    </li>
                    <li><strong>HwSw version:</strong> 0x{ this.props.devInfo.hw_sw_ver.toString(16) }</li>
                    <li><strong>UID:</strong> { this.props.devInfo.uid_str }</li>
                </ul>
            </div>
        </Card>
    }
}

export default connect(mapStateToProps, mapDispatchToProps)(DevInfoCard)
