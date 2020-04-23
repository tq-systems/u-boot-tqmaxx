/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2020 - Texas Instruments Incorporated - http://www.ti.com/
 *	Dave Gerlach <d-gerlach@ti.com>
 */

#ifndef __SOC_H
#define __SOC_H

#define SOC_MAX_STR_SIZE	128

struct soc_device_attribute {
	const char *machine;
	const char *family;
	const char *revision;
	const void *data;
};

struct soc_ops {
	/**
	 * get_machine() - Get machine name of an SOC
	 *
	 * @dev:	Device to check (UCLASS_SOC)
	 * @buf:	Buffer to place string
	 * @size:	Size of string space
	 * @return 0 if OK, -ENOSPC if buffer is too small, other -ve on error
	 */
	int (*get_machine)(struct udevice *dev, char *buf, int size);

	/**
	 * get_revision() - Get revision name of a SOC
	 *
	 * @dev:	Device to check (UCLASS_SOC)
	 * @buf:	Buffer to place string
	 * @size:	Size of string space
	 * @return 0 if OK, -ENOSPC if buffer is too small, other -ve on error
	 */
	int (*get_revision)(struct udevice *dev, char *buf, int size);

	/**
	 * get_family() - Get family name of an SOC
	 *
	 * @dev:	Device to check (UCLASS_SOC)
	 * @buf:	Buffer to place string
	 * @size:	Size of string space
	 * @return 0 if OK, -ENOSPC if buffer is too small, other -ve on error
	 */
	int (*get_family)(struct udevice *dev, char *buf, int size);
};

#define soc_get_ops(dev)        ((struct soc_ops *)(dev)->driver->ops)

/**
 * soc_get() - Return the soc device for the soc in use.
 * @devp: Pointer to structure to receive the soc device.
 *
 * Since there can only be at most one SOC instance, the API can supply a
 * function that returns the unique device.
 *
 * Return: 0 if OK, -ve on error.
 */
int soc_get(struct udevice **devp);

/**
 * soc_get_machine() - Get machine name of an SOC
 * @dev:	Device to check (UCLASS_SOC)
 * @buf:	Buffer to place string
 * @size:	Size of string space
 *
 * Return: 0 if OK, -ENOSPC if buffer is too small, other -ve on error
 */
int soc_get_machine(struct udevice *dev, char *buf, int size);

/**
 * soc_get_revision() - Get revision name of an SOC
 * @dev:	Device to check (UCLASS_SOC)
 * @buf:	Buffer to place string
 * @size:	Size of string space
 *
 * Return: 0 if OK, -ENOSPC if buffer is too small, other -ve on error
 */
int soc_get_revision(struct udevice *dev, char *buf, int size);

/**
 * soc_get_family() - Get family name of an SOC
 * @dev:	Device to check (UCLASS_SOC)
 * @buf:	Buffer to place string
 * @size:	Size of string space
 *
 * Return: 0 if OK, -ENOSPC if buffer is too small, other -ve on error
 */
int soc_get_family(struct udevice *dev, char *buf, int size);

/**
 * soc_device_match() - Return match from an array of soc_device_attribute
 * @matches:	Array with any combination of family, revision or machine set
 *
 * Return: Pointer to struct from matches array with set attributes matching
 *	   those provided by the soc device.
 */
const struct soc_device_attribute *
soc_device_match(const struct soc_device_attribute *matches);

#endif
