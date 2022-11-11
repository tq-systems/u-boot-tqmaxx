// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2014 - 2022 TQ-Systems GmbH <license@tq-group.com>, D-82229 Seefeld, Germany.
 * Author: Markus Niebel <Markus.Niebel@tq-group.com>
 *
 */

#include <common.h>
#include <console.h>
#include <u-boot/crc.h>
#include <i2c.h>
#include <malloc.h>
#include <linux/ctype.h>

#include "tqc_eeprom.h"

/*
 * read_eeprom - read the given EEPROM into memory
 */
int tqc_read_eeprom_buf(unsigned int bus, unsigned int i2c_addr,
			unsigned int alen, unsigned int addr,
			size_t bsize, uchar *buf)
{
	int ret;
#ifdef CONFIG_DM_I2C
	struct udevice *dev;
#else
	unsigned int oldbus;
#endif

	if (!buf)
		return -1;

#ifdef CONFIG_DM_I2C
	ret = i2c_get_chip_for_busnum(bus, i2c_addr, alen, &dev);
	if (ret) {
		debug("%s: Cannot find I2C chip for bus %d\n", __func__, bus);
		return ret;
	}

	ret = dm_i2c_read(dev, addr, buf, bsize);
#else
	oldbus = i2c_get_bus_num();
	i2c_set_bus_num(bus);
	ret = i2c_read(i2c_addr, addr, alen, buf, bsize);
	i2c_set_bus_num(oldbus);
#endif
	return ret;
}

#ifdef CONFIG_TQC_VARD
static struct tqc_eeprom_data eeprom;
static int tqc_vard_has_been_read = 0;
static bool tqc_vard_valid = false;

static int tqc_vard_cmp_crc(struct tqc_eeprom_data *data, uint16_t *cp)
{
	void *crc_offs;
	int crc_len;
	uint16_t crc;

	/* calculate crc over all vard data except checksum */
	crc_offs = (void *)(&data->crc + 1);
	crc_len = 0x20 - sizeof(data->crc);
	crc = crc16_ccitt(0, crc_offs, crc_len);

	if (cp)
		*cp = crc;

	if (crc != data->crc)
		return -1;

	return 0;
}

static int tqc_vard_is_crc_valid(void)
{
	uint16_t calculated;

	if (tqc_vard_valid)
		return 1;

	if (tqc_vard_cmp_crc(&eeprom, &calculated)) {
		printf("TQC_VARD: CRC mismatch (%04x != %04x)\n",
			eeprom.crc, calculated);
		return 0;
	}

	tqc_vard_valid = true;

	return 1;
}

static int tqc_vard_read_eeprom(void)
{
	int ret = 0;

	if (tqc_vard_has_been_read)
		return 0;

	ret = tqc_read_eeprom_buf(TQC_VARD_BUS, TQC_VARD_ADDR, 1, 0,
				  sizeof(eeprom), (void *)&eeprom);
	if (ret) {
		printf("Error reading VARD eeprom: %d\n", ret);
		return ret;
	}

	tqc_vard_has_been_read = 1;

	return 0;
}

static inline bool tqc_validate_eeprom(void)
{
	if (!tqc_vard_read_eeprom() && tqc_vard_is_crc_valid())
		return true;
	else
		return false;
};

int tqc_has_hwrev(u8 rev)
{
	if (tqc_validate_eeprom())
		return (eeprom.hwrev & be32_to_cpu(rev));
	else
		return 0;
};

int tqc_has_memsize(u8 size)
{
	if (tqc_validate_eeprom())
		return (eeprom.memsize & be32_to_cpu(size));
	else
		return 0;
};

int tqc_has_memtype(u8 type)
{
	if (tqc_validate_eeprom())
		return (eeprom.memtype & be32_to_cpu(type));
	else
		return 0;
};

int tqc_has_feature1(u32 mask)
{
	if (tqc_validate_eeprom())
		return ((eeprom.features1 & be32_to_cpu(mask)) == 0);
	else
		return 0;
};

int tqc_has_feature2(u32 mask)
{
	if (tqc_validate_eeprom())
		return ((eeprom.features2 & be32_to_cpu(mask)) == 0);
	else
		return 0;
};
#endif

#if !defined(CONFIG_SPL_BUILD)

void tqc_set_ethaddr(struct tqc_eeprom_data const *eedat,
		     const char *env_var, size_t additional)
{
	char safe_string[0x41];

	if (!tqc_parse_eeprom_mac_additional(eedat, safe_string,
					     ARRAY_SIZE(safe_string),
					     additional)) {
		char *ethaddr = env_get(env_var);

		if (ethaddr &&
		    strncmp(safe_string, ethaddr, 18)) {
			printf("\n");
			printf("Warning: MAC addresses for '%s' don't match:\n",
			       env_var);
			printf("Address in EEPROM is      %s\n",
			       safe_string);
			printf("Address in environment is %s\n",
			       ethaddr);
		} else {
			env_set(env_var, safe_string);
		}
	}
}

int tqc_parse_eeprom_mac_additional(struct tqc_eeprom_data const *eeprom,
				    char *buf, size_t len, size_t additional)
{
	u8 const *p;
	int ret = -1;
	u32 addr;

	if (!buf || !eeprom)
		return -1;

	p = eeprom->mac;

	addr = p[3] << 16 | p[4] << 8 | p[5];
	addr += additional;
	addr = addr & 0x00ffffff;

	ret = snprintf(buf, len,  "%02x:%02x:%02x:%02x:%02x:%02x",
		       p[0], p[1], p[2], (addr >> 16) & 0xff,
		       (addr >> 8) & 0xff, addr & 0xff);

	if (ret < 0)
		return ret;
	if (ret >= len)
		return ret;

	return !(is_valid_ethaddr(p));
}

int tqc_parse_eeprom_mac(struct tqc_eeprom_data const *eeprom, char *buf,
			 size_t len)
{
	return tqc_parse_eeprom_mac_additional(eeprom, buf, len, 0);
}

int tqc_parse_eeprom_serial(struct tqc_eeprom_data const *eeprom, char *buf,
			    size_t len)
{
	unsigned int i;

	if (!buf || !eeprom)
		return -1;
	if (len < (sizeof(eeprom->serial) + 1))
		return -1;

	for (i = 0; i < (sizeof(eeprom->serial)) &&
	     isdigit(eeprom->serial[i]); i++)
		buf[i] = eeprom->serial[i];
	buf[i] = '\0';
	if (sizeof(eeprom->serial) != strlen(buf))
		return -1;

	return 0;
}

int tqc_parse_eeprom_id(struct tqc_eeprom_data const *eeprom, char *buf,
			size_t len)
{
	unsigned int i;

	if (!buf || !eeprom)
		return -1;
	if (len < (sizeof(eeprom->id) + 1))
		return -1;

	for (i = 0; i < sizeof(eeprom->id) && isprint(eeprom->id[i]) &&
	     isascii(eeprom->id[i]); ++i)
		buf[i] = eeprom->id[i];
	buf[i] = '\0';

	return 0;
}

/*
 * show_eeprom - display the contents of the module EEPROM
 */
int tqc_show_eeprom(struct tqc_eeprom_data const *eeprom, const char *id)
{
	/* must hold largest field of eeprom data */
	char safe_string[0x41];

	if (!eeprom)
		return -1;

	puts(id);
	puts(" EEPROM:\n");
	/* ID */
	tqc_parse_eeprom_id(eeprom, safe_string,
			      ARRAY_SIZE(safe_string));
	if (strncmp(safe_string, id, strlen(id)) == 0)
		printf("  ID: %s\n", safe_string);
	else
		puts("  unknown hardware variant\n");

	/* Serial number */
	if (tqc_parse_eeprom_serial(eeprom, safe_string,
				    ARRAY_SIZE(safe_string)) == 0)
		printf("  SN: %s\n", safe_string);
	else
		puts("  unknown serial number\n");
	/* MAC address */
	if (tqc_parse_eeprom_mac(eeprom, safe_string,
				 ARRAY_SIZE(safe_string)) == 0)
		printf("  MAC: %s\n", safe_string);
	else
		puts("  invalid MAC\n");

	return 0;
}

/*
 * read_eeprom - read the given EEPROM into memory
 */
int tqc_read_eeprom_at(unsigned int bus, unsigned int i2c_addr,
		       unsigned int alen, unsigned int addr,
		       struct tqc_eeprom_data *eeprom)
{
	return tqc_read_eeprom_buf(bus, i2c_addr, alen, addr, sizeof(*eeprom),
				   (uchar *)eeprom);
}

#if defined(CONFIG_SYS_I2C_EEPROM_ADDR_LEN)
int tqc_read_eeprom(unsigned int bus, unsigned int i2c_addr,
		    unsigned int addr, struct tqc_eeprom_data *eeprom)
{
	return tqc_read_eeprom_at(bus, i2c_addr,
				  CONFIG_SYS_I2C_EEPROM_ADDR_LEN,
				  addr, eeprom);
}
#endif

#ifdef CONFIG_TQC_VARD
static int do_tqeeprom(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	if (argc < 2)
		return CMD_RET_USAGE;
	if (argc > 3)
		return CMD_RET_USAGE;

	if (strcmp(argv[1], "write") == 0) {
		ulong addr = simple_strtoul(argv[2], NULL, 16);
		int ret;
#ifdef CONFIG_DM_I2C
		struct udevice *dev;
#else
		unsigned int oldbus;
#endif

		if (!addr)
			return -1;

#ifdef CONFIG_DM_I2C
		ret = i2c_get_chip_for_busnum(TQC_VARD_BUS,
					      TQC_VARD_ADDR, 1, &dev);
		if (ret) {
			debug("%s: Cannot find I2C chip for bus %d\n",
			      __func__, TQC_VARD_BUS);
			return ret;
		}

		ret = dm_i2c_write(dev, 0, (void *)addr,
				   sizeof(struct tqc_eeprom_data));
#else
		oldbus = i2c_get_bus_num();
		i2c_set_bus_num(TQC_VARD_BUS);
		ret = i2c_write(TQC_VARD_ADDR, 0, 1, (void *)addr,
				sizeof(struct tqc_eeprom_data));
		i2c_set_bus_num(oldbus);
#endif
		return ret;
	}

	if (strcmp(argv[1], "read") == 0) {
		ulong addr = simple_strtoul(argv[2], NULL, 16);

		return tqc_read_eeprom_buf(TQC_VARD_BUS, TQC_VARD_ADDR,
				  1, 0,
				  sizeof(struct tqc_eeprom_data),
				  (void *)addr);
	}

	if (strcmp(argv[1], "crc") == 0) {
		int ret;
		struct tqc_eeprom_data eeprom_for_crc;
		uint16_t crc;

		ret = tqc_read_eeprom_buf(TQC_VARD_BUS, TQC_VARD_ADDR,
				  1, 0,
				  sizeof(struct tqc_eeprom_data),
				  (void *)&eeprom_for_crc);
		if (ret)
			return ret;

		tqc_vard_cmp_crc(&eeprom_for_crc, &crc);
		printf("CRC16 stored: %04x, calculated: %04x\n",
			eeprom_for_crc.crc, crc);
		return 0;
	}

	if (strcmp(argv[1], "protect") == 0) {
		printf("WARNING! Write-protection is permanent and\n");
		printf("cannot be released by any command or power-cycle!\n");
		printf("Device will stop ACK'ing on address 0x37.\n");
		printf("Really enable permanent write-protection?\n");
		
		if (confirm_yesno()) {
			uint8_t dummy = 0;
#ifdef CONFIG_DM_I2C
			struct udevice *dev;
			int ret = i2c_get_chip_for_busnum(TQC_VARD_BUS,
							0x37, 1, &dev);
			if (ret) {
				debug("%s: Cannot find I2C chip for bus %d\n",
			              __func__, TQC_VARD_BUS);
				return ret;
			}

			dm_i2c_write(dev, 0, &dummy, sizeof(dummy));
#else
			unsigned int oldbus = i2c_get_bus_num();
			i2c_set_bus_num(TQC_VARD_BUS);
			i2c_write(0x37, 0, 1, &dummy, sizeof(dummy));
			i2c_set_bus_num(oldbus);
#endif
			printf("Write-protection enabled\n");
			return 0;
		}
		printf("Abort - Write-protection NOT enabled\n");
		return CMD_RET_FAILURE;
	}

	return CMD_RET_USAGE;
}

U_BOOT_CMD(tqeeprom, 3, 0, do_tqeeprom,
	   "manage TQ module eeprom",
	   "<cmd>:\n"
	   "write <addr> - write full image to eeprom\n"
	   "read <addr>  - read whole eeprom to memory\n"
	   "crc          - calculate and compare CRC16\n"
	   "protect      - write-protect lower 128 byte (PERMANENT!)\n"
);
#endif
#endif
