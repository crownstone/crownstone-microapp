#!/bin/bash

usage="Usage: $0 <input file> <output file>"

ifile=${1:? "$usage"}
ofile=${2:? "$usage"}

printf "\x99\xc0" | cat - "$ifile" > "$ofile"
