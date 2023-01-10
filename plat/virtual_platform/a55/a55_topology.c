// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (C) 2020 VeriSilicon Holdings Co., Ltd.
 */

#include <arch.h>
#include <platform_def.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>

/* The power domain tree descriptor */
static unsigned char power_domain_tree_desc[] =
{
	1,			/* system level */
	PLATFORM_CLUSTER_COUNT,	/* cluster level */
	PLATFORM_CORE_COUNT,	/* cores level */
};

/*******************************************************************************
 * This function returns the ARM default topology tree information.
 ******************************************************************************/
const unsigned char *plat_get_power_domain_tree_desc(void)
{
	return power_domain_tree_desc;
}

/*******************************************************************************
 * This function implements a part of the critical interface between the psci
 * generic layer and the platform that allows the former to query the platform
 * to convert an MPIDR to a unique linear index. An error code (-1) is returned
 * in case the MPIDR is invalid.
 ******************************************************************************/
int plat_core_pos_by_mpidr(u_register_t mpidr)
{
	return (int) plat_arm_calc_core_pos(mpidr);
}
