import struct
import crcmod

phi_crc32 = crcmod.mkCrcFun(0x104c11db7, initCrc=0, xorOut=0xFFFFFFFF)

magic = 0xC0DE1337
sector_size = 2 * 1024
data = file('build/ch.bin', 'r').read()

# Pad from image start till when vectors actually start (due to alignment)
data = ('\x00' * (0x200 - 16)) + data

# Pad to sector size, 16 is the header size that gets added after data
data += '\x00' * (sector_size - ((len(data) + 16) % sector_size))

hdr = struct.pack(
    'IIII',
    magic,
    phi_crc32(data),
    len(data),
    0x8010200
)

file('build/fw.hdr', 'w').write(hdr)

file('build/fw.bin', 'w').write(
    hdr +
    data
)
