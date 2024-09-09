SUBTARGET:=ipq50xx
BOARDNAME:=Qualcomm Atheros IPQ50xx
DEFAULT_PACKAGES += \
	kmod-gpio-button-hotplug \
	uboot-envtools \
	nss-firmware-ipq5018 \
	kmod-ath11k kmod-ath11k-ahb kmod-ath11k-pci wpad-basic-wolfssl \
	kmod-qca-nss-ecm kmod-qca-nss-drv kmod-qca-nss-ecm kmod-qca-nss-dp \
	kmod-qca-nss-drv kmod-qca-nss-drv-pppoe kmod-qca-nss-drv-pptp \

define Target/Description
	Build firmware images for Qualcomm Atheros IPQ50xx based boards.
endef
