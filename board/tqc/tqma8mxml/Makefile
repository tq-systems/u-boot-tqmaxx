## SPDX-License-Identifier: GPL-2.0-or-later
##
## Copyright (c) 2020-2023 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
## D-82229 Seefeld, Germany.
## Author: Markus Niebel
##

obj-y += tqma8mxml.o
obj-$(CONFIG_TQMA8MXX_BB_MBA8MX) += tqma8mxml-mba8mx.o

ifdef CONFIG_SPL_BUILD
obj-y += spl.o
ifdef CONFIG_IMX8MM
obj-$(CONFIG_TQMA8MMX_RAM_4096MB) += tqma8mxml-4gb-lpddr4_timing.o
obj-$(CONFIG_TQMA8MMX_RAM_2048MB) += tqma8mxml-2gb-lpddr4_timing.o
obj-$(CONFIG_TQMA8MMX_RAM_1024MB) += tqma8mxml-1gb-lpddr4_timing.o
endif
ifdef CONFIG_IMX8MN
obj-$(CONFIG_TQMA8MMX_RAM_1024MB) += tqma8mxnl-1gb-lpddr4_timing.o
endif
obj-$(CONFIG_TQMA8MXX_BB_MBA8MX) += spl-mba8mx.o
endif
