## SPDX-License-Identifier: GPL-2.0-or-later
##
## Copyright (c) 2021-2023 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
## D-82229 Seefeld, Germany.
## Author: Markus Niebel
##

obj-y += tqma8mpxl.o tqma8mpxl-usbg.o
obj-$(CONFIG_TQMA8MPXL_BB_MBA8MPXL) += tqma8mpxl-mba8mpxl.o

ifdef CONFIG_SPL_BUILD
obj-y += spl.o
obj-$(CONFIG_TQMA8MPXL_BB_MBA8MPXL) += spl-mba8mpxl.o
obj-$(CONFIG_TQMA8MPXL_RAM_1024MB) += lpddr4_timing_1gb_no_ecc.o
obj-$(CONFIG_TQMA8MPXL_RAM_2048MB) += lpddr4_timing_2gb_no_ecc.o
obj-$(CONFIG_TQMA8MPXL_RAM_4096MB) += lpddr4_timing_4gb_no_ecc.o
obj-$(CONFIG_TQMA8MPXL_RAM_8192MB) += lpddr4_timing_8gb_no_ecc.o
endif
