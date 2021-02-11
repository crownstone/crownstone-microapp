#!/usr/bin/env python3

"""The microapp make-helper."""

import time
import argparse
import math

from crownstone_core.util.fletcher import fletcher32_uint8Arr

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

chunk_size = 40

# Create chunk of proper size
chunk = bytearray(chunk_size)

nof_chunks = math.ceil(len(buf) / chunk_size)

# Create a chunk size that can be zero-padded if it is originally of odd size
if chunk_size % 2 != 0:
    chunk.append(0)

for c in range(0, nof_chunks):
    # Segment [start,stop] in chunk
    start = c*chunk_size
    stop = (c+1)*chunk_size
    last = stop > len(buf)
    if last:
        stop = len(buf)
    this_chunk_size = stop - start;
    chunk[0:this_chunk_size] = buf[start:stop]

    # Add zero-padding to every chunk of odd size as well as the last chunk
    for i in range(this_chunk_size, chunk_size):
        chunk[i] = 0

    checksum = fletcher32_uint8Arr(chunk)

# Calculates checksum over the entire buffer
r_checksum = fletcher32_uint8Arr(buf)
checksum = r_checksum & 0xFFFF

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

