SUBTARGET:=generic
BOARDNAME:=QTI IPQ54xx(64bit) based boards
CPU_TYPE:=cortex-a55

define Target/Description
	Build images for ipq54xx 64 bit system.
endef

DEFAULT_PACKAGES += \
	uboot-ipq5424-mmc uboot-ipq5424-norplusmmc \
	uboot-ipq5424-norplusnand uboot-ipq5424-nand
