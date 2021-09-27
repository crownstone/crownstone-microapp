Example scripts to show how `srec_cat` can be used to calculate the CRC16 CCITT according to the "BAD" implementation, see http://srecord.sourceforge.net/crc16-ccitt.html.

This is the one that is used on Nordic:

* CRC-16/AUG-CCITT

This is the one called `calc_nordic.sh`. Use this one!

	./calc_nordic.sh ../../build/example.bin.tmp

This is the one that is normally calculated by `srec_cat`:

* CRC-16/CCITT-FALSE

This is the one called `calc_naive.sh`. Do not use this one!

# Calculate all

You can calculate everything you need by:

	./calc_all.sh ../../build/example.bin.tmp

This does the following:

* Generate the header
    * Calculate the checksum using `calc_nordic.sh`
        * Remove header
        * Prepend bytes
        * Calculate CRC
    * Calculate size
    * Set checksum and size in right endianness
    * Add protocol version, header size and zeros
* Prepend bytes
* Calculate CRC for header
* Update header with CRC for header
* Remove header from original binary
* Concatenate updated header with headerless binary

Take a diff and you see that it does the same as the Makefile.


