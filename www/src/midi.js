import React from 'react'
import { connect } from 'react-redux'
import { crc16ccitt, crc32 } from 'crc'
import Struct from 'struct'
import { EventEmitter } from 'fbemitter'


/////////////////
class BaseMidiBootloader {
    constructor(midi) {
        this.midi = midi

        this.BL_RETS = [
            'PHI_BL_RET_ERR_UNKNOWN',
            'PHI_BL_RET_OK',
            'PHI_BL_RET_SIZE_MISMATCH',
            'PHI_BL_RET_START_ADDR_MISMATCH',
            'PHI_BL_RET_START_ADDR_MISALIGNED',
            'PHI_BL_RET_IMG_SIZE_MISALIGNED',
            'PHI_BL_RET_DEV_ID_MISMATCH',
            'PHI_BL_RET_NOT_READY',
            'PHI_BL_RET_OFFSET_MISMATCH',
            'PHI_BL_RET_OFFSET_OVERFLOW',
            'PHI_BL_RET_FLASH_ERASE_ERR',
            'PHI_BL_RET_FLASH_WRITE_ERR',
            'PHI_BL_RET_MISSING_DATA',
            'PHI_BL_RET_BAD_CRC',
            'PHI_BL_RET_BAD_MAGIC',
            'PHI_BL_RET_AT45_ERROR',
        ];
        this.BL_PKT_SIZE = 128;
    }
}

class MidiBootloader extends BaseMidiBootloader {
    start(dev_id, buf) {
        // Read data from our header
        const bl_hdr_size = 31;
        const bl_hdr = Struct()
            .word32Ule('magic')
            .word32Ule('hdr_data_crc32')
            .word32Ule('dev_id')
            .word8Ule('sw_ver')
            .word8Ule('hw_ver')
            .word8Ule('hw_ver_mask')
            .word32Ule('write_addr')
            .word32Ule('start_addr')
            .word32Ule('fw_data_size')
            .word32Ule('fw_data_crc32');
        bl_hdr.setBuffer(buf);

        // TODO validate magic, dev_id

        const msg = Struct()
            .word32Ule('img_size')
            .word32Ule('img_crc32')
            .array('bl_hdr', bl_hdr_size, 'word8Ule');
        msg.allocate();

        msg.fields.img_size = buf.length;
        msg.fields.img_crc32 = crc32(buf);
        msg.fields.bl_hdr = buf.slice(0, bl_hdr_size);
        console.log(msg.fields.bl_hdr);

        this.sent = 0
        this.buf = buf

        return new Promise((resolve, reject) => {
            this.midi.callSysExCmd(
                SYSEX_CMD.USER + 0, // PHI_MAIN_BL_MIDI_SYSEX_CMD_BL_START
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
                SYSEX_CMD.USER + 1, // PHI_MAIN_BL_MIDI_SYSEX_CMD_BL_DATA
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
                SYSEX_CMD.USER + 2 // PHI_MAIN_BL_MIDI_SYSEX_CMD_BL_DONE
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
class MidiSerialFlashBootloader extends BaseMidiBootloader {
    constructor(midi, {updateSelfWhenDone = false} = {}) {
        super(midi);
        self.updateSelfWhenDone = updateSelfWhenDone;
    }

    start(dev_id, buf) {
        const msg = Struct()
            .word32Ule('img_size')
            .word32Ule('img_crc32')
        msg.allocate();

        msg.fields.img_size = buf.length;
        msg.fields.img_crc32 = crc32(buf);

        this.sent = 0
        this.buf = buf

        return new Promise((resolve, reject) => {
            this.midi.callSysExCmd(
                SYSEX_CMD.USER + 3, // CENX4_BL_MIDI_SYSEX_CMD_BL_SERFLASH_START
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
                SYSEX_CMD.USER + 4, // CENX4_BL_MIDI_SYSEX_CMD_BL_SERFLASH_DATA
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
                SYSEX_CMD.USER + 5, // CENX4_BL_MIDI_SYSEX_CMD_BL_SERFLASH_DONE
                null,
                5000
            ).then(
                (data) => {
                    if (data[1] !== 1)
                    {
                        reject(this.BL_RETS[data[1]])
                    }
                    else
                    {
                        if (self.updateSelfWhenDone)
                        {
                            this.midi.callSysExCmd(
                                SYSEX_CMD.USER + 6, // CENX4_BL_MIDI_SYSEX_CMD_BL_UPDATE_SELF_FROM_SERFLASH
                                null,
                                10000
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
                        } else {
                            resolve()
                        }
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
                        0x34584e43: 'CENX4',
                        0x4956524e: 'Narvi',
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

    getSerialFlashBootloader(args) {
        return new MidiSerialFlashBootloader(this, args)
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
