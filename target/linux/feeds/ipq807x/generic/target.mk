SUBTARGET:=generic
BOARDNAME:=QTI IPQ807x(64bit) based boards
CPU_TYPE:=cortex-a53

define Target/Description
	Build images for ipq807x 64 bit system.
endef

DEFAULT_PACKAGES += \
	sysupgrade-helper
