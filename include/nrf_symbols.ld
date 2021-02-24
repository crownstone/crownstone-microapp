APPLICATION_START_ADDRESS = 0x00069000;
/* We allow for 4 pages of flash */
APPLICATION_LENGTH = 0x4000;

/* We place the microapp ram area before IPC ram. */
/* We use 2 kB of RAM (.data and .bss). */
/* These settings have to match the settings in firmware. */
RAM_APPLICATION_AMOUNT = 0x800;
RAM_END = 0x20010000;
RAM_BLUENET_IPC_LENGTH = 0x100;

