# SPDX-License-Identifier: GPL-2.0-or-later
#
# Copyright (c) 2022-2023 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
# D-82229 Seefeld, Germany.
# Author: Markus Niebel
#

obj-y += tqma93xx.o

obj-$(CONFIG_TQMA93XX_BB_MBA91XXCA) += tqma93xx-mba91xx.o
obj-$(CONFIG_TQMA93XX_BB_MBA93XXCA) += tqma93xx-mba93xx.o
obj-$(CONFIG_TQMA93XX_BB_MBA93XXLA) += tqma93xx-mba93xx.o
obj-$(CONFIG_TQMA93XX_BB_MBA93XXLA_MINI) += tqma93xx-mba93xx.o

ifdef CONFIG_SPL_BUILD
obj-y += spl.o
obj-$(CONFIG_IMX91) += lpddr4_tqma91xxca_timing_1gb_noecc.o
obj-$(CONFIG_IMX91) += lpddr4_tqma91xxla_timing_1gb_noecc.o
obj-$(CONFIG_IMX91) += tqma91xx-dram.o
obj-$(CONFIG_IMX93) += lpddr4x_tqma93xxca_timing_1gb_noecc.o
obj-$(CONFIG_IMX93) += lpddr4x_tqma93xxla_timing_1gb_noecc.o
obj-$(CONFIG_IMX93) += lpddr4x_tqma93xxla_timing_1gb5_noecc.o
obj-$(CONFIG_IMX93) += lpddr4x_tqma93xxca_timing_2gb_noecc.o
obj-$(CONFIG_IMX93) += lpddr4x_tqma93xxla_timing_2gb_noecc.o
obj-$(CONFIG_IMX93) += tqma93xx-dram.o
endif
