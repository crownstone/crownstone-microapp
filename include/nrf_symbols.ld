/* The following settings have to match that in the bluenet firmware */

/* We allow for 4 pages of FLASH. Reminder, 0x4000=16374 (bytes). */
APPLICATION_LENGTH = 0x4000;

/* We allocate for now 4 kB of RAM for a microapp (.data and .bss). Reminder, 0x1000=4096 (bytes). */
RAM_APPLICATION_AMOUNT = 0x1000;

/* Amount of RAM for interprocess communication (also used by bootloader). Reminder, 0x100=256. */
RAM_BLUENET_IPC_LENGTH = 0x100;
