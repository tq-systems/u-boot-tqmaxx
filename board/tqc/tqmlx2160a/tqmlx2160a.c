#include <common.h>
#include <asm/io.h>
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
	.base = CONFIG_SYS_SERIAL0,
	.type = TYPE_PL011,
};

U_BOOT_DEVICE(nxp_serial0) = {
	.name = "serial_pl01x",
	.platdata = &serial0,
};

static void uart_get_clock(void)
{
	serial0.clock = get_serial_clock();
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

#if defined(CONFIG_VID)
int init_func_vid(void)
{
	struct ccsr_gur *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	u32 fusesr;
	u8 gur_vid;
	u8 sysc_vid;
	struct udevice *dev;

	/* get the voltage ID from fuse status register */
	fusesr = in_le32(&gur->dcfg_fusesr);
	gur_vid = (fusesr >> FSL_CHASSIS3_DCFG_FUSESR_ALTVID_SHIFT) &
		FSL_CHASSIS3_DCFG_FUSESR_ALTVID_MASK;
	if ((gur_vid == 0) || (gur_vid == FSL_CHASSIS3_DCFG_FUSESR_ALTVID_MASK)) {
		gur_vid = (fusesr >> FSL_CHASSIS3_DCFG_FUSESR_VID_SHIFT) &
			FSL_CHASSIS3_DCFG_FUSESR_VID_MASK;
	}

	if (i2c_get_chip_for_busnum(SYSCTRL_I2C_BUS_NUM, SYSCTRL_I2C_ADDR, 1, &dev))
		return -ENODEV;

	sysc_vid = dm_i2c_reg_read(dev, 0x30) & 0x1F;

	printf("VID device config: %x, VID systemcontroller config: %x\n", gur_vid, sysc_vid);
	if (sysc_vid != gur_vid) {
		dm_i2c_reg_write(dev, 0x30, gur_vid);
		printf("VDD core voltage mismatch. Setting VID voltage\n");
	}
	return 0;
}
#endif

int checkboard(void)
{
	printf("TQMLX2160A ");
	checkboard_tqmlx2160a_bb();
	puts("\n");

	return 0;
}

int board_init(void)
{
	int ret;

	ret = rtc_init();

#if defined(CONFIG_MBLX2160A)
	ret = mblx2160a_gpios_init();
	ret = mblx2160a_board_init();
#endif

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
	tqc_bb_board_eth_late_init();
}
#endif /* CONFIG_RESET_PHY_R */

#if defined(CONFIG_ARCH_MISC_INIT)
int arch_misc_init(void)
{
	return 0;
}
#endif

int ft_board_setup(void *blob, bd_t *bd)
{
	int i;
	bool mc_memory_bank = false;

	u64 *base;
	u64 *size;
	u64 mc_memory_base = 0;
	u64 mc_memory_size = 0;
	u16 total_memory_banks;

	ft_cpu_setup(blob, bd);
	fdt_fixup_mc_ddr(&mc_memory_base, &mc_memory_size);

	if (mc_memory_base != 0)
		mc_memory_bank = true;

	total_memory_banks = CONFIG_NR_DRAM_BANKS + mc_memory_bank;

	base = calloc(total_memory_banks, sizeof(u64));
	size = calloc(total_memory_banks, sizeof(u64));

	/* fixup DT for the three GPP DDR banks */
	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		base[i] = gd->bd->bi_dram[i].start;
		size[i] = gd->bd->bi_dram[i].size;
	}

#ifdef CONFIG_RESV_RAM
	/* reduce size if reserved memory is within this bank */
	if (gd->arch.resv_ram >= base[0] &&
	    gd->arch.resv_ram < base[0] + size[0])
		size[0] = gd->arch.resv_ram - base[0];
	else if (gd->arch.resv_ram >= base[1] &&
		 gd->arch.resv_ram < base[1] + size[1])
		size[1] = gd->arch.resv_ram - base[1];
	else if (gd->arch.resv_ram >= base[2] &&
		 gd->arch.resv_ram < base[2] + size[2])
		size[2] = gd->arch.resv_ram - base[2];
#endif

	if (mc_memory_base != 0) {
		for (i = 0; i <= total_memory_banks; i++) {
			if (base[i] == 0 && size[i] == 0) {
				base[i] = mc_memory_base;
				size[i] = mc_memory_size;
				break;
			}
		}
	}

	fdt_fixup_memory_banks(blob, base, size, total_memory_banks);

#ifdef CONFIG_USB
	fsl_fdt_fixup_dr_usb(blob, bd);
#endif

#ifdef CONFIG_FSL_MC_ENET
	fdt_fsl_mc_fixup_iommu_map_entry(blob);
	fdt_fixup_board_enet(blob);
#endif

	return 0;
}
#ifdef CONFIG_FSL_MC_ENET
extern int fdt_fixup_board_phy(void *fdt);

void fdt_fixup_board_enet(void *fdt)
{
	int offset;

	offset = fdt_path_offset(fdt, "/soc/fsl-mc");

	if (offset < 0)
		offset = fdt_path_offset(fdt, "/fsl-mc");

	if (offset < 0) {
		printf("%s: fsl-mc node not found in device tree (error %d)\n",
		       __func__, offset);
		return;
	}

	if (get_mc_boot_status() == 0 &&
	    (is_lazy_dpl_addr_valid() || get_dpl_apply_status() == 0)) {
		fdt_status_okay(fdt, offset);
		fdt_fixup_board_phy(fdt);
	} else {
		fdt_status_fail(fdt, offset);
	}
}

void board_quiesce_devices(void)
{
	fsl_mc_ldpaa_exit(gd->bd);
}
#endif

#ifdef CONFIG_OF_BOARD_FIXUP
int board_fix_fdt(void *fdt)
{
	char *reg_name, *old_str, *new_str;
	const char *reg_names;
	int names_len, old_str_len, new_str_len, remaining_str_len;
	struct str_map {
		char *old_str;
		char *new_str;
	} reg_names_map[] = {
		{ "ccsr", "dbi" },
		{ "pf_ctrl", "ctrl" }
	};
	int off = -1, i;

	if (IS_SVR_REV(get_svr(), 1, 0))
		return 0;

	off = fdt_node_offset_by_compatible(fdt, -1, "fsl,lx2160a-pcie");
	while (off != -FDT_ERR_NOTFOUND) {
		fdt_setprop(fdt, off, "compatible", "fsl,ls-pcie",
			    strlen("fsl,ls-pcie") + 1);

		reg_names = fdt_getprop(fdt, off, "reg-names", &names_len);
		if (!reg_names)
			continue;

		reg_name = (char *)reg_names;
		remaining_str_len = names_len - (reg_name - reg_names);
		i = 0;
		while ((i < ARRAY_SIZE(reg_names_map)) && remaining_str_len) {
			old_str = reg_names_map[i].old_str;
			new_str = reg_names_map[i].new_str;
			old_str_len = strlen(old_str);
			new_str_len = strlen(new_str);
			if (memcmp(reg_name, old_str, old_str_len) == 0) {
				/* first only leave required bytes for new_str
				 * and copy rest of the string after it
				 */
				memcpy(reg_name + new_str_len,
				       reg_name + old_str_len,
				       remaining_str_len - old_str_len);
				/* Now copy new_str */
				memcpy(reg_name, new_str, new_str_len);
				names_len -= old_str_len;
				names_len += new_str_len;
				i++;
			}

			reg_name = memchr(reg_name, '\0', remaining_str_len);
			if (!reg_name)
				break;

			reg_name += 1;

			remaining_str_len = names_len - (reg_name - reg_names);
		}

		fdt_setprop(fdt, off, "reg-names", reg_names, names_len);
		off = fdt_node_offset_by_compatible(fdt, off,
						    "fsl,lx2160a-pcie");
	}

	return 0;
}
#endif
