## SPDX-License-Identifier: GPL-2.0-or-later
##
## Copyright (c) 2025 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
## D-82229 Seefeld, Germany.
## Author: Paul Gerber
##

obj-y += tqma8mpxs.o
obj-$(CONFIG_TQMA8MPXS_BB_MB_SMARC_2) += tqma8mpxs-mb-smarc-2.o

ifdef CONFIG_SPL_BUILD
obj-y += spl.o tqma8mpxs-dram.o
obj-$(CONFIG_TQMA8MPXS_BB_MB_SMARC_2) += spl-mb-smarc-2.o

ifdef CONFIG_IMX8M_DRAM_INLINE_ECC
obj-$(CONFIG_TQMA8MPXS_RAM_1024MB) += lpddr4_timing_1gb_ecc.o
obj-$(CONFIG_TQMA8MPXS_RAM_2048MB) += lpddr4_timing_2gb_ecc.o
obj-$(CONFIG_TQMA8MPXS_RAM_4096MB) += lpddr4_timing_4gb_ecc.o
obj-$(CONFIG_TQMA8MPXS_RAM_8192MB) += lpddr4_timing_8gb_ecc.o
else
obj-$(CONFIG_TQMA8MPXS_RAM_1024MB) += lpddr4_timing_1gb_no_ecc.o
obj-$(CONFIG_TQMA8MPXS_RAM_2048MB) += lpddr4_timing_2gb_no_ecc.o
obj-$(CONFIG_TQMA8MPXS_RAM_4096MB) += lpddr4_timing_4gb_no_ecc.o
obj-$(CONFIG_TQMA8MPXS_RAM_8192MB) += lpddr4_timing_8gb_no_ecc.o
endif

endif
