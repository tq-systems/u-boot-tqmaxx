# SPDX-License-Identifier: GPL-2.0-or-later
#
# Copyright (c) 2016 - 2022 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
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
obj-$(CONFIG_TQ_BB) += tq_bb.o
obj-$(CONFIG_TQ_BOARDGPIO) += tq_board_gpio.o
obj-$(CONFIG_TQ_EEPROM) += tq_eeprom.o
obj-$(CONFIG_TQ_EMMC) += tq_emmc.o
obj-$(CONFIG_TQ_ENVLOC) += tq_envloc.o
obj-$(CONFIG_TQ_MBLS10XXA) += tq_mbls10xxa.o
obj-$(CONFIG_TQ_RTC) += tq_rtc.o
obj-$(CONFIG_TQ_SCU) += tq_scu.o
obj-$(CONFIG_TQ_SDMMC) += tq_sdmmc.o
obj-$(CONFIG_TQ_SPI_NOR) += tq_spi_nor.o
obj-$(CONFIG_TQ_TQMLS10XXA_COMMON) += tq_tqmls10xxa_common.o
obj-$(CONFIG_TQ_VARD) += tq_som_features.o

ifndef CONFIG_SPL_BUILD
obj-$(CONFIG_USB_TCPC) += tcpc.o
endif

endif
