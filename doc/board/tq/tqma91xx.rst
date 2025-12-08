.. SPDX-License-Identifier: GPL-2.0+

TQ-Systems GmbH TQMa91xx SoM Family
===================================

The TQ-Systems TQMa91xx[CA/LA] are System-on-Module based on the NXP i.MX91 SoC family.
The SoM is pin compatible with TQMa93xx[CA/LA] SoM.

Module variants
---------------

Currently, following TQMa91xx[CA/LA] variants with LPDDR4 RAM are supported:

* 1 GiB

The RAM configuration, form factor and other optional features (SPI-NOR,
Secure Element, ...) are detected automatically based on VARD
(Variant and Revision Detection) data read from an EEPROM.

Base boards / configurations
----------------------------

The following combinations of base board and SOM are supported:

* TQ-Systems TQMa91xxLA (REV.010x) with adaptor on MBa91xxCA (REV.020x) and TQ-Systems TQMa91xxCA (REV.010x) on
  MBa91xxCA (REV.020x)

  * `tqma91xx_mba91xxca_defconfig`

  * `tqma91xx_mba91xxca_uuu_defconfig`

* TQ-Systems TQMa91xxLA (REV.010x) with adaptor on MBa93xxCA (REV.020x) and TQ-Systems TQMa91xxCA (REV.010x) on
  MBa93xxCA (REV.020x)

  * `tqma91xx_mba93xxca_defconfig`

  * `tqma91xx_mba93xxca_uuu_defconfig`

Boot sequence
-------------

The boot firmware on the i.MX91 is loaded in multiple stages. Two of these stages
are provided by this U-Boot repository:

* i.MX91 U-Boot SPL
* i.MX91 U-Boot (`u-boot.img`, combined with DTB)

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

Boot from SD and eMMC
---------------------

**Attention**: partition in this context means the eMMC hardware partitions. This is unrelated to MBR or
GPT partitions in eMMC user partition.

The following table applies:

+------------+-----------------+-------------------+
|   Device   |     offset      | Block (512 Bytes) |
+============+=================+===================+
|  SD-Card   | 32 KiB (0x8000) |     64 / 0x40     |
+------------+-----------------+-------------------+
| eMMC USER  | 32 KiB (0x8000) |     64 / 0x40     |
+------------+-----------------+-------------------+
| eMMC BOOT  | 0 KiB  (0x0000) |      0 / 0x00     |
+------------+-----------------+-------------------+

.. code-block:: bash

    # assign bstart with start block number in hex
    # use correct device
    # eMMC:                0
    # SD-Card:             1
    # use correct partition
    # SD-Card:             0
    # eMMC user partition: 0
    # eMMC boot partition: 1 or 2
    # assign partnum with partition number to use

    setenv devnum <0|1>
    setenv bstart <block number>
    setenv partnum <part number>
    tftp <bootstream>
    setexpr bsz ${filesize} + 1ff
    setexpr bsz ${bsz} / 200
    printenv bsz
    mmc dev ${devnum} ${partnum}
    mmc write ${loadaddr} ${bstart} ${bsz}
    mmc dev ${devnum} 0

MBa91xx[CA/LA] and MBa91xxCA DIP settings for boot sources
----------------------------------------------------------

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

Memory usage in SPL
-------------------

SRAM (OCRAM)

+------------+------------+------+---------------------------------------+
| Start      | End        | Size | Note                                  |
+============+============+======+=======================================+
| 0x20480000 | 0x20497FFF |  96K | ROM reserved                          |
+------------+------------+------+---------------------------------------+
| 0x20498000 | 0x20499FFF |   8K | initial image                         |
+------------+------------+------+---------------------------------------+
| 0x2049A000 | 0x2049BFFF |   8K | SPL_BSS_START_ADDR + SPL_BSS_MAX_SIZE |
+------------+------------+------+---------------------------------------+
| 0x2049C000 | 0x2049FFFF |  16K | SAVED_DRAM_TIMING_BASE                |
+------------+------------+------+---------------------------------------+
| 0x204A0000 | 0x204BFFFF | 128K | SPL (SPL_TEXT_BASE + SPL_MAX_SIZE)    |
+------------+------------+------+---------------------------------------+
| 0x204C0000 | 0x204DFFFF | 128K | TF-A (BL31_BASE + BL31_LIMIT)         |
+------------+------------+------+---------------------------------------+
| 0x204DFDD0 |            |      | SPL_STACK before DRAM                 |
+------------+------------+------+---------------------------------------+

DRAM

+------------+------------+------+---------------------------------------+
| Start      | End        | Size | Note                                  |
+============+============+======+=======================================+
| 0x80200000 |            |      | PLAT_NS_IMAGE_OFFSET (TF-A            |
|            |            |      | CONFIG_TEXT_BASE (U-Boot)             |
+------------+------------+------+---------------------------------------+
| 0x96000000 | 0x97FFFFFF |      | BL32_BASE + BL32_SIZE (optee)         |
+------------+------------+------+---------------------------------------+
