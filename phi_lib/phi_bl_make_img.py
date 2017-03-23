import argparse
import binascii
import struct
import sys

auto_int = lambda v: int(v, 0)

parser = argparse.ArgumentParser(description='Creates bootloader images')
parser.add_argument('--magic', type=auto_int, required=True, help='image magic word')
parser.add_argument('--page-size', type=auto_int, required=True, help='flash page size')
parser.add_argument('--start-offset', type=auto_int, required=True, help='image start offset (where vectors should be placed)')
parser.add_argument('input', help='input filename')
parser.add_argument('output', help='output filename')
args = parser.parse_args()

bl_hdr_size = 16
data = file(args.input, 'rb').read()

# Pad from image start till when vectors actually start (due to alignment)
data = ('\x00' * (args.start_offset - bl_hdr_size)) + data

# Pad to sector size, 16 is the header size that gets added after data
data += '\x00' * (args.page_size - ((len(data) + bl_hdr_size) % args.page_size))

hdr = struct.pack(
    'IIII',
    long(args.magic),
    binascii.crc32(data) & 0xffffffff,
    len(data),
    0x8010200
)


file(args.output, 'wb').write(hdr + data)
