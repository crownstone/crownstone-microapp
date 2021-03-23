/* Linker script for Nordic Semiconductor nRF devices
 *
 * Version: Sourcery G++ 4.5-1
 * Support: https://support.codesourcery.com/GNUToolchain/
 *
 * Copyright (c) 2007, 2008, 2009, 2010 CodeSourcery, Inc.
 *
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions.  No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 */
OUTPUT_FORMAT ("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")

INCLUDE "microapp_symbols.ld"
INCLUDE "microapp_header_symbols.ld"

/* Linker script to place sections and symbol values. Should be used together
 * with other linker script that defines memory regions FLASH and RAM.
 * It references following symbols, which must be defined in code:
 *   Reset_Handler : Entry of reset handler
 *
 * It defines following symbols, which code can use without definition:
 *   __exidx_start
 *   __exidx_end
 *   __etext
 *   __data_start__
 *   __preinit_array_start
 *   __preinit_array_end
 *   __init_array_start
 *   __init_array_end
 *   __fini_array_start
 *   __fini_array_end
 *   __data_end__
 *   __bss_start__
 *   __bss_end__
 *   __end__
 *   end
 *   __HeapBase
 *   __HeapLimit
 *   __StackLimit
 *   __StackTop
 *   __stack
 */
ENTRY(main)

SECTIONS
{
	.firmware_header :
	{
		. = ALIGN(4);
		KEEP(*(.firmware_header))
		BYTE(__SDK_VERSION_MAJOR)
		BYTE(__SDK_VERSION_MINOR)
		SHORT(APP_BINARY_SIZE)
		SHORT(CHECKSUM)
		SHORT(CHECKSUM_HEADER)
		LONG(APP_BUILD_VERSION)
		SHORT(START_OFFSET)
		SHORT(HEADER_RESERVED)
		LONG(HEADER_RESERVED2)
		. = ALIGN(4);
	} > FLASH

	.text :
	{
		KEEP(*(.isr_vector))
		*(.text*)

		KEEP(*(.init))
		KEEP(*(.fini))

		/* .ctors */
		*crtbegin.o(.ctors)
		*crtbegin?.o(.ctors)
		*(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors)
		*(SORT(.ctors.*))
		*(.ctors)

		/* .dtors */
		*crtbegin.o(.dtors)
		*crtbegin?.o(.dtors)
		*(EXCLUDE_FILE(*crtend?.o *crtend.o) .dtors)
		*(SORT(.dtors.*))
		*(.dtors)

		*(.rodata*)

		KEEP(*(.eh_frame*))
} > FLASH

.ARM.extab :
{
	*(.ARM.extab* .gnu.linkonce.armextab.*)
} > FLASH

__exidx_start = .;
.ARM.exidx :
{
	*(.ARM.exidx* .gnu.linkonce.armexidx.*)
} > FLASH
__exidx_end = .;

__etext = .;

.data : AT (__etext)
{
	__data_start__ = .;
	*(vtable)
		*(.data*)

		. = ALIGN(4);
		/* preinit data */
		PROVIDE_HIDDEN (__preinit_array_start = .);
		KEEP(*(.preinit_array))
		PROVIDE_HIDDEN (__preinit_array_end = .);

		. = ALIGN(4);
		/* init data */
		PROVIDE_HIDDEN (__init_array_start = .);
		KEEP(*(SORT(.init_array.*)))
		KEEP(*(.init_array))
		PROVIDE_HIDDEN (__init_array_end = .);


		. = ALIGN(4);
		/* finit data */
		PROVIDE_HIDDEN (__fini_array_start = .);
		KEEP(*(SORT(.fini_array.*)))
		KEEP(*(.fini_array))
		PROVIDE_HIDDEN (__fini_array_end = .);

		KEEP(*(.jcr*))
		. = ALIGN(4);
		/* All data end */
		__data_end__ = .;

} > RAM

.bss :
{
	. = ALIGN(4);
	__bss_start__ = .;
	*(.bss*)
		*(COMMON)
		. = ALIGN(4);
		__bss_end__ = .;
} > RAM

.heap (COPY):
{
	__HeapBase = .;
	__end__ = .;
	PROVIDE(end = .);
	KEEP(*(.heap*))
	__HeapLimit = .;
} > RAM

/* .stack_dummy section doesn't contains any symbols. It is only
 * used for linker to calculate size of stack sections, and assign
 * values to stack symbols later */
.stack_dummy (COPY):
{
	KEEP(*(.stack*))
} > RAM

/*
 * There is startup code required before you can use data in the .data
 * section. You might also want to initialize .bss to zeros (just to
 * convenience the developer). Alternatively, the bootloader (in this
 * case the Crownstone firmware) might already initialize this to zero
 * before the microapp is started.
 *
 * The .data copy should be from the end of the .text section, which is
 * at __etext to the __data_start__ up to __data_end__.
 */
}
