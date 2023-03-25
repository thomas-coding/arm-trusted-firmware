/*
 * Contains the functions support Synopsys DesignWare Cores
 *
 */

#ifndef DWC_MSHC_H
#define DWC_MSHC_H
#include <mmc.h>

#define BLOCK_SIZE		512

#define SDHCI_MAX_DIV		1024

#define MMC_RSP_PRESENT		(1 << 0)

#define SDHCI_DMA_ADDRESS	0x00
#define SDHCI_BLOCK_SIZE	0x04
#define SDHCI_SDMA_BUF_BDARY_512K 0x7
#define SDHCI_SDMA_BUF_BDARY_SIZE (512 * 1024)
#define SDHCI_BLOCK_COUNT	0x06
#define SDHCI_ARGUMENT		0x08
#define SDHCI_TRANSFER_MODE	0x0c
#define SDHCI_TRNS_DMA		0x01
#define SDHCI_TRNS_BLK_CNT_EN	0x02
#define SDHCI_TRNS_AUTO_CMD12	0x04
#define SDHCI_TRNS_AUTO_CMD23	0x08
#define SDHCI_TRNS_READ		0x10
#define SDHCI_TRNS_MULTI	0x20
#define SDHCI_COMMAND		0x0e
#define SDHCI_CMD_RESP_MASK	0x03
#define SDHCI_CMD_CRC		0x08
#define SDHCI_CMD_INDEX		0x10
#define SDHCI_CMD_DATA		0x20
#define SDHCI_CMD_ABORTCMD	0xc0

#define SDHCI_CMD_RESP_NONE	0x00
#define SDHCI_CMD_RESP_LONG	0x01
#define SDHCI_CMD_RESP_SHORT	0x02
#define SDHCI_CMD_RESP_SHORT_BUSY 0x03

#define SDHCI_MAKE_CMD(c, f)	(((c & 0xff) << 8) | (f & 0xff))

#define SDHCI_RESPONSE		0x10

#define SDHCI_PRESENT_STATE	0x24
#define SDHCI_CMD_INHIBIT	0x00000001
#define SDHCI_DATA_INHIBIT	0x00000002
#define SDHCI_DOING_WRITE	0x00000100
#define SDHCI_DOING_READ	0x00000200
#define SDHCI_SPACE_AVAILABLE	0x00000400
#define SDHCI_DATA_AVAILABLE	0x00000800
#define SDHCI_CARD_PRESENT	0x00010000
#define SDHCI_WRITE_PROTECT	0x00080000
#define SDHCI_DATA_LVL_MASK	0x00f00000
#define SDHCI_DATA_LVL_SHIFT	20
#define SDHCI_DATA_0_LVL_MASK	0x00100000
#define SDHCI_CMD_LVL		0x01000000

#define SDHCI_HOST_CONTROL	0x28
#define SDHCI_CTRL_LED		0x01
#define SDHCI_CTRL_4BITBUS	0x02
#define SDHCI_CTRL_HISPD	0x04
#define SDHCI_CTRL_DMA_MASK	0x18
#define SDHCI_CTRL_SDMA		0x00
#define SDHCI_CTRL_ADMA1	0x08
#define SDHCI_CTRL_ADMA32	0x10
#define SDHCI_CTRL_ADMA64	0x18
#define SDHCI_CTRL_8BITBUS	0x20
#define SDHCI_CTRL_CDTEST_INS	0x40
#define SDHCI_CTRL_CDTEST_EN	0x80

#define SDHCI_PWR_CONTROL	0x29
#define SD_BUS_PWR_VDD1_ON	0x1

#define SDHCI_CLOCK_CONTROL	0x2c
#define SDHCI_DIVIDER_SHIFT	8
#define SDHCI_DIVIDER_HI_SHIFT	6
#define SDHCI_DIV_MASK		0xff
#define SDHCI_DIV_MASK_LEN	8
#define SDHCI_DIV_HI_MASK	0x300
#define SDHCI_PROG_CLOCK_MODE	0x0020
#define SDHCI_CLOCK_CARD_EN	0x0004
#define SDHCI_CLOCK_INT_STABLE	0x0002
#define SDHCI_CLOCK_INT_EN	0x0001

#define SDHCI_TIMEOUT_CONTROL	0x2e

#define SDHCI_SOFTWARE_RESET	0x2f
#define SDHCI_RESET_ALL		0x01
#define SDHCI_RESET_CMD		0x02
#define SDHCI_RESET_DATA	0x04

#define SDHCI_INT_STATUS		0x30
#define SDHCI_INT_ENABLE	0x34
#define SDHCI_SIGNAL_ENABLE	0x38
#define SDHCI_INT_RESPONSE	0x00000001
#define SDHCI_INT_DATA_END	0x00000002
#define SDHCI_INT_BLK_GAP	0x00000004
#define SDHCI_INT_DMA_END	0x00000008
#define SDHCI_INT_SPACE_AVAIL	0x00000010
#define SDHCI_INT_DATA_AVAIL	0x00000020
#define SDHCI_INT_CARD_INSERT	0x00000040
#define SDHCI_INT_CARD_REMOVE	0x00000080
#define SDHCI_INT_CARD_INT	0x00000100
#define SDHCI_INT_RETUNE	0x00001000
#define SDHCI_INT_CQE		0x00004000
#define SDHCI_INT_ERROR		0x00008000
#define SDHCI_INT_TIMEOUT	0x00010000
#define SDHCI_INT_CRC		0x00020000
#define SDHCI_INT_END_BIT	0x00040000
#define SDHCI_INT_INDEX		0x00080000
#define SDHCI_INT_DATA_TIMEOUT	0x00100000
#define SDHCI_INT_DATA_CRC	0x00200000
#define SDHCI_INT_DATA_END_BIT	0x00400000
#define SDHCI_INT_BUS_POWER	0x00800000
#define SDHCI_INT_ACMD12ERR	0x01000000
#define SDHCI_INT_ADMA_ERROR	0x02000000

#define SDHCI_INT_CMD_MASK (SDHCI_INT_RESPONSE | SDHCI_INT_TIMEOUT | \
	SDHCI_INT_CRC | SDHCI_INT_END_BIT | SDHCI_INT_INDEX)
#define SDHCI_INT_DATA_MASK (SDHCI_INT_DATA_END | SDHCI_INT_DMA_END | \
	SDHCI_INT_DATA_AVAIL | SDHCI_INT_SPACE_AVAIL | \
	SDHCI_INT_DATA_TIMEOUT | SDHCI_INT_DATA_CRC | \
	SDHCI_INT_DATA_END_BIT | SDHCI_INT_ADMA_ERROR | \
	SDHCI_INT_BLK_GAP)
#define SDHCI_INT_ALL_MASK ((unsigned int) - 1)

#define SDHCI_HOST_CONTROL2	0x3e
#define SDHCI_CTRL_HOST_VER4_EN	0x1000
#define SDHCI_CTRL_64BIT_ADDR	0x2000
#define SDHCI_CTRL_SIG_EN_1V8	0x0008

#define SDHCI_ADMA_ADDRESS	0x58
#define SDHCI_ADMA_ADDRESS_HI	0x5c

typedef struct dwc_mshc_params {
	uintptr_t       reg_base;
	int             base_clk;
	int             clk_rate;
	int             bus_width;
	unsigned int    flags;
	struct mmc_device_info info;
	void (*plat_init)(uintptr_t base);
} dwc_mshc_params_t;

void dwc_mshc_init(dwc_mshc_params_t *params);

#endif /* DWC_MSHC_H */
