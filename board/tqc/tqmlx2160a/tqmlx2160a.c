#include <common.h>
#include <dm.h>
#include <dm/platform_data/serial_pl01x.h>
#include <fsl_mdio.h>
#include <fm_eth.h>
#include <fsl-mc/ldpaa_wriop.h>
#include <fsl-mc/fsl_mc.h>
#include <i2c.h>
#include <netdev.h>

#include "../common/tqc_bb.h"

DECLARE_GLOBAL_DATA_PTR;

static struct pl01x_serial_platdata serial0 = {
#if CONFIG_CONS_INDEX == 0
	.base = CONFIG_SYS_SERIAL0,
#elif CONFIG_CONS_INDEX == 1
	.base = CONFIG_SYS_SERIAL1,
#else
#error "Unsupported console index value."
#endif
	.type = TYPE_PL011,
};

U_BOOT_DEVICE(nxp_serial0) = {
	.name = "serial_pl01x",
	.platdata = &serial0,
};

static struct pl01x_serial_platdata serial1 = {
	.base = CONFIG_SYS_SERIAL1,
	.type = TYPE_PL011,
};

U_BOOT_DEVICE(nxp_serial1) = {
	.name = "serial_pl01x",
	.platdata = &serial1,
};

static void uart_get_clock(void)
{
	serial0.clock = get_serial_clock();
	serial1.clock = get_serial_clock();
}

unsigned long get_board_sys_clk(void)
{
	return 100000000;
}

unsigned long get_board_ddr_clk(void)
{
	return 100000000;
}

int board_early_init_f(void)
{
	uart_get_clock();
	fsl_lsch3_early_init_f();
	return 0;
}

int rtc_init(void)
{
	u8 val;
	struct udevice *dev;

	if (i2c_get_chip_for_busnum(CONFIG_SYS_RTC_BUS_NUM, CONFIG_SYS_I2C_RTC_ADDR, 1, &dev))
		return -ENODEV;

	/* Set Bit 0 of Register 0 of RTC to adjust to 12.5 pF */
	val = dm_i2c_reg_read(dev, 0x00);

	if (!(val & 0x01))
		dm_i2c_reg_write(dev, 0x00, val | 0x01);

	return 0;
}

int board_init(void)
{
	int ret;

	ret = rtc_init();

	return ret;
}

int board_eth_init(bd_t *bis)
{
#if defined(CONFIG_FSL_MC_ENET)
	struct memac_mdio_info mdio_info;
	struct memac_mdio_controller *reg;

	reg = (struct memac_mdio_controller *)CONFIG_SYS_FSL_WRIOP1_MDIO1;
	mdio_info.regs = reg;
	mdio_info.name = DEFAULT_WRIOP_MDIO1_NAME;

	/* Register the EMI 1 */
	fm_memac_mdio_init(bis, &mdio_info);

	reg = (struct memac_mdio_controller *)CONFIG_SYS_FSL_WRIOP1_MDIO2;
	mdio_info.regs = reg;
	mdio_info.name = DEFAULT_WRIOP_MDIO2_NAME;

	/* Register the EMI 2 */
	fm_memac_mdio_init(bis, &mdio_info);

	tqc_bb_board_eth_init();

	cpu_eth_init(bis);
#endif /* CONFIG_FSL_MC_ENET */

	return pci_eth_init(bis);
}

#if defined(CONFIG_RESET_PHY_R)
void reset_phy(void)
{
#if defined(CONFIG_FSL_MC_ENET)
	mc_env_boot();
#endif
}
#endif /* CONFIG_RESET_PHY_R */

#if defined(CONFIG_ARCH_MISC_INIT)
int arch_misc_init(void)
{
	return 0;
}
#endif

#ifdef CONFIG_FSL_MC_ENET
void fdt_fixup_board_enet(void *fdt)
{
}
#endif
