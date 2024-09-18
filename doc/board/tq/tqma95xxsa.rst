.. SPDX-License-Identifier: GPL-2.0+

TQ-Systems GmbH TQMa95xxSA SoM Family
=====================================

The TQ-Systems TQMa93xxSA is a System-on-Module based on the NXP i.MX95 SoC.

Module variants
---------------

Currently, following TQMa95xxSA variants with LPDDR5 RAM are supported:

* 2 GiB
* 4 GiB

The RAM configuration 

TODO: VARD (Variant and Revision Detection) data from EEPROM not implemented yet.

Base boards / configurations
----------------------------

The following combinations of base board and SOM are supported:

* TQ-Systems TQMa95xxSA (REV.010x) on MB-SMARC-2 (REV.010x)

  * `tqma95xxsa_2gb_mb_smarc_2_defconfig`

  * `tqma95xxsa_4gb_mb_smarc_2_defconfig`

Boot sequence
-------------

The boot firmware on the i.MX95 is loaded in multiple stages. Two of these stages
are provided by this U-Boot repository:

* i.MX95 U-Boot SPL
* i.MX95 U-Boot (`u-boot.img`, combined with DTB)

The SPL itself will be started from System Manager firmware running in Cortex-M33.
Complete resource management is done by System Manager firmware via SCMI firmware calls.
The configuration of System Manager firmware and U-Boot must match.

The SPL performs the following actions:

* Initialize Cortex-A55 CPU
* Load U-Boot and DTB into DDR RAM
* Boot TF-A

The next stage is handled by the TF-A. It will optionally start the OP-TEE and
starts Cortex-A55 U-Boot proper from `u-boot.img`.

Finally, the full U-Boot runs, which can provide a command line interface
and boot into the actual OS depending on its configuration.

Build
-----

The SPL and full U-Boot are both built from the given defconfigs. The build
requires an ARM-v8/AArch64 cross compiler.

Boot from eMMC boot partitions
------------------------------

TBD.

MB-SMARC-2 DIP settings for boot sources
----------------------------------------

BOOT\_MODE can be configured using DIP switch S3.

+----------+-----------------------+------+------+------+------+
| Bootmode | Description           | S3-1 | S3-2 | S3-3 | S3-4 |
+==========+=======================+======+======+======+======+
| 0000     | Boot from fuses       | OFF  | OFF  | OFF  | OFF  |
+----------+-----------------------+------+------+------+------+
| 0001     | Serial Downloader     | OFF  | OFF  | OFF  | ON   |
+----------+-----------------------+------+------+------+------+
| 0010     | e-MMC (USDHC1)        | ON   | OFF  | OFF  | OFF  |
+----------+-----------------------+------+------+------+------+
| 0011     | SD Card (USDHC2)      | OFF  | ON   | ON   | OFF  |
+----------+-----------------------+------+------+------+------+
| 0100     | QSPI (FlexSPI NOR)    | ON   | ON   | OFF  | OFF  |
+----------+-----------------------+------+------+------+------+

**NOTE:** S3-4 enforces serial downloader mode and must be set, even if
the Boot ROM of i.MX95 will enter serial downloader mode in case of
error. S3-4 is needed to route the USB Signals from X4 to the correct
USB controller of i.MX95

