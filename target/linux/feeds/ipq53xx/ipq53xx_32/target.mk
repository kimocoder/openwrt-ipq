
ARCH:=arm
SUBTARGET:=ipq53xx_32
BOARDNAME:=QTI IPQ53xx(32bit) based boards
CPU_TYPE:=cortex-a7

define Target/Description
	Build firmware image for IPQ53xx SoC devices.
endef

DEFAULT_PACKAGES += \
	uboot-ipq5332-mmc32 \
	uboot-ipq5332-norplusmmc32 \
	uboot-ipq5332-norplusnand32 \
	uboot-ipq5332-nand32 \
	sysupgrade-helper \
	uboot-ipq5332-tiny_nand32 \
	uboot-ipq5332-tiny_v2_nand32 \
	uboot-ipq5332-tiny_nand32_64M \
	uboot-ipq5332-tiny_v2_nand32_64M \
	uboot-ipq5332-tiny_nor32 \
	uboot-ipq5332-tiny_norplusnand32 \
	uboot-ipq5332-tiny_v2_norplusnand32
