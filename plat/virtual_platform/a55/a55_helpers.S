// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (C) 2020 VeriSilicon Holdings Co., Ltd.
 */

#include <arch.h>
#include <asm_macros.S>
#include <drivers/arm/gicv2.h>
#include <drivers/arm/gicv3.h>
#include <platform_def.h>

#define MPIDR_EL1_AFF_WIDTH	8
#define MPIDR_EL1_AFF0_LSB	0
#define MPIDR_EL1_AFF1_LSB	8

	.globl	plat_my_core_pos
	.globl	plat_secondary_cold_boot_setup
	.globl	plat_get_my_entrypoint
	.globl	plat_is_my_cpu_primary
	.globl	plat_arm_calc_core_pos
	.globl	platform_mem_init
	.globl	plat_crash_console_init
	.globl	plat_crash_console_putc
	.globl	plat_crash_console_flush
	.globl	plat_invalidate_icache_all
	.globl  plat_report_exception
	.globl	plat_panic_handler

	/* -----------------------------------------------------
	 *  unsigned int plat_my_core_pos(void)
	 *  This function uses the plat_arm_calc_core_pos()
	 *  definition to get the index of the calling CPU.
	 * -----------------------------------------------------
	 */
func plat_my_core_pos
	mrs	x0, mpidr_el1
	b	plat_arm_calc_core_pos
endfunc plat_my_core_pos

	/* -----------------------------------------------------
	 * void plat_secondary_cold_boot_setup (void);
	 *
	 * This function performs any platform specific actions
	 * needed for a secondary cpu after a cold reset e.g
	 * mark the cpu's presence, mechanism to place it in a
	 * holding pen etc.
	 * -----------------------------------------------------
	 */
func plat_secondary_cold_boot_setup
	/* Calculate address of our hold entry */
	bl	plat_my_core_pos
	lsl	x0, x0, #PLAT_A55_HOLD_ENTRY_SHIFT
	mov_imm	x2, PLAT_A55_HOLD_BASE

	/* Wait until we have a go */
poll_mailbox:
	ldr	x1, [x2, x0]
	cbz	x1, 1f

	/* Clear the mailbox again ready for next time. */
	mov x1, #PLAT_A55_HOLD_STATE_WAIT
	str x1, [x2, x0]

	/* Jump to the provided entrypoint. */
	mov_imm	x0, PLAT_A55_MAILBOX_BASE
	ldr	x1, [x0]
	br	x1
1:
	wfe
	b	poll_mailbox
endfunc plat_secondary_cold_boot_setup

	/* -----------------------------------------------------
	 * void plat_get_my_entrypoint (void);
	 */
func plat_get_my_entrypoint
	/* Cold reset */
	mov	x0, #0
	ret
endfunc plat_get_my_entrypoint

	/* -----------------------------------------------------
	 * unsigned int plat_is_my_cpu_primary (void);
	 *
	 * Find out whether the current cpu is the primary
	 * cpu.
	 * -----------------------------------------------------
	 */
func plat_is_my_cpu_primary
	mrs	x0, mpidr_el1
	mov_imm	x1, MPIDR_AFFINITY_MASK
	and	x0, x0, x1
	cmp	x0, #PLAT_PRIMARY_CPU
	cset	w0, eq
	ret
endfunc plat_is_my_cpu_primary

	/* ---------------------------------------------------------------------
	 * unsigned int plat_arm_calc_core_pos(u_register_t mpidr)
	 *
	 * Function to calculate the core position on platform.
	 *
	 * ---------------------------------------------------------------------
	 */
func plat_arm_calc_core_pos
	and	x1, x0, #MPIDR_CPU_MASK
	and	x0, x0, #MPIDR_CLUSTER_MASK
	add	x0, x1, x0, LSR #6
	ret
endfunc plat_arm_calc_core_pos

	/* --------------------------------------------------------
	 * void platform_mem_init (void);
	 *
	 * we actually do not need any mem init work at the very
	 * beginning of boot
	 * --------------------------------------------------------
	 */
func platform_mem_init
	ret
endfunc platform_mem_init

	/* ---------------------------------------------
	 * int plat_crash_console_init(void)
	 * Function to initialize the crash console
	 * without a C Runtime to print crash report.
	 * Clobber list : x0, x1, x2
	 * ---------------------------------------------
	 */
func plat_crash_console_init
	mov_imm x0, PLAT_A55_UART_BASE
	mov_imm x1, PLAT_A55_UART_CLK_IN_HZ
	mov_imm x2, PLAT_A55_BAUDRATE
	b   console_16550_core_init
endfunc plat_crash_console_init

	/* ---------------------------------------------
	 * int plat_crash_console_putc(int c)
	 * Function to print a character on the crash
	 * console without a C Runtime.
	 * Clobber list : x1, x2
	 * ---------------------------------------------
	 */
func plat_crash_console_putc
	mov_imm x1, PLAT_A55_UART_BASE
	b   console_16550_core_putc
endfunc plat_crash_console_putc

	/* ---------------------------------------------
	 * int plat_crash_console_flush(int c)
	 * Function to force a write of all buffered
	 * data that hasn't been output.
	 * Out : return -1 on error else return 0.
	 * Clobber list : x0, x1
	 * ---------------------------------------------
	 */
func plat_crash_console_flush
	mov_imm x0, PLAT_A55_UART_BASE
	b   console_16550_core_flush
endfunc plat_crash_console_flush

	/*
	 * void plat_invalidate_icache_all(void)
	 * Disable all of the i caches
	 */
func plat_invalidate_icache_all
	ic	ialluis
	isb	sy
	ret
endfunc plat_invalidate_icache_all

	/* ----------------------------------------------------------
	 * void plat_report_exception(unsigned int exception_type)
	 * Function to show register information in exception.
	 * BL1 uses SP_EL0, but BL2 uses SP_ELx.
	 * ----------------------------------------------------------
	 */
func plat_report_exception
#ifdef IMAGE_BL1
	msr	spsel, #MODE_SP_EL0
#endif
	/* exit if SP is not initialized, we can't call c function */
	cmp	sp, #0
	beq	report_exit
	bl	a55_report_exception
report_exit:
#ifdef IMAGE_BL1
	msr	spsel, #MODE_SP_ELX
#endif
	ret
endfunc plat_report_exception

/*
 * void plat_panic_handler(void) __dead2
 * Endliss loop by default.
 */
func plat_panic_handler
	wfi
	b	   plat_panic_handler
endfunc plat_panic_handler
