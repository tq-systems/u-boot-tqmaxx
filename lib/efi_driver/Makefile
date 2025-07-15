# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2017 Heinrich Schuchardt

# This file only gets included with CONFIG_EFI_LOADER set, so all
# object inclusion implicitly depends on it

obj-y += efi_uclass.o
ifeq ($(CONFIG_PARTITIONS),y)
obj-y += efi_block_device.o
endif
obj-$(CONFIG_SYSRESET_SBI) += efi_reset_riscv.o

obj-$(CONFIG_EFI_GBL_IMAGE_LOADING) += efi_gbl_image_loading_protocol.o
obj-$(CONFIG_EFI_GBL_AVB) += efi_gbl_avb_protocol.o
obj-$(CONFIG_EFI_GBL_OS_CONFIGURATION) += efi_gbl_os_configuration_protocol.o
