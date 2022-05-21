/*
 * Power topology code implemention for Alius ACS domain.
 *
 * Maintainer: Liuwei <Wei.Liu@verisilicon.com>
 *
 * Copyright (C) 2021 VeriSilicon Microelectronics (Shanghai) Co., Ltd.
 *
 */

#include <arch.h>
#include <platform_def.h>

/* The power domain tree descriptor */
static unsigned char power_domain_tree_desc[] =
{
	1,			/* system level */
	PLATFORM_CLUSTER_COUNT,	/* cluster level */
	PLATFORM_CORE_COUNT,	/* cores level */
};

const unsigned char *plat_get_power_domain_tree_desc(void)
{
	return power_domain_tree_desc;
}

int plat_core_pos_by_mpidr(u_register_t mpidr)
{
	return 0;
}
