#
# Copyright (C) 2016 - 2020, Markus Niebel <Markus.Niebel@tq-group.com>
#
# SPDX-License-Identifier:	GPL-2.0+
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
endif
