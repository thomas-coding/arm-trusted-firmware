/*
 * Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>
#include <asm_macros.S>

	.globl	plat_get_my_entrypoint
	.globl	plat_is_my_cpu_primary
	.globl	plat_secondary_cold_boot_setup

	.globl	plat_crash_console_init	
	.globl	plat_crash_console_putc
	.globl	plat_crash_console_flush

	/* For sp-min */
	.globl	plat_arm_calc_core_pos
	.globl	plat_my_core_pos

	/* -----------------------------------------------------
	 *  unsigned int plat_my_core_pos(void)
	 *  This function uses the plat_arm_calc_core_pos()
	 *  definition to get the index of the calling CPU.
	 * -----------------------------------------------------
	 */
func plat_my_core_pos
	ldcopr	r0, MPIDR
	b	plat_arm_calc_core_pos
endfunc plat_my_core_pos

	/* -----------------------------------------------------
	 *  unsigned int plat_arm_calc_core_pos(uint64_t mpidr)
	 *  Helper function to calculate the core position.
	 *  With this function: CorePos = CoreId
	 * -----------------------------------------------------
	 */
func plat_arm_calc_core_pos
	and	r0, r0, #MPIDR_CPU_MASK
	bx	lr
endfunc plat_arm_calc_core_pos


	/* ---------------------------------------------------------------------
	 * unsigned long plat_get_my_entrypoint (void);
	 *
	 * Main job of this routine is to distinguish between a cold and warm
	 * boot. 
	 * ---------------------------------------------------------------------
	 */
func plat_get_my_entrypoint
	mov	r0, #0
	bx	lr
endfunc plat_get_my_entrypoint

	/* -----------------------------------------------------
	 * unsigned int plat_is_my_cpu_primary (void);
	 *
	 * Find out whether the current cpu is the primary
	 * cpu.
	 * -----------------------------------------------------
	 */
func plat_is_my_cpu_primary
	ldcopr	r0, MPIDR
	ldr	r1, =0xff	/* Aff0 */
	and	r0, r1
	cmp	r0, #A15_PRIMARY_CPU
	moveq	r0, #1
	movne	r0, #0
	bx	lr
endfunc plat_is_my_cpu_primary

	/* --------------------------------------------------------------------
	 * void plat_secondary_cold_boot_setup (void);
	 *
	 * For AArch32, cold-booting secondary CPUs is not yet
	 * implemented and they panic.
	 * --------------------------------------------------------------------
	 */
func plat_secondary_cold_boot_setup
cb_panic:
	b	cb_panic
endfunc plat_secondary_cold_boot_setup

func plat_crash_console_init
	bx	lr
endfunc plat_crash_console_init

func plat_crash_console_putc
	bx lr
endfunc plat_crash_console_putc

func plat_crash_console_flush
	bx lr
endfunc plat_crash_console_flush
