    .syntax unified
    .arch armv7e-m

// Define reset handler

    .text
    .thumb
    .thumb_func
    .align 1
    .globl Reset_Handler
    .type Reset_Handler, %function
Reset_Handler:

// Copy data from read only memory to RAM.

#ifndef __STARTUP_SKIP_ETEXT
    ldr r1, =__etext
    ldr r2, =__data_start__
    ldr r3, =__bss_start__

    subs r3, r3, r2
    ble .L_loop1_done

.L_loop1:
    subs r3, r3, #4
    ldr r0, [r1,r3]
    str r0, [r2,r3]
    bgt .L_loop1

.L_loop1_done:
#endif

#ifdef __STARTUP_CLEAR_BSS
    ldr r1, =__bss_start__
    ldr r2, =__bss_end__

    movs r0, 0

    subs r2, r2, r1
    ble .L_loop3_done

.L_loop3:
    subs r2, r2, #4
    str r0, [r1, r2]
    bgt .L_loop3

.L_loop3_done:
#endif

// execute main
//    bl main

// call _start (define yourself if no libs are used)
#ifndef __START
#define __START _start
#endif
    bl __START

    .pool
    .size   Reset_Handler,.-Reset_Handler

    .section ".text"

  .end
