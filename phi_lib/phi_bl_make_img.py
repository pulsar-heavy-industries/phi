import argparse
import binascii
import struct
import sys

auto_int = lambda v: int(v, 0)
BL_HDR_DATA_FMT = '<IBBBIIII'
BL_HDR_PREFIX_FMT = '<II'
BL_HDR_FMT = BL_HDR_PREFIX_FMT + BL_HDR_DATA_FMT[1:]

parser = argparse.ArgumentParser(description='Creates bootloader images')
parser.add_argument('--page-size', type=auto_int, required=True, help='flash page size')
parser.add_argument('--dev-id', type=auto_int, required=True, help='device id')
parser.add_argument('--sw-ver', type=auto_int, required=True, help='software version in this image')
parser.add_argument('--hw-ver', type=auto_int, required=True, help='hardware versions this image can run on')
parser.add_argument('--hw-ver-mask', type=auto_int, required=True, help='hardware version mask')
parser.add_argument('--write-addr', type=auto_int, required=True, help='image write address (where the image gets written)')
parser.add_argument('--start-addr', type=auto_int, required=True, help='image start address (where vectors should be placed)')
parser.add_argument('input', help='input filename')
parser.add_argument('output', help='output filename')
args = parser.parse_args()

bl_hdr_size = struct.calcsize(BL_HDR_FMT)
fw_data = file(args.input, 'rb').read()

# Some sanity tests
assert args.start_addr > args.write_addr
assert 0 < args.sw_ver
assert 0 < args.hw_ver < 0x7f
assert args.dev_id

# Pad from image start till when vectors actually start (due to alignment)
fw_data = ('\x00' * ((args.start_addr - args.write_addr) - bl_hdr_size)) + fw_data

# Pad to sector size, 16 is the header size that gets added after data
fw_data += '\x00' * (args.page_size - ((len(fw_data) + bl_hdr_size) % args.page_size))

hdr_data = struct.pack(
    BL_HDR_DATA_FMT,

    # Device ID this image is for
    args.dev_id,

    # Software version this image holds
    args.sw_ver,

    # Hardware version + mask this image can run on
    args.hw_ver,
    args.hw_ver_mask,

    # Address this image should be written to
    args.write_addr,

    # Address to jump to when starting
    args.start_addr,

    # Size of data
    len(fw_data),

    # CRC of the data that follows
    binascii.crc32(fw_data) & 0xffffffff,
)

img = struct.pack(
    BL_HDR_PREFIX_FMT,

    # Magic
    0xC0DE1337,

    # Header CRC
    binascii.crc32(hdr_data) & 0xffffffff,
) + hdr_data + fw_data


file(args.output, 'wb').write(img)
