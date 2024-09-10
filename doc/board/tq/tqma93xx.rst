.. SPDX-License-Identifier: GPL-2.0+

TQ-Systems GmbH TQMa93xx SoM Family
===================================

The TQ-Systems TQMa93xx[CA/LA] are System-on-Module based on the NXP i.MX93 SoC family.

Module variants
---------------

Currently, following TQMa93xx[CA/LA] variants with LPDDR4x RAM are supported:

* 1 GiB
* 1.5 GiB
* 2 GiB

The RAM configuration, form factor and other optional features (SPI-NOR,
Secure Element, ...) are detected automatically based on VARD
(Variant and Revision Detection) data read from an EEPROM.

Base boards / configurations
----------------------------

The following combinations of base board and SOM are supported:

* TQ-Systems TQMa93xxLA (REV.010x) with adaptor on MBa93xxCA (REV.020x) and TQ-Systems TQMa93xxCA (REV.010x) on
  MBa93xxCA (REV.020x)

  * `tqma93xx_mba93xxca_defconfig`

  * `tqma93xx_mba93xxca_uuu_defconfig`

* TQ-Systems TQMa93xxLA (REV.010x) soldered on MBa93xxLA (REV.020x)

  * `tqma93xx_mba93xxla_defconfig`

  * `tqma93xx_mba93xxla_uuu_defconfig`

* TQ-Systems TQMa93xxLA (REV.010x) with adaptor on MBa91xxCA (REV.010x) and TQ-Systems TQMa93xxCA (REV.010x) on
  MBa91xxCA (REV.010x)

  * `tqma93xx_mba91xxca_defconfig`

  * `tqma93xx_mba91xxca_uuu_defconfig`

Boot sequence
-------------

The boot firmware on the i.MX93 is loaded in multiple stages. Two of these stages
are provided by this U-Boot repository:

* i.MX93 U-Boot SPL
* i.MX93 U-Boot (`u-boot.img`, combined with DTB)

The SPL performs the following actions:

* Initialize CPU
* Initialize DDR controller
* Load U-Boot and DTB into DDR RAM
* Boot TF-A

The next stage is handled by the TF-A. It will optionally start the OP-TEE and
the start full A53 U-Boot from `u-boot.img`.

Finally, the full U-Boot runs, which can provide a command line interface
and boot into the actual OS depending on its configuration.

Build
-----

The SPL and full U-Boot are both built from the given defconfigs. The build
requires an ARM-v8/AArch64 cross compiler.

Boot from eMMC boot partitions
------------------------------

TBD.

MBa93xx[CA/LA] DIP settings for boot sources
--------------------------------------------

BOOT\_MODE can be configured using DIP switch S1.

+----------+-----------------------+------+------+------+------+
| Bootmode | Description           | S1-4 | S1-3 | S1-2 | S1-1 |
+==========+=======================+======+======+======+======+
| 0000     | Boot from fuses       | OFF  | OFF  | OFF  | OFF  |
+----------+-----------------------+------+------+------+------+
| 0001     | Serial Downloader     | OFF  | OFF  | OFF  | ON   |
+----------+-----------------------+------+------+------+------+
| 0010     | e-MMC (USDHC1)        | OFF  | OFF  | ON   | OFF  |
+----------+-----------------------+------+------+------+------+
| 0011     | SD Card (USDHC2)      | OFF  | OFF  | ON   | ON   |
+----------+-----------------------+------+------+------+------+
| 0100     | QSPI (FlexSPI NOR)    | OFF  | ON   | OFF  | OFF  |
+----------+-----------------------+------+------+------+------+

**NOTE:** LPB boot modes not supported yet.

