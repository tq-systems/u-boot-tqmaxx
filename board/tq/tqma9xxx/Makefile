# SPDX-License-Identifier: GPL-2.0-or-later
#
# Copyright (c) 2022 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
# D-82229 Seefeld, Germany.
# Author: Markus Niebel
#

obj-y += tqma9xxxla.o
obj-$(CONFIG_TQMa9XXX_BB_MBA9XXXCA) += tqma9xxxla-mba9xxxca.o

ifdef CONFIG_SPL_BUILD
obj-y += spl.o
obj-$(CONFIG_TQMA93XXLA) += lpddr4x_tqma9xxxla_timing.o
endif
