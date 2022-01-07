# The location we will write into (in flash)

# For nRF52832
START_ADDRESS_WITHOUT_PREFIX=69000

# The location including 0x
START_ADDRESS=0x$(START_ADDRESS_WITHOUT_PREFIX)

# The end of RAM
RAM_END=0x20010000
