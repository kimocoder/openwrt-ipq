SUBTARGET:=generic
BOARDNAME:=QTI IPQ60xx(64bit) based boards
CPU_TYPE:=cortex-a53

define Target/Description
	Build images for ipq60xx 64 bit system.
endef

DEFAULT_PACKAGES += \
	sysupgrade-helper
