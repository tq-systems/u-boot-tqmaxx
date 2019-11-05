#
# Copyright 2016 Freescale Semiconductor
# Copyright 2018 TQ Systems GmbH
#
# SPDX-License-Identifier:      GPL-2.0+
#

obj-y += tqma8mx.o
obj-$(CONFIG_MBA8MX) += tqma8mx-mba8mx.o

ifdef CONFIG_SPL_BUILD
obj-y += spl.o
obj-y += ddr/ddr_init.o ddr/ddrphy_train.o ddr/helper.o
endif
