#
# Copyright 2020 TQ Systems GmbH
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-y += tqma8mmx.o
obj-$(CONFIG_TQMA8MXX_BB_MBA8MX) += tqma8mmx-mba8mx.o

ifdef CONFIG_SPL_BUILD
obj-y += spl.o
ifdef CONFIG_IMX8MM
obj-$(CONFIG_TQMA8MMX_RAM_1024MB) += tqma8mmx-1gb-lpddr4_timing.o
endif
ifdef CONFIG_IMX8MN
obj-$(CONFIG_TQMA8MMX_RAM_1024MB) += tqma8mnx-1gb-lpddr4_timing.o
endif
obj-$(CONFIG_TQMA8MXX_BB_MBA8MX) += spl-mba8mx.o
endif
