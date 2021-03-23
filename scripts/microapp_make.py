#!/usr/bin/env python3

"""The microapp make-helper."""

import argparse

from CRC import crc16ccitt

from MicroappBinaryHeaderPacket import MicroappBinaryHeaderPacket

parser = argparse.ArgumentParser(description='Manipulate microapp binary')
parser.add_argument('-i', '--input',
        help='The binary file to be processed. If no input is given, the fields will be set to dummy values.')
parser.add_argument('output',
        help='The file to write output to.')

args = parser.parse_args()

inputFilename=args.input
outputFilename=args.output

header = MicroappBinaryHeaderPacket()
if inputFilename != None:
    # The input file includes the header.
    with open(inputFilename, "rb") as f:
        buf = f.read()

    size = len(buf)

    # Fill fields from binary, as some fields are already set.
    header.fromBuffer(buf)
    print(f"Read header: {header}")

    # Set header fields that are not set yet.
    headerSize = len(header.toBuffer())

    header.startOffset = headerSize
    header.size = len(buf)

    # # Test value: remove on release:
    # header.appBuildVersion = 987654321

    header.checksum = crc16ccitt(buf[headerSize:])
    print(f"Calculate header checksum from: {header} {header.toBuffer()}")
    header.checksumHeader = crc16ccitt(bytearray(header.toBuffer()))
    print(f"Final header: {header}")

with open(outputFilename, "w") as outputFile:
    outputFile.write(f"APP_BINARY_SIZE = {header.size};\n")
    outputFile.write(f"CHECKSUM = {header.checksum};\n")
    outputFile.write(f"CHECKSUM_HEADER = {header.checksumHeader};\n")
    outputFile.write(f"APP_BUILD_VERSION = {header.appBuildVersion};\n")
    outputFile.write(f"START_OFFSET = {header.startOffset};\n")
    outputFile.write(f"HEADER_RESERVED = {header.reserved};\n")
    outputFile.write(f"HEADER_RESERVED2 = {header.reserved2};\n")
