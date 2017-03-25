from Cenx4Mgr import Cenx4Mgr

CRC16_TABLE = [
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
    0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
    0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
    0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
    0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
    0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
    0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
    0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
    0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
    0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
    0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
    0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
    0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
    0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
    0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
    0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
    0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
    0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
    0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
    0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
    0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
    0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
    0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
    0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0,
]


def crc16(data, crc=0xffff, table=CRC16_TABLE):
    for byte in data:
        crc = ((crc<<8)&0xff00) ^ table[((crc>>8)&0xff)^(byte)]
    return crc & 0xffff


CENX4_MIDI_SYSEX_CMD_USER = 32
CENX4_MAIN_MIDI_SYSEX_APP_CMD = CENX4_MIDI_SYSEX_CMD_USER + 1

CENX4_APP_ABLETON_SYSEX_INVALID = 0
CENX4_APP_ABLETON_SYSEX_SET_POT_VAL = 1
CENX4_APP_ABLETON_SYSEX_SET_POT_TEXT = 2
CENX4_APP_ABLETON_SYSEX_SET_POT_ALL = 3
CENX4_APP_ABLETON_SYSEX_RESYNC = 4
CENX4_UI_MAX_LINE_TEXT_LEN = 16


class SysExProtocol(object):
    def __init__(self, send_midi):
        self._send_midi = send_midi

    def send_midi(self, data):
        self._send_midi(tuple(self.encode(data)))

    def decode(self, data):
        # 240, 65, 18, 0, 123, 8, 33, 3, 2, 247)
        # minimal msg: 0xf0 0x41 0x12 crc crc cmd 0xf7
        if (
            (len(data) < 7) or
            (data[0] != 0xf0) or
            (data[1] != 0x41) or
            (data[2] != 0x12) or
            (data[-1] != 0xf7)
        ):
            return

        data = list(data)

        data = data[3:-1]
        w = rd = 0
        while rd < len(data):
            # Bits byte
            bits = data[rd]
            rd += 1

            # See how many bytes we will be processing in this chunk
            chunk = min(7, len(data) - rd)
            for i in range(chunk):
                data[w] = data[rd] | (0x80 if (bits & (1 << i)) else 0)
                rd += 1
                w += 1

        # Check crc
        crc1 = data[0] | data[1] << 8
        data = data[2:w]
        crc2 = crc16(data)

        if crc1 == crc2:
            return data

    def encode(self, data):
        crc = crc16(data)
        data = [crc & 0xff, (crc >> 8) & 0xff] + data

        out_buf = []
        r = 0
        while r < len(data):
            chunk = min(len(data) - r, 7)

            bits = 0
            for i in range(chunk):
                if data[r + i] & 0x80:
                    bits |= 1 << i
            out_buf.append(bits)

            for i in range(chunk):
                out_buf.append(data[r] & 0x7F)
                r += 1

        assert len(out_buf) <= 295
        return [0xf0, 0x41, 0x12] + out_buf + [0xf7]

    def c_str(self, s, l):
        return [ord(ch) for ch in s[:l - 1].ljust(l, '\x00')]

    def set_pot_val(self, pot, val):
        assert pot < Cenx4Mgr.cfg.num_pots, pot
        buf = [
            CENX4_MAIN_MIDI_SYSEX_APP_CMD,
            CENX4_APP_ABLETON_SYSEX_SET_POT_VAL,
            pot,
            val,
        ]
        self.send_midi(buf)

    def set_pot_text(self, pot, top, bottom):
        assert pot < Cenx4Mgr.cfg.num_pots, pot
        buf = [
            CENX4_MAIN_MIDI_SYSEX_APP_CMD,
            CENX4_APP_ABLETON_SYSEX_SET_POT_TEXT,
            pot,
        ] + self.c_str(top, CENX4_UI_MAX_LINE_TEXT_LEN) + self.c_str(bottom, CENX4_UI_MAX_LINE_TEXT_LEN)
        self.send_midi(buf)

    def set_pot_all(self, pot, val, top, bottom):
        assert pot < Cenx4Mgr.cfg.num_pots, pot
        buf = [
            CENX4_MAIN_MIDI_SYSEX_APP_CMD,
            CENX4_APP_ABLETON_SYSEX_SET_POT_ALL,
            pot,
            val,
        ] + self.c_str(top, CENX4_UI_MAX_LINE_TEXT_LEN) + self.c_str(bottom, CENX4_UI_MAX_LINE_TEXT_LEN)
        self.send_midi(buf)

    def set_pot_all_scaled(self, pot, val, val_min, val_max, top, bottom):
        # Figure out how 'wide' each range is
        val_span = val_max - val_min
        rightSpan = 100 - 0

        # Convert the left range into a 0-1 range (float)
        valueScaled = float(val - val_min) / float(val_span)

        # Convert the 0-1 range into a value in the right range.
        val = int(0 + (valueScaled * rightSpan))
        self.set_pot_all(pot, val, top, bottom)


