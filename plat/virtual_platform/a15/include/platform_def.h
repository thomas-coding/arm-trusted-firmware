/*
 * Copyright (c) 2014-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <lib/utils_def.h>
#include <plat/common/common_def.h>

/* Thomas-a15 memory map
 *
 *  0x00000000 .. 0x01ffffff : Flash(32M)
 *  0x10000000 .. 0x11ffffff : SRAM(32M)
 *  0x20000000 .. 0x27ffffff : DDR(128M)
 *  0x40000000 .. 0x40000fff : UART(4K)
 *  0x50000000 .. 0x50000fff : Thomas Test Device(4K)
 *  0x60000000 .. 0x6000ffff : GIC Distributor(64K)
 *  0x61000000 .. 0x6001ffff : GIC Redistributor(128K)
 */

/* ATF memory map
 *  FLASH binary
 *  0x00000000 .. 0x00100000 : BL1 binary (1M)
 *  0x00100000 .. 0x001fffff : BL2 binary (1M)
 *
 *  RAM (4M)
 *  0x10300000 .. 0x103fffff : BL1 RW RAM (top 1M)
 *  0x10000000 .. 0x100fffff : BL2 Runing RAM (1M)
 */

#define PLATFORM_CORE_COUNT	U(1)

#define PLATFORM_STACK_SIZE		0x1000

//TODO
#define PLAT_MAX_OFF_STATE		U(2)
#define PLAT_MAX_RET_STATE		U(1)
#define PLAT_MAX_PWR_LVL		U(1)

#define BL_RAM_BASE			UL(0x10000000)
#define BL_RAM_SIZE			UL(4 * 1024 * 1024)

#define BL_FLASH_BASE			UL(0x00000000)
#define BL_FLASH_SIZE			UL(32 * 1024 * 1024)

/* BL1 1M SRAM */
/* bl1_calc_bl2_mem_layout assuming this is on top */
#define BL1_RW_BASE			UL(0x10300000)
#define BL1_RW_SIZE			UL(1024 * 1024)
#define BL1_RW_LIMIT		(BL1_RW_BASE + BL1_RW_SIZE)

/* BL1 1M FLASH*/
#define BL1_RO_BASE			UL(0x00000000)
#define BL1_RO_SIZE			UL(1024 * 1024)
#define BL1_RO_LIMIT		(BL1_RO_BASE + BL1_RO_SIZE)

/* BL2 1M FLASH and SRAM */
#define BL2_BINARY_BASE			UL(0x00100000)
#define BL2_BINARY_SIZE			UL(1024 * 1024)
#define BL2_BINARY_LIMIT		(BL2_BINARY_BASE + BL2_BINARY_SIZE)

#define BL2_BASE			UL(0x10000000)
#define BL2_SIZE			UL(1024 * 1024)
#define BL2_LIMIT			(BL2_BASE + BL2_SIZE)

//TODO
#define CACHE_WRITEBACK_GRANULE		64

//TODO
#define A15_PRIMARY_CPU			0x0

//TODO
#define MAX_IO_HANDLES			7
#define MAX_IO_DEVICES			7

//TODO
#define PLAT_VIRT_ADDR_SPACE_SIZE	(1ULL << 32)
#define PLAT_PHY_ADDR_SPACE_SIZE	(1ULL << 32)
#define MAX_MMAP_REGIONS		16
#define MAX_XLAT_TABLES			16

/*
 * PL011 related constants
 */
#define PL011_UART0_BASE		UL(0x40000000)
#define PL011_BAUDRATE			UL(115200)
#define PL011_UART_CLK_IN_HZ	UL(24000000)
#define PL011_UART_SIZE			UL(0x1000)

#endif /* PLATFORM_DEF_H */
