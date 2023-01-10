# SPDX-License-Identifier: GPL-2.0-or-later
#
# Copyright (c) 2016-2023 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
# D-82229 Seefeld, Germany.
# Author: Markus Niebel
#

MINIMAL=

ifdef CONFIG_SPL_BUILD
ifdef CONFIG_SPL_INIT_MINIMAL
MINIMAL=y
endif
endif

ifdef MINIMAL
# necessary to create built-in.o
obj- := __dummy__.o
else
obj-$(CONFIG_DM_GPIO) += tqc_board_gpio.o
obj-$(CONFIG_TQC_EEPROM) += tqc_eeprom.o
obj-$(CONFIG_TQC_EMMC) += tqc_emmc.o
obj-$(CONFIG_TQC_SDMMC) += tqc_sdmmc.o
obj-$(CONFIG_TQC_SCU) += tqc_scu.o
obj-$(CONFIG_TQC_RTC) += tqc_rtc.o
obj-$(CONFIG_TQC_ENVLOC) += tqc_envloc.o
obj-$(CONFIG_TQC_VARD) += tq_som_features.o
obj-$(CONFIG_TQC_SPI_NOR) += tqc_spi_nor.o
endif
