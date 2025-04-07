# U-Boot for the TQ Systems TQMLS102xA modules

This file contains information for the port of
U-Boot to the TQ Systems TQMLS102xA modules.

## Boot source

The following boot sources are supported:

- e-MMC/SD
- QSPI NOR

## Building

To build U-Boot for the TQ Systems TQMLS102xA modules:
```
make <module>_<baseboard>_[ecc]_<boot>_defconfig
make
```
`<module>` is a placeholder for the module variant:
- `tqmls102xa`		- generic TQMLS102xA build (1.0 GHz, 1 GiB DDR)
- `tqmls1021a-ae`	- build for variant TQMLS1021A-AE (1.2 GHz, 2 GiB DDR)
			  (not all combinations available)

`<baseboard>` is a placeholder for the board the module is plugged into:
- `mbls102xa`		- Starter kit STKLS102xA by TQ

`[ecc]` is an optional boolean parameter that enables usage of ECC RAM:
- `ecc`		- ECC enabled
- -not given-	- ECC disabled

Note: `ecc` requires a module with additionally installed ECC RAM!

`<boot>` is a placeholder for the boot device
- `mmcsd`		- build to boot from device on esDHC (eMMC or SD card)
- `qspi`		- build to boot from flash on QSPI interface

This gives the following configurations:

- `tqmls102xa_mbls102xa_mmcsd_defconfig`
- `tqmls102xa_mbls102xa_qspi_defconfig`
- `tqmls102xa_mbls102xa_ecc_mmcsd_defconfig`
- `tqmls102xa_mbls102xa_ecc_qspi_defconfig`
- `tqmls1021a-ae_mbls102xa_ecc_qspi_defconfig`

Note: RCW is not part of u-boot. It has to be configured and created
externally and put together with u-boot accordingly.

RCW repository: https://github.com/tq-systems/rcw

## Merging U-Boot and RCW

Depending on the boot media the bootstream has to be created differently. Below are example
commands for creating the bootstream manually

### SD card
```
mkimage -n rcw.bin -T pblimage -A arm -C none -a 0x10000000 -e 0 -d spl/u-boot-spl.bin spl/${spl_file}
objcopy --gap-fill=0xff -I binary -O binary --pad-to=0x1c000 --gap-fill=0xff spl/${spl_file} u-boot-rcw.bin
cat u-boot.bin >> u-boot-rcw.bin
```

### QSPI
`bytesawp.tcl` can be found at https://github.com/tq-systems/meta-tq/tree/scarthgap/meta-tq/recipes-bsp/swap-file-endianess
```
objcopy --gap-fill=0xff -I binary -O binary --pad-to=0x10000 --gap-fill=0xff rcw.bin rcw.temp.bin
cat rcw.temp.bin u-boot.bin > rcw_uboot.bin
objcopy --gap-fill=0xff -I binary -O binary --pad-to=0x40000 --gap-fill=0xff rcw_uboot.bin u-boot-rcw.bin.temp
tclsh byteswap.tcl u-boot-rcw.bin.temp u-boot-rcw.bin 8
```
