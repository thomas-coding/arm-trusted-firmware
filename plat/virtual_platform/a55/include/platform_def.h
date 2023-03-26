/*
 * Copyright (c) 2014-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <lib/utils_def.h>
#include <plat/common/common_def.h>

/* Thomas-a55 memory map
 *
 *  0x10000000 .. 0x11ffffff : Flash(32M)
 *  0x20000000 .. 0x21ffffff : SRAM(32M)
 *  0x30000000 .. 0x3fffffff : DDR(256M)
 *  0x40000000 .. 0x40000fff : UART(4K)
 *  0x50000000 .. 0x50000fff : Thomas Test Device(4K)
 *  0x60000000 .. 0x6000ffff : GIC Distributor(64K)
 *  0x61000000 .. 0x6001ffff : GIC Redistributor(128K)
 */

/* ATF memory map
 *  FLASH binary
 *  0x10000000 .. 0x10100000 : BL1 binary (1M)
 *  0x10100000 .. 0x101fffff : BL2 binary (1M)
 *  0x10200000 .. 0x102fffff : BL32 binary (1M)
 *
 *  RAM (4M)
 *  0x20300000 .. 0x203fffff : BL1 RW RAM (top 1M)
 *  0x202f0000 .. 0x202f0fff : Shared RW RAM (4k)
 *  SPACE
 *  0x20000000 .. 0x200fffff : BL2 Runing RAM (1M)
 *  0x20100000 .. 0x201fffff : BL32 Runing RAM (1M)
 *
 *  0x21000000 .. 0x21ffffff : Binary location(load by kermit)
 */

#define PLATFORM_STACK_SIZE		0x1000

/* Core Clock Configurations */
#define PLAT_A55_OSC_CLK		24000000
#define PLAT_A55_CFG_APB_PLL_CLK	200000000

#define BL_RAM_BASE			UL(0x20000000)
#define BL_RAM_SIZE			UL(32 * 1024 * 1024)

#define BL_FLASH_BASE			UL(0x10000000)
#define BL_FLASH_SIZE			UL(32 * 1024 * 1024)

#define BL_DDR_BASE			UL(0x30000000)
#define BL_DDR_SIZE			UL(0x10000000)

/* BL1 1M SRAM */
/* bl1_calc_bl2_mem_layout assuming this is on sram top */
#define BL1_RW_BASE			(BL_RAM_BASE + BL_RAM_SIZE - 1024 * 1024)
#define BL1_RW_SIZE			UL(1024 * 1024)
#define BL1_RW_LIMIT		(BL1_RW_BASE + BL1_RW_SIZE)

#define BL_SHARE_MEM_SIZE		(4 * 1024)
#define BL_SHARE_MEM_BASE		(BL1_RW_BASE - BL_SHARE_MEM_SIZE)
#define BL_SHARE_LIMIT			(BL1_RW_BASE)

/* ATF share memory map (4k)
 *  0x202f0000 .. 0x202f0bff : Shared RW RAM (3k)
 *  0x202f0c00 .. 0x202f0fff : Mailbox (1k)
 *    0x202f0c00 .. 0x202f0c07 : warm boot entry (8 bytes)
 *    0x202f0c08 .. 0x202f0c0f : core0 hold (8 bytes)
 *    0x202f0c10 .. 0x202f0c17 : core1 hold (8 bytes)
 */
#define PLAT_A55_MAILBOX_SIZE	(1024)
#define PLAT_A55_MAILBOX_BASE	(BL_SHARE_LIMIT - PLAT_A55_MAILBOX_SIZE)

#define PLAT_A55_HOLD_BASE		(PLAT_A55_MAILBOX_BASE + 8)
#define PLAT_A55_HOLD_SIZE		(PLATFORM_CORE_COUNT * \
					 PLAT_A55_HOLD_ENTRY_SIZE)
#define PLAT_A55_HOLD_ENTRY_SHIFT	3
#define PLAT_A55_HOLD_ENTRY_SIZE	(1 << PLAT_QEMU_HOLD_ENTRY_SHIFT)
#define PLAT_A55_HOLD_STATE_WAIT	0
#define PLAT_A55_HOLD_STATE_GO		1


/* BL1 1M FLASH*/
#define BL1_RO_BASE			UL(0x10000000)
#define BL1_RO_SIZE			UL(1024 * 1024)
#define BL1_RO_LIMIT		(BL1_RO_BASE + BL1_RO_SIZE)

/* BL2 1M SRAM */
#define BL2_BASE			UL(0x20000000)
#define BL2_SIZE			UL(1024 * 1024)
#define BL2_LIMIT			(BL2_BASE + BL2_SIZE)

/* BL31 1M SRAM */
#define BL31_BASE			UL(0x20100000)
#define BL31_SIZE			UL(1024 * 1024)
#define BL31_LIMIT			(BL31_BASE + BL31_SIZE)

/* BL32(optee) 1M SRAM */
#define BL32_BASE			UL(0x30000000)
#define BL32_SIZE			UL(1024 * 1024)
#define BL32_LIMIT			(BL32_BASE + BL32_SIZE)

/* BL33(uboot) 1M SRAM*/
#define BL33_BASE			UL(0x31000000)
#define BL33_SIZE			UL(4 * 1024 * 1024)
#define BL33_LIMIT			(BL33_BASE + BL33_SIZE)

/* Binary memory layout (for kermit load) */
#define PLAT_A55_FIP_MEM_BASE	UL(0x21000000)
#define PLAT_A55_FIP_MEM_SIZE	UL(4 * 1024 * 1024)
#define PLAT_A55_FIP_MEM_LIMIT	(PLAT_A55_FIP_MEM_BASE + PLAT_A55_FIP_MEM_LIMIT)

#define A55_OPTEE_PAGEABLE_LOAD_BASE	UL(0x30000000)
#define A55_OPTEE_PAGEABLE_LOAD_SIZE UL(8 * 1024 * 1024)

//TODO
#define CACHE_WRITEBACK_GRANULE		64

//TODO
#define MAX_IO_HANDLES			7
#define MAX_IO_DEVICES			7
#define MAX_IO_BLOCK_DEVICES		4 /* eMMC, SD, SF, DDR */

//TODO
#define PLAT_VIRT_ADDR_SPACE_SIZE	(1ULL << 32)
#define PLAT_PHY_ADDR_SPACE_SIZE	(1ULL << 32)
#define MAX_MMAP_REGIONS		16
#define MAX_XLAT_TABLES			16

/*
 * NS16550 related constants
 */
#define PLAT_A55_UART_BASE		UL(0x40000000)
#define PLAT_A55_BAUDRATE			UL(115200)
#define PLAT_A55_UART_CLK_IN_HZ	UL(24000000)
#define PLAT_A55_UART_SIZE			UL(0x1000)

// GIC
#define PLAT_ARM_GICD_BASE		UL(0x60000000)
#define PLAT_ARM_GICD_SIZE		UL(0x10000)
#define PLAT_ARM_GICR_BASE		UL(0x61000000)
#define PLAT_ARM_GICR_SIZE		UL(0xF60000)

/* Interrupt numbers */
#define ARM_IRQ_SEC_PHY_TIMER		29

#define ARM_IRQ_SEC_SGI_0		8
#define ARM_IRQ_SEC_SGI_1		9
#define ARM_IRQ_SEC_SGI_2		10
#define ARM_IRQ_SEC_SGI_3		11
#define ARM_IRQ_SEC_SGI_4		12
#define ARM_IRQ_SEC_SGI_5		13
#define ARM_IRQ_SEC_SGI_6		14
#define ARM_IRQ_SEC_SGI_7		15

#define PLAT_ARM_G1S_IRQ_PROPS(grp) \
	INTR_PROP_DESC(ARM_IRQ_SEC_PHY_TIMER, GIC_HIGHEST_SEC_PRIORITY, (grp), \
		GIC_INTR_CFG_LEVEL), \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_0, GIC_HIGHEST_SEC_PRIORITY, (grp), \
		GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_1, GIC_HIGHEST_SEC_PRIORITY, (grp), \
		GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_2, GIC_HIGHEST_SEC_PRIORITY, (grp), \
		GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_3, GIC_HIGHEST_SEC_PRIORITY, (grp), \
		GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_4, GIC_HIGHEST_SEC_PRIORITY, (grp), \
		GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_5, GIC_HIGHEST_SEC_PRIORITY, (grp), \
		GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_6, GIC_HIGHEST_SEC_PRIORITY, (grp), \
		GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_7, GIC_HIGHEST_SEC_PRIORITY, (grp), \
		GIC_INTR_CFG_EDGE)

#define PLAT_ARM_G0_IRQ_PROPS(grp)

/* DynamIQ */
#define PLATFORM_CLUSTER_COUNT		1
#define PLATFORM_CORE_COUNT		8

#define PLAT_MAX_CPUS_PER_CLUSTER	8
#define PLAT_MAX_PE_PER_CPU		1
#define PLAT_PRIMARY_CPU		0x0

/* TODO */
/* Dummy Power Management */
#define PWRC_BASE			0x1c100000
#define PLAT_NUM_PWR_DOMAINS		(1 + PLATFORM_CLUSTER_COUNT + \
					 PLATFORM_CORE_COUNT)
#define PLAT_MAX_PWR_LVL		U(2)
#define PLAT_MAX_OFF_STATE		U(2)
#define PLAT_MAX_RET_STATE		U(1)

/* SDHCI */
#define PLAT_A55_SD_BASE		UL(0x70000000)
#define PLAT_A55_SD_SIZE		UL(0x00010000)
#define PLAT_A55_SD_CLK_RATE	4000000

/* SD */
#define PLAT_A55_IMAGE_SD_BASE		(40 * 512)
#define PLAT_A55_IMAGE_SD_SIZE		(512 * 8 * 1024)

//TODO: FIXME: read 8k once fail
#define PLAT_A55_BLK_BUF_SIZE		(512)//(8 * 1024)

#endif /* PLATFORM_DEF_H */
