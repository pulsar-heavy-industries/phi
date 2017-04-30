import binascii
import os
import struct
import sys
from collections import namedtuple

if len(sys.argv) < 3:
    print 'Usage: {} <output file> <inputs>'.format(sys.argv[0])
    sys.exit(1)

output_filename, input_filenames = sys.argv[1], sys.argv[2:]

payload = ''
for input_filename in input_filenames:
    try:
        payload += file(input_filename, 'rb').read()
    except IOError:
        print 'Error reading {}'.format(input_filename)
        sys.exit(1)

data = struct.pack(
    '<IIIB',
    0x4d4c5431, # magic,
    binascii.crc32(payload) & 0xffffffff,
    len(payload),
    len(input_filenames),
) + payload

# Pad to 128 bytes
data += '\x00' * (128 - (len(data) % 128))

# Write
file(output_filename, 'wb').write(data)
