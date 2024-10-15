define Device/qcom_mpxx
        $(call Device/MultiDTBFitImage)
	DEVICE_VENDOR := Qualcomm Technologies, Inc.
	DEVICE_MODEL := AP-MPXX
	DEVICE_VARIANT :=
	BOARD_NAME := ap-mpxx
	SOC := ipq5018
	KERNEL_INSTALL := 1
	KERNEL_SIZE := $(if $(CONFIG_DEBUG),8680k,6500k)
	IMAGE_SIZE := 25344k
	IMAGE/sysupgrade.bin := append-kernel | pad-to $$$$(KERNEL_SIZE) | append-rootfs | pad-rootfs | append-metadata
endef
TARGET_DEVICES += qcom_mpxx

define Device/qcom_mp03.1
	$(call Device/FitImageLzma)
	DEVICE_VENDOR := Qualcomm Technologies, Inc.
	DEVICE_MODEL := AP-MP03-C1
	DEVICE_VARIANT := C1
	BOARD_NAME := ap-mp03.1-c1
	BUILD_DTS_ipq5018-mp03.1 := 1
	SOC := ipq5018
	KERNEL_INSTALL := 1
	KERNEL_SIZE := 6096k
	IMAGE_SIZE := 25344k
	IMAGE/sysupgrade.bin := append-kernel | pad-to $$$$(KERNEL_SIZE) | append-rootfs | pad-rootfs | append-metadata
endef
TARGET_DEVICES += $(if $(CONFIG_LINUX_5_4), qcom_mp03.1)

define Device/qcom_mp03.3
	$(call Device/FitImageLzma)
	DEVICE_VENDOR := Qualcomm Technologies, Inc.
	DEVICE_MODEL := AP-MP03.3-C1
	DEVICE_VARIANT := C1
	BOARD_NAME := ap-mp03.3-c1
	BUILD_DTS_ipq5018-mp03.3 := 1
	SOC := ipq5018
	KERNEL_INSTALL := 1
	KERNEL_SIZE := 6096k
	IMAGE_SIZE := 25344k
	IMAGE/sysupgrade.bin := append-kernel | pad-to $$$$(KERNEL_SIZE) | append-rootfs | pad-rootfs | append-metadata
endef
TARGET_DEVICES += $(if $(CONFIG_LINUX_5_4), qcom_mp03.3)

define Device/qcom_mp03.5-c1
	$(call Device/FitImageLzma)
	DEVICE_VENDOR := Qualcomm Technologies, Inc.
	DEVICE_MODEL := AP-MP03.5-C1
	DEVICE_VARIANT := C1
	BOARD_NAME := ap-mp03.5-c1
	BUILD_DTS_ipq5018-mp03.5-c1 := 1
	SOC := ipq5018
	KERNEL_INSTALL := 1
	KERNEL_SIZE := 6096k
	IMAGE_SIZE := 25344k
	IMAGE/sysupgrade.bin := append-kernel | pad-to $$$$(KERNEL_SIZE) | append-rootfs | pad-rootfs | append-metadata
endef
TARGET_DEVICES += $(if $(CONFIG_LINUX_5_4), qcom_mp03.5-c1)
