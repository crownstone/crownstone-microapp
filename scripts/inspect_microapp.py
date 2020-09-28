#!/usr/bin/env python3

"""An example to inspect a micro app package."""

import time
import argparse
import math

from crownstone_core.util.fletcher import fletcher32_uint8Arr

parser = argparse.ArgumentParser(description='Inspect microapp package')
parser.add_argument('microapp', 
        help='The microapp (.obj) file to be sent.')

args = parser.parse_args()

print("===========================================\n\nStarting Example\n\n===========================================")

print("Read microapp binary", args.microapp)
with open(args.microapp, "rb") as f:
    buf = f.read()

print("The overall binary contains", len(buf), "bytes")

chunk_size = 40
print("We will use a chunk size of", chunk_size, "bytes")

# Create chunk of proper size
chunk = bytearray(chunk_size)

nof_chunks = math.ceil(len(buf) / chunk_size)
print("Total number of chunks is", nof_chunks)

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
    print("Checksum of chunk", c, "is", hex(checksum & 0xFFFF));

print(len(buf))
# Calculates checksum over the entire buffer
checksum = fletcher32_uint8Arr(buf)
print("The checksum of the complete file is", hex(checksum & 0xFFFF))


print("===========================================\n\nFinished Example\n\n===========================================")
