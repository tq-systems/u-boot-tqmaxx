/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2013, 2014, 2016 - 2022 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 */

#ifndef __TQC_BB__
#define __TQC_BB__

struct mmc;
struct bd_info;

int tq_bb_board_mmc_getwp(struct mmc *mmc);
int tq_bb_board_mmc_getcd(struct mmc *mmc);
int tq_bb_board_mmc_init(struct bd_info *bis);

void tq_bb_board_init_f(ulong dummy);
int tq_bb_board_early_init_f(void);
int tq_bb_board_init(void);
int tq_bb_board_late_init(void);
int tq_bb_checkboard(void);
void tq_bb_board_quiesce_devices(void);

const char *tq_bb_get_boardname(void);

#if defined(CONFIG_SPL_BUILD)
void tq_bb_spl_board_init(void);
#endif

/*
 * Device Tree Support
 */
#if defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT)
struct bd_info;

int tq_bb_ft_board_setup(void *blob, struct bd_info *bis);
#endif /* defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT) */

#if defined(CONFIG_TQ_RTC) && defined(CONFIG_DM_I2C)

#define TQ_PCF85063_CLKOUT_OFF 0x07

int tq_pcf85063_adjust_capacity(int bus, int address, int quartz_load);
int tq_pcf85063_set_clkout(int bus, int address, uint8_t clkout);
int tq_pcf85063_set_offset(int bus, int address, bool mode, int offset);
#endif /* CONFIG_TQC_RTC */

#endif /* __TQC_BB_H */
