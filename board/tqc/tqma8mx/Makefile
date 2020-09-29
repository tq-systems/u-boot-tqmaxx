#
# Copyright 2018 - 2020 TQ Systems GmbH
#
# SPDX-License-Identifier:      GPL-2.0+
#

obj-y += tqma8mx.o
obj-$(CONFIG_TQMA8MX_BB_MBA8MX) += tqma8mx-mba8mx.o

ifdef CONFIG_SPL_BUILD
obj-y += spl.o
obj-y += tqma8mx-4gb-lpddr4_timing.o
obj-y += tqma8mx-2gb-lpddr4_timing.o
obj-y += tqma8mx-1gb-lpddr4_timing.o
obj-$(CONFIG_TQMA8MX_BB_MBA8MX) += spl-mba8mx.o
endif
