
ARCH:=arm
SUBTARGET:=ipq60xx_32
BOARDNAME:=QTI IPQ60xx(32bit) based boards
CPU_TYPE:=cortex-a7

define Target/Description
	Build firmware image for IPQ53xx SoC devices.
endef

DEFAULT_PACKAGES += \
	uboot-2016-ipq6018 uboot-2016-ipq6018-debug uboot-2016-ipq6018_tiny fwupgrade-tools \
	sysupgrade-helper
