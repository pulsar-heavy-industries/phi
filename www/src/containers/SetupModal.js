import React from 'react'
import { connect } from 'react-redux'
import { Form, Select, Button, Modal } from 'antd'

const FormItem = Form.Item;
const Option = Select.Option;

import { midiSetup } from '../actions'
import { getMidiDeviceList } from '../midi'


const SetupForm = Form.create()(
    (props) => {
        const { inputs, outputs, onContinue, form } = props
        const { getFieldDecorator } = form

        const formItemLayout = {
            labelCol: {
                xs: { span: 24 },
                sm: { span: 6 },
            },
            wrapperCol: {
                xs: { span: 24 },
                sm: { span: 14 },
            },
        };

        let defaultOutput = null;
        let defaultInput = null;
        [
            'Pulsar Heavy Industries CENX4 CENX4 MIDI',
            'Pulsar Heavy Industries CENX4',
            'Pulsar Heavy Industries Narvi NarviCtrl',
        ].forEach((val) => {
            if (!defaultInput && inputs.indexOf(val) !== -1)
            {
                defaultInput = val
            }

            if (!defaultOutput && outputs.indexOf(val) !== -1)
            {
                defaultOutput = val
            }
        })

        return <Modal
            title="Setup"
            onOk={onContinue}
            closable={false}
            visible
            width={700}
            footer={[
                <Button key="continue" type="primary" size="large" onClick={onContinue}>
                    Continue
                </Button>
            ]}
        >
            <Form>
                <FormItem
                    {...formItemLayout}
                    label="Input device"
                    hasFeedback
                >
                    {getFieldDecorator('input', {
                        rules: [{ required: true }],
                        initialValue: defaultInput,
                    })(
                        <Select>{inputs.map((input) =>
                            <Option key={input} value={input}>{input}</Option>
                        )}</Select>
                    )}
                </FormItem>

                <FormItem
                    {...formItemLayout}
                    label="Output device"
                    hasFeedback
                >
                    {getFieldDecorator('output', {
                        rules: [{ required: true }],
                        initialValue: defaultOutput,
                    })(
                        <Select>{outputs.map((output) =>
                            <Option key={output} value={output}>{output}</Option>
                        )}</Select>
                    )}
                </FormItem>
          </Form>
        </Modal>
    }
)


class SetupModal extends React.Component {
    constructor(props) {
        super(props)
        this.state = {
            inputs: [],
            outputs: [],
        }
    }

    onContinue = () => {
        this.form.validateFields((err, values) => {
            if (err) {
                return;
            }

            this.props.onContinueClick(values.input, values.output)
        })
    }

    componentWillMount() {
        getMidiDeviceList()
            .then(({inputs, outputs}) => {
                this.setState({
                    inputs,
                    outputs,
                })
            })
            .catch(() => {
                console.log('TODO')
            })
    }

    render() {
        return <SetupForm
            ref={(form) => {this.form = form}}
            onContinue={this.onContinue}
            {...this.state}
        />
    }
}

const mapStateToProps = (state) => {
    return {
    }
}

const mapDispatchToProps = (dispatch) => {
    return {
        onContinueClick: (inputName, outputName) => {
            dispatch(midiSetup(inputName, outputName))
        }
    }
}

export default connect(mapStateToProps, mapDispatchToProps)(SetupModal)

