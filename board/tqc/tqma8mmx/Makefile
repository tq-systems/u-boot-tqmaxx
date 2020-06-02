#
# Copyright 2020 TQ Systems GmbH
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-y += tqma8mmx.o
obj-$(CONFIG_TQMA8MMX_BB_MBA8MX) += tqma8mmx-mba8mx.o

ifdef CONFIG_SPL_BUILD
obj-y += spl.o
obj-y += tqma8mmx-512mb-lpddr4_timing.o
obj-$(CONFIG_TQMA8MMX_BB_MBA8MX) += spl-mba8mx.o
endif
