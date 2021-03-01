#!/usr/bin/env python3

"""The microapp make-helper."""

import time
import argparse
import math

from crownstone_core.util import CRC

parser = argparse.ArgumentParser(description='Manipulate microapp binary')
parser.add_argument('input',
        help='The binary file to be processed.')
parser.add_argument('output',
        help='The file to write output to.')
parser.add_argument('field',
        help='The field to write.')

args = parser.parse_args()

ifile=args.input
ofile=args.output
field=args.field

with open(ifile, "rb") as f:
    buf = f.read()
f.close()

size=len(buf)

header_size = 16
chunk_size = 40


# print(f"header: {list(buf[0:header_size])}")

numChunks = math.ceil((size - header_size) / chunk_size)
# print(f"numChunks={numChunks}")
for i in range(0, numChunks):
    chunk = buf[header_size + i * chunk_size: header_size + (i+1) * chunk_size]
    # print(f"CRC chunk {i} = {hex(CRC.CRC_16_CCITT.crc(chunk))}")
    # print(f"  data: {list(chunk)}")

# Calculates checksum over the entire buffer
checksum = CRC.CRC_16_CCITT.crc(buf)

offset = 16
null = 0

#b_offset = bytearray(offset.to_bytes(4, 'little'))
#b_size = bytearray(size.to_bytes(4,'little'))
#b_checksum = bytearray(h_checksum.to_bytes(4,'little'))
#b_reserve = bytearray(null.to_bytes(4, 'little'))

with open(ofile, "w") as new:
    if field == 'offset':
        new.write(str(offset))
    if field == 'size':
        new.write(str(size))
    if field == 'checksum':
        new.write(str(checksum))
    if field == 'reserve':
        new.write(str(null))

