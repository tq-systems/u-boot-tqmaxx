## SPDX-License-Identifier: GPL-2.0-or-later
##
## Copyright 2019-2023 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
## D-82229 Seefeld, Germany.
## Author: Markus Niebel
##

obj-y += tqma8mx.o
obj-$(CONFIG_TQMA8MX_BB_MBA8MX) += tqma8mx-mba8mx.o

ifdef CONFIG_SPL_BUILD
obj-y += spl.o
obj-$(CONFIG_TQMA8MX_RAM_4G) += tqma8mx-4gb-lpddr4_timing.o
obj-$(CONFIG_TQMA8MX_RAM_2G) += tqma8mx-2gb-lpddr4_timing.o
obj-$(CONFIG_TQMA8MX_RAM_1G) += tqma8mx-1gb-lpddr4_timing.o
obj-$(CONFIG_TQMA8MX_BB_MBA8MX) += spl-mba8mx.o
endif
