#!/bin/bash

usage="Usage: $0 <input file> <output file>"

ifile=${1:? "$usage"}
ofile=${2:? "$usage"}

# Just get the header by getting the first 20 bytes
head -c 20 "$ifile" > "$ofile"
