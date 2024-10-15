
ARCH:=arm
SUBTARGET:=ipq807x_32
BOARDNAME:=QTI IPQ807x(32bit) based boards
CPU_TYPE:=cortex-a7

define Target/Description
	Build firmware image for IPQ807x SoC devices.
endef

DEFAULT_PACKAGES += \
	uboot-2016-ipq807x uboot-2016-ipq807x-debug uboot-2016-ipq807x_tiny sysupgrade-helper \
	fwupgrade-tools
