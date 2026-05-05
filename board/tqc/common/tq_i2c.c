// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2026 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Max Merchel
 */

#include <common.h>
#include <i2c.h>
#include <log.h>
#include <linux/libfdt.h>
#include <linux/types.h>

#include "tq_i2c.h"

#define I2C_VENDOR_REG_OFFSET		0xfe
#define I2C_DEVICE_REG_OFFSET		0xff

#define I2C_VENDOR_NXP			0xa1
#define I2C_VENDOR_TI			0x55

/* NXP SA56004 */
#define I2C_TEMPSENSOR_NXP_SA56004	0x0

/* TI TMP411C/TMP411DC */
#define I2C_TEMPSENSOR_TI_TMP411C	0x10
/* TI TMP411A/TMP411DA/TMP411E/TMP411DE */
#define I2C_TEMPSENSOR_TI_TMP411A	0x12
/* TI TMP411B/TMP411DB */
#define I2C_TEMPSENSOR_TI_TMP411B	0x13

struct tq_i2c_device {
	u8 vendor_id;
	u8 rev_id;
};

/*
 * tq_i2c_read_vendor_device - read vendor and device register
 */
static int tq_i2c_read_vendor_device(int bus, int addr, uint offset_len,
				     struct tq_i2c_device *device)
{
	struct udevice *dev;
	int ret;

	ret = i2c_get_chip_for_busnum(bus, addr,
				      offset_len, &dev);
	if (ret) {
		printf("%s: Cannot find I2C chip for bus %d\n",
		       __func__, bus);
		return ret;
	}

	ret = dm_i2c_read(dev, I2C_VENDOR_REG_OFFSET, &device->vendor_id,
			  sizeof(device->vendor_id));

	if (ret) {
		debug("ERROR: failed to read tempsensor vendor id: %d\n", ret);
		return ret;
	}

	ret = dm_i2c_read(dev, I2C_DEVICE_REG_OFFSET, &device->rev_id,
			  sizeof(device->rev_id));
	if (ret) {
		debug("ERROR: failed to read Device ID or revision: %d\n", ret);
		return ret;
	}

	return 0;
}

static int tq_i2c_select_temp_compatible(void *blob, int path_offset,
					 const struct tq_i2c_device *device)
{
	int ret;

	switch (device->vendor_id) {
	case I2C_VENDOR_NXP:
		puts("Tempsensor: NXP ");
		switch (device->rev_id) {
		case I2C_TEMPSENSOR_NXP_SA56004:
			puts("SA56004\n");
			ret = fdt_setprop_string(blob,
						 path_offset,
						 "compatible",
						 "nxp,sa56004");
			if (ret)
				return ret;

			break;
		default:
			puts("<unknown>\n");
			return -EINVAL;
		}
		break;
	case I2C_VENDOR_TI:
		puts("Tempsensor: TI ");
		switch (device->rev_id) {
		case I2C_TEMPSENSOR_TI_TMP411A:
		case I2C_TEMPSENSOR_TI_TMP411B:
		case I2C_TEMPSENSOR_TI_TMP411C:
			puts("TMP411\n");
			ret = fdt_setprop_string(blob,
						 path_offset,
						 "compatible",
						 "ti,tmp411");
			if (ret)
				return ret;

			break;
		default:
			puts("<unknown>\n");
			return -EINVAL;
		}
		break;
	default:
		printf("WARNING: unknown temperature sensor vendor id 0x%02x\n",
		       device->vendor_id);
		return -EINVAL;
	}

	return 0;
}

int tq_i2c_detect_fixup_temp_compatible(void *blob, int bus, int addr,
					uint offset_len,
					const char *fdt_path)
{
	struct tq_i2c_device device;
	int path_offset;
	int ret;

	path_offset = fdt_path_offset(blob, fdt_path);
	if (path_offset < 0) {
		printf("ERROR: sensor node not found in device tree (error %d)\n",
		       path_offset);
		return -EINVAL;
	}

	ret = tq_i2c_read_vendor_device(bus, addr,
					offset_len, &device);
	if (ret)
		return -EINVAL;

	ret = tq_i2c_select_temp_compatible(blob, path_offset,
					    &device);
	if (ret)
		return ret;

	return 0;
}
