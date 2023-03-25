/*
 *  Contains the functions support Synopsys DesignWare Cores
 *  Mobile Storage Host Controller - DWC_mshc �C Product Code: A555-0.
 *  DesignWare Cores Mobile Storage Host Controller 1.70a
 *
 *
 */

#include <arch_helpers.h>
//#include <debug.h>
#include <dwc_mshc.h>
#include <errno.h>
#include <delay_timer.h>
#include <mmio.h>
#include <assert.h>

#define USE_32BIT_SDMA

static void dwc_mshc_initialize(void);
static int dwc_mshc_send_cmd(struct mmc_cmd *cmd);
static int dwc_mshc_set_ios(unsigned int clk, unsigned int width);
static int dwc_mshc_prepare(int lba, uintptr_t buf, size_t size);
static int dwc_mshc_read(int lba, uintptr_t buf, size_t size);
static int dwc_mshc_write(int lba, uintptr_t buf, size_t size);

static const struct mmc_ops dwc_mshc_ops = {
	.init		= dwc_mshc_initialize,
	.send_cmd	= dwc_mshc_send_cmd,
	.set_ios	= dwc_mshc_set_ios,
	.prepare	= dwc_mshc_prepare,
	.read		= dwc_mshc_read,
	.write		= dwc_mshc_write,
};

static dwc_mshc_params_t dwc_mshc_params;
#ifdef USE_32BIT_SDMA
static uintptr_t start_addr;
#endif

static inline bool is_emmc()
{
	return dwc_mshc_params.info.mmc_dev_type == MMC_IS_EMMC ? true : false;
}

static void mshc_writel(unsigned int val, int reg)
{
	mmio_write_32(dwc_mshc_params.reg_base + reg, val);
}

static void mshc_writew(unsigned short val, int reg)
{
	mmio_write_16(dwc_mshc_params.reg_base + reg, val);
}

static void mshc_writeb(unsigned char val, int reg)
{
	mmio_write_8(dwc_mshc_params.reg_base + reg, val);
}

static unsigned int mshc_readl(int reg)
{
	return mmio_read_32(dwc_mshc_params.reg_base + reg);
}

static unsigned short mshc_readw(int reg)
{
	return mmio_read_16(dwc_mshc_params.reg_base + reg);
}

static unsigned char mshc_readb(int reg)
{
	return mmio_read_8(dwc_mshc_params.reg_base + reg);
}

static void dwc_mshc_reset(unsigned char mask)
{
	unsigned long timeout = 100;
	mshc_writeb(mask, SDHCI_SOFTWARE_RESET);
	while (mshc_readb(SDHCI_SOFTWARE_RESET) & mask) {
		if (timeout == 0) {
			ERROR("%s: Reset 0x%x never completed.\n",
				   __func__, (int)mask);
			return;
		}
		timeout--;
		mdelay(1);
	}
}

static void sdhci_cmd_done(struct mmc_cmd *cmd)
{
	int i = 0;
	if (cmd->resp_type & MMC_RSP_136) {
		/* CRC is stripped so we need to do some shifting. */
		for (i = 0; i < 4; i++) {
			cmd->resp_data[i] =
				((mshc_readl(SDHCI_RESPONSE + i * 4)) << 8);
			if (i != 0) {
				cmd->resp_data[i] |=
				mshc_readb(SDHCI_RESPONSE + i * 4 - 1);
			}
		}
	} else {
		cmd->resp_data[0] = mshc_readl(SDHCI_RESPONSE);
	}
}

__unused static void dwc_mshci_registers_dump(void)
{
	printf("HOST_CONTROL:	0x%x\n", mshc_readb(SDHCI_HOST_CONTROL));
	printf("HOST_CONTROL2:	0x%x\n", mshc_readw(SDHCI_HOST_CONTROL2));
	printf("COMMAND:	0x%x\n", mshc_readw(SDHCI_COMMAND));
	printf("ARGUMENT:	0x%x\n", mshc_readl(SDHCI_ARGUMENT));
	printf("TRANSFER_MODE:	0x%x\n", mshc_readw(SDHCI_TRANSFER_MODE));
	printf("BLOCK_COUNT:	0x%x\n", mshc_readw(SDHCI_BLOCK_COUNT));
	printf("BLOCK_SIZE:	0x%x\n", mshc_readw(SDHCI_BLOCK_SIZE));
	printf("DMA_ADDRESS:	0x%x\n", mshc_readl(SDHCI_DMA_ADDRESS));
	printf("ADMA_ADDRESS:	0x%x\n", mshc_readl(SDHCI_ADMA_ADDRESS));
	printf("ADMA_ADDRESS_H:	0x%x\n", mshc_readl(SDHCI_ADMA_ADDRESS_HI));
}

/*
 * No command will be sent by driver if card is busy, so driver must wait
 * for card ready state.
 * Every time when card is busy after timeout then (last) timeout value will be
 * increased twice but only if it doesn't exceed global defined maximum.
 * Each function call will use last timeout value. Max timeout can be redefined
 * in board config file.
 */
#define CONFIG_SDHCI_CMD_MAX_TIMEOUT		32000
#define CONFIG_SDHCI_CMD_DEFAULT_TIMEOUT	100
#define SDHCI_READ_STATUS_TIMEOUT		1000
#define TRANSFER_TIMEOUT			1000000

int dwc_mshc_send_cmd(struct mmc_cmd *cmd)
{
	unsigned int stat = 0;
	unsigned int mask = 0;
	unsigned int time = 0;
	unsigned short xfer_mode = 0;
	unsigned char flags = 0;
	unsigned int cmd_timeout = CONFIG_SDHCI_CMD_DEFAULT_TIMEOUT;

	mask = (SDHCI_CMD_INHIBIT | SDHCI_DATA_INHIBIT);

	/* We shouldn't wait for data inihibit for stop commands, even
	 * though they might use busy signaling
	 */
	if (cmd->cmd_idx == MMC_CMD(12)) {
		mask &= (~SDHCI_DATA_INHIBIT);
	}

	while (mshc_readl(SDHCI_PRESENT_STATE) & mask) {
		if (time >= cmd_timeout) {
			WARN("%s: MMC: busy ", __func__);
			if (2 * cmd_timeout <= CONFIG_SDHCI_CMD_MAX_TIMEOUT) {
				cmd_timeout += cmd_timeout;
				WARN("timeout increasing to: %u ms.\n",
					   cmd_timeout);
			} else {
				ERROR("timeout.\n");
				return -EIO;
			}
		}
		time++;
		mdelay(1);
	}

	mshc_writel(SDHCI_INT_ALL_MASK, SDHCI_INT_STATUS);

	switch (cmd->cmd_idx) {
	case MMC_CMD(1):
		cmd->cmd_arg |= OCR_POWERUP;
		break;
	case MMC_CMD(18):
		xfer_mode |= (SDHCI_TRNS_MULTI | SDHCI_TRNS_BLK_CNT_EN);
	case MMC_CMD(17):
	case MMC_ACMD(51):
		flags |= SDHCI_CMD_DATA;
		xfer_mode |= (SDHCI_TRNS_READ | SDHCI_TRNS_DMA |
				SDHCI_TRNS_BLK_CNT_EN);
		break;
	case MMC_CMD(8):
		/*
		 * When the card type is eMMC, the CMD8 appears as SEND_EXT_CSD,
		 * and data transmission is followed with command. But CMD8 is
		 * SEND_IF_COND for SD, there is no data transmission followed
		 * the command.
		 */
		if (is_emmc()) {
			flags |= SDHCI_CMD_DATA;
			xfer_mode |= (SDHCI_TRNS_READ | SDHCI_TRNS_DMA);
		}
		break;
	case MMC_CMD(25):
		xfer_mode |= (SDHCI_TRNS_MULTI | SDHCI_TRNS_BLK_CNT_EN);
	case MMC_CMD(24):
		flags |= SDHCI_CMD_DATA;
		xfer_mode |= SDHCI_TRNS_DMA;
		break;
	default:
		xfer_mode = 0;
		break;
	}

	mshc_writel(cmd->cmd_arg, SDHCI_ARGUMENT);
	mshc_writew(xfer_mode, SDHCI_TRANSFER_MODE);

	mask = SDHCI_INT_RESPONSE;
	if (!(cmd->resp_type & MMC_RSP_PRESENT)) {
		flags |= SDHCI_CMD_RESP_NONE;
	} else if (cmd->resp_type & MMC_RSP_136) {
		flags |= SDHCI_CMD_RESP_LONG;
	} else if (cmd->resp_type & MMC_RSP_BUSY) {
		flags |= SDHCI_CMD_RESP_SHORT_BUSY;
		mask |= SDHCI_INT_DATA_END;
	} else {
		flags |= SDHCI_CMD_RESP_SHORT;
	}

	if (cmd->resp_type & MMC_RSP_CRC) {
		flags |= SDHCI_CMD_CRC;
	}
	if (cmd->resp_type & MMC_RSP_CMD_IDX) {
		flags |= SDHCI_CMD_INDEX;
	}

	mshc_writew(SDHCI_MAKE_CMD(cmd->cmd_idx, flags), SDHCI_COMMAND);

	time = 0;
	do {
		stat = mshc_readl(SDHCI_INT_STATUS);
		if (stat & SDHCI_INT_ERROR) {
			break;
		}
		time++;
		mdelay(1);
	} while (((stat & mask) != mask) && (time < SDHCI_READ_STATUS_TIMEOUT));

	if (time >= SDHCI_READ_STATUS_TIMEOUT) {
		ERROR("%s: Timeout for status update!\n", __func__);
		goto err;
	}

	if ((stat & (SDHCI_INT_ERROR | mask)) == mask) {
		sdhci_cmd_done(cmd);
	} else {
		ERROR("%s: CMD(%d) failed! 0x%x\n", __func__,
						cmd->cmd_idx, stat);
		goto err;
	}
	if ((flags & SDHCI_CMD_DATA) != 0) {
		time = 0;
		do {
			stat = mshc_readl(SDHCI_INT_STATUS);

			if ((stat & SDHCI_INT_DATA_END) != 0) {
				break;
			} else if ((stat & SDHCI_INT_DMA_END) != 0) {
				mshc_writel(SDHCI_INT_DMA_END,
						SDHCI_INT_STATUS);
#ifndef USE_32BIT_SDMA
				mshc_writel(mshc_readl(SDHCI_ADMA_ADDRESS),
						SDHCI_ADMA_ADDRESS);
				mshc_writel(mshc_readl(SDHCI_ADMA_ADDRESS_HI),
						SDHCI_ADMA_ADDRESS_HI);
#else
				start_addr &= ~(SDHCI_SDMA_BUF_BDARY_SIZE - 1);
				start_addr += SDHCI_SDMA_BUF_BDARY_SIZE;
				mshc_writel(start_addr, SDHCI_DMA_ADDRESS);
#endif
			}

			time++;
			udelay(10);
		} while (!(stat & SDHCI_INT_DATA_END) && (time < TRANSFER_TIMEOUT));

		if (time >= TRANSFER_TIMEOUT) {
			ERROR("Data timeout\n");
			goto err;
		}
	}
	mshc_writel(SDHCI_INT_ALL_MASK, SDHCI_INT_STATUS);

	return 0;
err:
	dwc_mshc_reset(SDHCI_RESET_CMD);
	dwc_mshc_reset(SDHCI_RESET_DATA);
	mshc_writel(SDHCI_INT_ALL_MASK, SDHCI_INT_STATUS);
	if (stat & SDHCI_INT_TIMEOUT)
		return -ETIMEDOUT;
	else
		return -EIO;
}

static void dwc_mshc_set_clock(unsigned int clk)
{
	unsigned int div = 0;
	unsigned int clock = 0;
	unsigned int timeout = 0;
	unsigned int reg = 0;

	timeout = 200;
	while (mshc_readl(SDHCI_PRESENT_STATE) &
			   (SDHCI_CMD_INHIBIT | SDHCI_DATA_INHIBIT)) {
		if (timeout == 0) {
			ERROR("%s: Timeout to wait cmd & data inhibit\n",
				   __func__);
			return;
		}

		timeout--;
		mdelay(1);
	}

	reg = mshc_readw(SDHCI_CLOCK_CONTROL);
	reg &= (~SDHCI_CLOCK_CARD_EN);
	mshc_writew(reg, SDHCI_CLOCK_CONTROL);

	if (clk == 0)
		return;
#if FIX_PROG_CLK_MODE
	/* Programmable Clock Mode : base clock / (N + 1)= sdhci clock */
	for (div = 1; div < SDHCI_MAX_DIV; div++) {
		if ((dwc_mshc_params.base_clk / div) <= clk)
			break;
	}
	div -= 1;
#else
	/* Divided Clock Mode : base clock / (2N) = sdhci clock */
	for (div = 1; div < SDHCI_MAX_DIV; div++) {
		if ((dwc_mshc_params.base_clk / (div * 2)) <= clk)
			break;
	}
#endif
	clock = ((div & SDHCI_DIV_MASK) << SDHCI_DIVIDER_SHIFT);
	clock |= (((div & SDHCI_DIV_HI_MASK) >> SDHCI_DIV_MASK_LEN)
				<< SDHCI_DIVIDER_HI_SHIFT);
	clock |= SDHCI_CLOCK_INT_EN;
	mshc_writew(clock, SDHCI_CLOCK_CONTROL);

	timeout = 150;
	while (!((clock = mshc_readw(SDHCI_CLOCK_CONTROL))
		& SDHCI_CLOCK_INT_STABLE)) {
		if (timeout == 0) {
			WARN("%s: Internal clock never stabilized.\n",
				   __func__);
			break;
		}
		timeout--;
		mdelay(1);
	}

	clock |= SDHCI_CLOCK_CARD_EN;
	mshc_writew(clock, SDHCI_CLOCK_CONTROL);
	/* Host provides at least 74 clocks before issuing first command */
	udelay(200);

	mshc_writeb(0xe, SDHCI_TIMEOUT_CONTROL);
}

static int dwc_mshc_set_ios(unsigned int clk, unsigned int width)
{
	unsigned int ctrl = 0;

	dwc_mshc_set_clock(clk);

	ctrl = mshc_readb(SDHCI_HOST_CONTROL);

	if (width == MMC_BUS_WIDTH_8)
		ctrl |= SDHCI_CTRL_8BITBUS;
	else
		ctrl &= (~SDHCI_CTRL_8BITBUS);

	if (width == MMC_BUS_WIDTH_4)
		ctrl |= SDHCI_CTRL_4BITBUS;
	else
		ctrl &= (~SDHCI_CTRL_4BITBUS);

	if (clk > 26000000)
		ctrl |= SDHCI_CTRL_HISPD;
	else
		ctrl &= (~SDHCI_CTRL_HISPD);

	mshc_writeb(ctrl, SDHCI_HOST_CONTROL);

	return 0;
}

static void dwc_mshc_initialize(void)
{
	/* Enable internel clock before software reset */
	mshc_writew(mshc_readw(SDHCI_CLOCK_CONTROL) | SDHCI_CLOCK_INT_EN,
			SDHCI_CLOCK_CONTROL);
	/* Watchout RESET_ALL clear you PHY configuration */
	dwc_mshc_reset(SDHCI_RESET_CMD | SDHCI_RESET_DATA);

	mshc_writeb(mshc_readb(SDHCI_PWR_CONTROL) | SD_BUS_PWR_VDD1_ON,
			SDHCI_PWR_CONTROL);

	dwc_mshc_set_ios(MMC_BOOT_CLK_RATE, MMC_BUS_WIDTH_1);
	mshc_writel(SDHCI_INT_DATA_MASK |
			SDHCI_INT_CMD_MASK |
			SDHCI_INT_CARD_REMOVE |
			SDHCI_INT_CARD_INSERT,
			SDHCI_INT_ENABLE);
	mshc_writel(SDHCI_INT_CARD_INSERT | SDHCI_INT_CARD_REMOVE,
					SDHCI_SIGNAL_ENABLE);

#ifndef USE_32BIT_SDMA
	/* Enable 64bit-SDMA */
	mshc_writew(mshc_readw(SDHCI_HOST_CONTROL2) | SDHCI_CTRL_64BIT_ADDR |
		    SDHCI_CTRL_HOST_VER4_EN, SDHCI_HOST_CONTROL2);
#endif
	mshc_writew(0, SDHCI_BLOCK_COUNT);
}

static int dwc_mshc_prepare(int lba, uintptr_t buf, size_t size)
{
	unsigned short ctrl = 0;

	ctrl = mshc_readb(SDHCI_HOST_CONTROL);
	ctrl &= (~SDHCI_CTRL_DMA_MASK);
	ctrl |= SDHCI_CTRL_SDMA;
	mshc_writeb(ctrl, SDHCI_HOST_CONTROL);

	if (size < BLOCK_SIZE) {
		/* Some command like CMD51, block size is 16Bytes */
		mshc_writew(size | (SDHCI_SDMA_BUF_BDARY_512K << 12),
				SDHCI_BLOCK_SIZE);
		mshc_writew(1, SDHCI_BLOCK_COUNT);
	} else {
		mshc_writew(BLOCK_SIZE | (SDHCI_SDMA_BUF_BDARY_512K << 12),
					SDHCI_BLOCK_SIZE);
		mshc_writew(size / MMC_BLOCK_SIZE, SDHCI_BLOCK_COUNT);
	}
#ifndef USE_32BIT_SDMA
	mshc_writel((unsigned int)(buf & 0xffffffff), SDHCI_ADMA_ADDRESS);
	mshc_writel((unsigned int)(buf >> 32), SDHCI_ADMA_ADDRESS_HI);
#else
	mshc_writel(buf, SDHCI_DMA_ADDRESS);
	start_addr = buf;
#endif
	flush_dcache_range(buf, size);

	return 0;
}

static int dwc_mshc_read(int lba, uintptr_t buf, size_t size)
{
	/*
	 * If your buf and other variables share the same cache line.
	 * Do inv_dcache_range to make sure consistency
	 */
	inv_dcache_range(buf, size);
	return 0;
}

static int dwc_mshc_write(int lba, uintptr_t buf, size_t size)
{
	return 0;
}

void dwc_mshc_init(dwc_mshc_params_t *params)
{
	assert((params != 0) &&
		((params->reg_base & MMC_BLOCK_MASK) == 0) &&
		(params->clk_rate > 0) &&
		((params->bus_width == MMC_BUS_WIDTH_1) ||
		(params->bus_width == MMC_BUS_WIDTH_4) ||
		(params->bus_width == MMC_BUS_WIDTH_8)));

	memcpy(&dwc_mshc_params, params, sizeof(dwc_mshc_params_t));

	if (params->plat_init)
		params->plat_init(params->reg_base);

	mmc_init(&dwc_mshc_ops, params->clk_rate, params->bus_width,
		params->flags, &params->info);
}
