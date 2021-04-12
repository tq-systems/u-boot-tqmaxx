## SPDX-License-Identifier: GPL-2.0-or-later
##
## Copyright 2021 TQ-Systems GmbH
##

obj-y += tqma8mpxl.o
obj-$(CONFIG_TQMA8MPXL_BB_MBA8MPXL) += tqma8mpxl-mba8mpxl.o

ifdef CONFIG_SPL_BUILD
obj-y += spl.o
obj-$(CONFIG_TQMA8MPXL_BB_MBA8MPXL) += spl-mba8mpxl.o
obj-$(CONFIG_TQMA8MPXL_RAM_2048MB) += lpddr4_timing_2gb_no_ecc.o
endif
