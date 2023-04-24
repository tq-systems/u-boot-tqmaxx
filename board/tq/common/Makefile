# SPDX-License-Identifier: GPL-2.0-or-later
#
# Copyright (c) 2016-2023 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
# D-82229 Seefeld, Germany.
# Author: Markus Niebel, Matthias Schiffer

obj-$(CONFIG_$(SPL_)TQ_EEPROM) += tq_eeprom.o

ifndef CONFIG_SPL_BUILD
obj-$(CONFIG_TQ_EMMC) += tq_emmc.o
obj-$(CONFIG_TQ_SDMMC) += tq_sdmmc.o
obj-$(CONFIG_TQ_SPI_NOR) += tq_spi_nor.o
endif
