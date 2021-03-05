#!/usr/bin/env python3

"""The microapp make-helper."""

import argparse

from crownstone_core.util import CRC
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

    size=len(buf)

    headerSize = len(header.toBuffer())

    header.startAddress = headerSize
    header.size = len(buf)

    # Test value: remove on release:
    header.appBuildVersion = 987654321

    header.checksum = CRC.CRC_16_CCITT.crc(buf[headerSize:])
    header.checksumHeader = CRC.CRC_16_CCITT.crc(bytearray(header.toBuffer()))

with open(outputFilename, "w") as outputFile:
    outputFile.write(f"START_ADDRESS = {header.startAddress};\n")
    outputFile.write(f"APP_BINARY_SIZE = {header.size};\n")
    outputFile.write(f"CHECKSUM = {header.checksum};\n")
    outputFile.write(f"CHECKSUM_HEADER = {header.checksumHeader};\n")
    outputFile.write(f"APP_BUILD_VERSION = {header.appBuildVersion};\n")
    outputFile.write(f"HEADER_RESERVED = {header.reserved};\n")
