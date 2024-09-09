SUBTARGET:=ipq60xx
BOARDNAME:=Qualcomm Atheros IPQ60xx
DEFAULT_PACKAGES += \
	ath11k-firmware-ipq6018 nss-firmware-ipq6018 kmod-qca-nss-ecm kmod-qca-nss-drv kmod-qca-nss-dp \
	kmod-qca-nss-drv-pppoe kmod-qca-nss-drv-pptp \
	
define Target/Description
	Build firmware images for Qualcomm Atheros IPQ60xx based boards.
endef
