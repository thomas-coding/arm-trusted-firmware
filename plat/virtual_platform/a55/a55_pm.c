// SPDX-License-Identifier: BSD-2-ClausePLAT_MAX_PWR_LVLPLAT_MAX_PWR_LVL
/*
 * Copyright (C) 2020 VeriSilicon Holdings Co., Ltd.
 */

#include <assert.h>
#include <arch_helpers.h>
#include <lib/psci/psci.h>
#include <plat/common/platform.h>
#include <platform_def.h>

static void a55_cpu_standby(plat_local_state_t cpu_state)
{
	u_register_t scr = read_scr_el3();

	write_scr_el3(scr | SCR_IRQ_BIT);

	isb();
	dsb();
	wfi();

	write_scr_el3(scr);
}

static int a55_pwr_domain_on(u_register_t mpidr)
{
	return PSCI_E_SUCCESS;
}

static void a55_pwr_domain_on_finish(const psci_power_state_t *target_state)
{

}

void a55_pwr_domain_off(const psci_power_state_t *target_state)
{

}

void a55_pwr_domain_suspend(const psci_power_state_t *target_state)
{

}

void a55_pwr_domain_suspend_finish(const psci_power_state_t *target_state)
{

}

/*
 * When code comes here, already send system suspend
 * scmi command to PMC. Now the core just enter WFI.
 */
void __dead2 a55_pwr_domain_pwr_down_wfi(const psci_power_state_t *target_state)
{
	while (1)
		wfi();
}

void __dead2 a55_system_reset(void)
{
	while (1)
		wfi();
}

void __dead2 a55_system_off(void)
{
	while (1)
		wfi();
}

static int a55_validate_power_state(unsigned int power_state,
			    psci_power_state_t *req_state)
{
	unsigned int pstate = psci_get_pstate_type(power_state);
	unsigned int pwr_lvl = psci_get_pstate_pwrlvl(power_state);
	unsigned int i;

	assert(req_state != NULL);

	if (pwr_lvl > PLAT_MAX_PWR_LVL)
		return PSCI_E_INVALID_PARAMS;

	/* Sanity check the requested state */
	if (pstate == PSTATE_TYPE_STANDBY) {
		/*
		 * It's possible to enter standby only on power level 0
		 * Ignore any other power level.
		 */
		if (pwr_lvl != MPIDR_AFFLVL0)
			return PSCI_E_INVALID_PARAMS;

		req_state->pwr_domain_state[MPIDR_AFFLVL0] =
					PLAT_MAX_RET_STATE;
	} else {
		for (i = MPIDR_AFFLVL0; i <= pwr_lvl; i++)
			req_state->pwr_domain_state[i] =
					PLAT_MAX_OFF_STATE;
	}

	/* We expect the 'state id' to be zero. */
	if (psci_get_pstate_id(power_state) != 0U)
		return PSCI_E_INVALID_PARAMS;

	return PSCI_E_SUCCESS;
}

static int a55_validate_ns_entrypoint(uintptr_t entrypoint)
{
	return PSCI_E_SUCCESS;
}

static void a55_get_sys_suspend_power_state(psci_power_state_t *req_state)
{
	unsigned int i;

	for (i = MPIDR_AFFLVL0; i <= PLAT_MAX_PWR_LVL; i++)
		req_state->pwr_domain_state[i] = PLAT_MAX_OFF_STATE;
}

static plat_psci_ops_t a55_psci_ops = {
	.cpu_standby			= a55_cpu_standby,
	.pwr_domain_on 			= a55_pwr_domain_on,
	.pwr_domain_on_finish 	= a55_pwr_domain_on_finish,
	.pwr_domain_off 		= a55_pwr_domain_off,
	.pwr_domain_suspend 	= a55_pwr_domain_suspend,
	.pwr_domain_suspend_finish = a55_pwr_domain_suspend_finish,
	.pwr_domain_pwr_down_wfi   = a55_pwr_domain_pwr_down_wfi,
	.system_reset			= a55_system_reset,
	.system_off			= a55_system_off,
	.validate_ns_entrypoint = a55_validate_ns_entrypoint,
	.validate_power_state 	= a55_validate_power_state,
	.get_sys_suspend_power_state 	= a55_get_sys_suspend_power_state,
};

int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	*psci_ops = &a55_psci_ops;

	INFO("%s: sec_entrypoint = 0x%lx\n", __func__,
		(uintptr_t)sec_entrypoint);

	return 0;
}
