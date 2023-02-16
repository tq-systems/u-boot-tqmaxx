# SPDX-License-Identifier: GPL-2.0-or-later
#
# Copyright (c) 2022-2023 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
# D-82229 Seefeld, Germany.
# Author: Markus Niebel
#

obj-y += tqma93xx.o
obj-$(CONFIG_TQMA93XX_BB_MBA93XXCA) += tqma93xxla-mba93xxca.o

ifdef CONFIG_SPL_BUILD
obj-y += spl.o
obj-y += lpddr4x_tqma93xxca_timing.o
obj-y += lpddr4x_tqma93xxla_timing.o
endif
