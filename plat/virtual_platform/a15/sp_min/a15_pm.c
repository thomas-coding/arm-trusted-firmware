/*
 * Power Management for Alius LCS Domain
 *
 * Maintainer: Liuwei <Wei.Liu@verisilicon.com>
 *
 * Copyright (C) 2021 VeriSilicon Microelectronics (Shanghai) Co., Ltd.
 *
 */

#include <assert.h>
#include <arch_helpers.h>
#include <lib/psci/psci.h>

static int a15_pwr_domain_on(u_register_t mpidr)
{
	return 0;
}

static void a15_pwr_domain_on_finish(const psci_power_state_t *target_state)
{

}

const plat_psci_ops_t a15_psci_ops = {
	.pwr_domain_on		= a15_pwr_domain_on,
	.pwr_domain_on_finish	= a15_pwr_domain_on_finish,
};

int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	*psci_ops = &a15_psci_ops;

	return 0;
}
