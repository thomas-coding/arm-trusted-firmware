/*
 * Declaration of LCS private functions.
 *
 * Maintainer: Liuwei <Wei.Liu@verisilicon.com>
 *
 * Copyright (C) 2021 VeriSilicon Microelectronics (Shanghai) Co., Ltd.
 *
 */

#ifndef A55_PRIVATE_H
#define A55_PRIVATE_H

void a55_configure_mmu_el1(unsigned long total_base, unsigned long total_size,
			unsigned long code_start, unsigned long code_limit);

void a55_configure_mmu_el3(unsigned long total_base, unsigned long total_size,
			unsigned long code_start, unsigned long code_limit);

void a55_io_setup(void);

uint32_t a55_get_spsr_for_bl33_entry(void);

#if SECURE_DEBUG
int verify_debug_cert(void);
#endif

#endif /* A55_PRIVATE_H */
