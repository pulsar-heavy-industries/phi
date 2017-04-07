import React from 'react'
import { connect } from 'react-redux'
import { crc16ccitt, crc32 } from 'crc'
import Struct from 'struct'
import { EventEmitter } from 'fbemitter'


/////////////////
class MidiBootloader {
    constructor(midi) {
        this.midi = midi

        this.BL_RETS = [
            'AB_BL_RET_ERR_UNKNOWN',
            'AB_BL_RET_OK',
            'AB_BL_RET_SIZE_MISMATCH',
            'AB_BL_RET_START_ADDR_MISMATCH',
            'AB_BL_RET_START_ADDR_MISALIGNED',
            'AB_BL_RET_IMG_SIZE_MISALIGNED',
            'AB_BL_RET_DEV_ID_MISMATCH',
            'AB_BL_RET_NOT_READY',
            'AB_BL_RET_OFFSET_MISMATCH',
            'AB_BL_RET_OFFSET_OVERFLOW',
            'AB_BL_RET_FLASH_ERASE_ERR',
            'AB_BL_RET_FLASH_WRITE_ERR',
            'AB_BL_RET_MISSING_DATA',
            'AB_BL_RET_BAD_CRC',
        ];
        this.BL_PKT_SIZE = 128;
    }

    start(dev_id, buf) {
        const msg = Struct()
            .word32Ule('img_size')
            .word32Ule('img_start_addr')
            .word32Ule('dev_id')
            .word32Ule('crc32')
            .array('reserved', 4, 'word32Ule');
        msg.allocate();

        let start_addr;
        if (dev_id === 0x42455259 || dev_id === 0x434e5834)
        {
            start_addr = 0x8010000;
        }
        else
        {
            alert('TODO!');
            debugger;
        }


        msg.fields.img_size = buf.length;
        msg.fields.img_start_addr = start_addr; // TODO
        msg.fields.dev_id = dev_id;
        msg.fields.crc32 = crc32(buf);

        this.sent = 0
        this.buf = buf

        return new Promise((resolve, reject) => {
            this.midi.callSysExCmd(
                SYSEX_CMD.USER + 0, // AB_MAIN_BL_MIDI_SYSEX_CMD_BL_START
                Array.from(msg.buffer())
            ).then(
                (data) => {
                    if (data[1] !== 1)
                    {
                        reject(this.BL_RETS[data[1]]);
                        // TODO handle this
                        console.log('boo', this.BL_RETS[data[1]])
                    }
                    else
                    {
                        resolve();
                    }
                },
                reject
            );
        });
    }

    sendData(offset, buf) {
        const msg = Struct()
            .word32Ule('offset')
            .array('data', this.BL_PKT_SIZE, 'word8Ule');
        msg.allocate();

        //buf = Buffer.from(buf)
        msg.fields.offset = offset
        msg.fields.data = buf

        return new Promise((resolve, reject) => {
            this.midi.callSysExCmd(
                SYSEX_CMD.USER + 1, // AB_MAIN_BL_MIDI_SYSEX_CMD_BL_DATA
                Array.from(msg.buffer()),
                2000 // Flash writes can take time
            ).then(
                (data) => {
                    if (data[1] !== 1)
                    {
                        reject(this.BL_RETS[data[1]]);
                    }
                    else
                    {
                        resolve();
                    }
                },
                reject
            );
        });
    }


    sendNextChunk(chunkSent, done) {
        const chunk_size = Math.min(this.BL_PKT_SIZE, this.buf.length - this.sent);
        console.log(`bl data: ${this.sent}/${this.buf.length}: ${chunk_size}`)

        this.sendData(this.sent, this.buf.slice(this.sent, this.sent + chunk_size)).then(
            () => {
                this.sent += chunk_size
                chunkSent(chunk_size)
                if (this.sent === this.buf.length) {
                    console.log('bl: sending done!');

                    this.handleSendComplete().then(done)//TODO .catch...
               } else {
                    this.sendNextChunk(chunkSent, done);
                }
            },
            (error) => {
                console.log('bl data error: ', error);
            }
        );
    }

    handleSendComplete() {
        return new Promise((resolve, reject) => {
            this.midi.callSysExCmd(
                SYSEX_CMD.USER + 2 // AB_MAIN_BL_MIDI_SYSEX_CMD_BL_DONE
            ).then(
                (data) => {
                    if (data[1] !== 1)
                    {
                        reject(this.BL_RETS[data[1]])
                    }
                    else
                    {
                        resolve()
                    }
                },
                reject
            )
        })
    }
}
/////////////////


export const getMidiDeviceList = () => {
    return window.navigator.requestMIDIAccess({sysex: true})
        .then((access) => ({
            inputs: [...access.inputs.values()],
            outputs: [...access.outputs.values()],
        }))
        .then(({inputs, outputs}) => ({
            inputs: inputs.map((input) => input.name),
            outputs: outputs.map((output) => output.name),
        }))
}


const SYSEX_CMD = {
    ECHO: 1,
    DEV_INFO: 2,
    USER: 32,
};


class MidiDevice {
    constructor(input, output) {
        this.input = input
        this.output = output
        this.settings = {
            timeout: 500, // in msec
        }
        this.emitter = new EventEmitter()
        this.SYSEX_CMD = SYSEX_CMD
    }

    open() {
        this.input.onmidimessage = (msg) => { this.onMidiMessage(msg) }
        console.log('midi open')
    }

    close() {
        this.input.onmidimessage = null
        console.log('midi close')
    }

    onMidiMessage(msg) {
        var data = msg.data;

        switch (data[0])
        {
            // SysEx (at least 0xf0 0x41 0x12 crc crc cmd 0xf7)
            case 0xf0:
                if ((data.length >= 7) &&
                    (data[1] === 0x41) &&
                    (data[2] === 0x12) &&
                    (data[data.length - 1] === 0xf7))
                {
                    data = data.slice(3, -1);
                    let w = 0, rd = 0;
                    while (rd < data.length)
                    {
                        // read the bits byte
                        let bits = data[rd];
                        rd++;

                        // see how many bytes we will be processing in this chunk
                        let chunk = Math.min(7, data.length - rd);

                        for (let i = 0; i < chunk; ++i)
                        {
                            data[w] = data[rd] | ( (bits & (1 << i)) ? 0x80 : 0 );
                            rd++;
                            w++;
                        }
                    }

                    let crc1 = data[0] | (data[1] << 8);
                    data = data.slice(2, w);
                    let crc2 = crc16ccitt(data);

                    if (crc1 === crc2)
                    {
                        this.emitter.emit('in-sysex', data);
                    }
                    else
                    {
                        console.log('SYSEX BAD CRC', crc1, crc2, data);
                    }
                }
                break;

            default:
                console.log('Unknown MIDI data', data);
                break;
        }

    }

    sendSysEx(buf) {
        return new Promise((resolve, reject) => {
            let crc = crc16ccitt(buf);
            buf = [crc & 0xff, (crc >> 8) & 0xff].concat(buf);

            let out_buf = [];
            for (let r = 0; r < buf.length; )
            {
                let chunk = buf.length - r;
                if (chunk > 7)
                {
                    chunk = 7;
                }

                let bits = 0;
                for (let i = 0; i < chunk; ++i)
                {
                    if (buf[r + i] & 0x80)
                    {
                        bits |= (1 << i);
                    }
                }
                out_buf.push(bits);
                for (let i = 0; i < chunk; ++i)
                {
                    out_buf.push(buf[r] & 0x7F);
                    r++;
                }
            }

            if (out_buf.length > 295) {
                reject('nope: ' + out_buf.length);
            }

            let message = [].concat([0xf0, 0x41, 0x12], out_buf, [0xf7]);
            this.output.send(message);
            resolve(true);
        });
    }

    callSysExCmd(cmd, buf, timeout) {
        return new Promise((resolve, reject) => {
            var timer = setTimeout(() => reject('timeout'), timeout || this.settings.timeout)
            var token = this.emitter.addListener('in-sysex', (data) => {
                if (data[0] !== cmd)
                {
                    return
                }

                token.remove()
                clearTimeout(timer)

                resolve(data)
            });
            this.sendSysEx([cmd].concat(buf || []));
        });
    }

    getDevInfo() {
        return new Promise((resolve, reject) => {
            this.callSysExCmd(SYSEX_CMD.DEV_INFO).then(
                (data) => {
                    const DevInfo = Struct()
                        .word8('sysex_cmd')
                        .word32Ule('dev_id')
                        .word32Ule('hw_sw_ver')
                        .array('uid', 16, 'word8')
                        .array('reserved', 4, 'word32Ule')
                    DevInfo.setBuffer(data);
                    let fields = DevInfo.fields

                    fields.uid_str = ''
                    const toHex = (v) => {
                        const h = v.toString(16)
                        return h.length === 1 ? '0' + h : h
                    }
                    for (let i = 0; i < 16; ++i) {
                        fields.uid_str += toHex(fields.uid[i])
                    }

                    const deviceNames = {
                        0x434e5834: 'CENX4',
                    }
                    fields.dev_name = deviceNames[fields.dev_id] || 'Unknown'

                    fields.is_bootloader = fields.hw_sw_ver & 0x80000000
                    fields.hw_ver = (fields.hw_sw_ver >> 24) & 0x7f
                    fields.sw_ver = fields.hw_sw_ver & 0xffffff

                    resolve(fields);
                },
                reject
            )
        });
    }

    getBootloader() {
        return new MidiBootloader(this)
    }
}


class _MidiManager extends React.Component {
    componentDidMount() {
        this.props.dev.open()
    }

    componentWillUnmount()  {
        this.props.dev.close()
    }

    render() {
        return null
    }
}
export const MidiManager = connect(
    (state) => {
        return {
            dev: state.midi.dev,
        }
    }
)(_MidiManager)


export const openMidiDevice = (inputName, outputName) => {
    const findByName = (list, name) => {
        let result = null
        list.forEach((dev) => {
            if (dev.name === name) {
                result = dev
            }
        })
        return result
    }

    return window.navigator.requestMIDIAccess({sysex: true})
        .then((access) => ({
            input: findByName(access.inputs, inputName),
            output: findByName(access.outputs, outputName),
        }))
        .then((io) => {
            if (!io.input || !io.output) {
        throw Error('could not find device specified');
      } else {
        return Promise.all([io.input.open(), io.output.open()]);
      }
    })
    .then(([input, output]) => new MidiDevice(input, output));
}
