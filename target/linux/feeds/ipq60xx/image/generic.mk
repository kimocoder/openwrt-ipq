
define Device/qcom_cpxx
	$(call Device/MultiDTBFitImage)
	DEVICE_VENDOR := Qualcomm Technologies, Inc.
	DEVICE_MODEL := AP-CPXX
	DEVICE_VARIANT :=
	BOARD_NAME := ap-cpxx
	SOC := ipq6018
	KERNEL_INSTALL := 1
	KERNEL_SIZE := $(if $(CONFIG_DEBUG),8680k,6500k)
	IMAGE_SIZE := 25344k
	IMAGE/sysupgrade.bin := append-kernel | pad-to $$$$(KERNEL_SIZE) | append-rootfs | pad-rootfs | append-metadata
endef
TARGET_DEVICES += qcom_cpxx

define Device/qcom_cp01-c1
	$(call Device/FitImageLzma)
	DEVICE_VENDOR := Qualcomm Technologies, Inc.
	DEVICE_MODEL := AP-CP01-C1
	DEVICE_VARIANT :=
	BOARD_NAME := ap-cp01-c1
	BUILD_DTS_ipq6018-cp01-c1 := 1
	SOC := ipq6018
	KERNEL_INSTALL := 1
	KERNEL_SIZE := $(if $(CONFIG_DEBUG),8680k,6500k)
	IMAGE_SIZE := 25344k
	IMAGE/sysupgrade.bin := append-kernel | pad-to $$$$(KERNEL_SIZE) | append-rootfs | pad-rootfs | append-metadata
endef
TARGET_DEVICES += qcom_cp01-c1

define Device/qcom_cp01-c3
	$(call Device/FitImageLzma)
	DEVICE_VENDOR := Qualcomm Technologies, Inc.
	DEVICE_MODEL := AP-CP01-C3
	DEVICE_VARIANT :=
	BOARD_NAME := ap-cp01-c3
	BUILD_DTS_ipq6018-cp01-c3 := 1
	SOC := ipq6018
	KERNEL_INSTALL := 1
	KERNEL_SIZE := $(if $(CONFIG_DEBUG),8680k,6500k)
	IMAGE_SIZE := 25344k
	IMAGE/sysupgrade.bin := append-kernel | pad-to $$$$(KERNEL_SIZE) | append-rootfs | pad-rootfs | append-metadata
endef
TARGET_DEVICES += qcom_cp01-c3

define Device/qcom_cp01-c4
	$(call Device/FitImageLzma)
	DEVICE_VENDOR := Qualcomm Technologies, Inc.
	DEVICE_MODEL := AP-CP01-C4
	DEVICE_VARIANT :=
	BOARD_NAME := ap-cp01-c4
	BUILD_DTS_ipq6018-cp01-c4 := 1
	SOC := ipq6018
	KERNEL_INSTALL := 1
	KERNEL_SIZE := $(if $(CONFIG_DEBUG),8680k,6500k)
	IMAGE_SIZE := 25344k
	IMAGE/sysupgrade.bin := append-kernel | pad-to $$$$(KERNEL_SIZE) | append-rootfs | pad-rootfs | append-metadata
endef
TARGET_DEVICES += qcom_cp01-c4

define Device/qcom_cp01-c5
	$(call Device/FitImageLzma)
	DEVICE_VENDOR := Qualcomm Technologies, Inc.
	DEVICE_MODEL := AP-CP01-C5
	DEVICE_VARIANT :=
	BOARD_NAME := ap-cp01-c5
	BUILD_DTS_ipq6018-cp01-c5 := 1
	SOC := ipq6018
	KERNEL_INSTALL := 1
	KERNEL_SIZE := $(if $(CONFIG_DEBUG),8680k,6500k)
	IMAGE_SIZE := 25344k
	IMAGE/sysupgrade.bin := append-kernel | pad-to $$$$(KERNEL_SIZE) | append-rootfs | pad-rootfs | append-metadata
endef
TARGET_DEVICES += qcom_cp01-c5

define Device/qcom_db-cp01
	$(call Device/FitImageLzma)
	DEVICE_VENDOR := Qualcomm Technologies, Inc.
	DEVICE_MODEL := DB-CP01
	DEVICE_VARIANT :=
	BOARD_NAME := db-cp01
	BUILD_DTS_ipq6018-db-cp01 := 1
	SOC := ipq6018
	KERNEL_INSTALL := 1
	KERNEL_SIZE := $(if $(CONFIG_DEBUG),8680k,6500k)
	IMAGE_SIZE := 25344k
	IMAGE/sysupgrade.bin := append-kernel | pad-to $$$$(KERNEL_SIZE) | append-rootfs | pad-rootfs | append-metadata
endef
TARGET_DEVICES += qcom_db-cp01

define Device/qcom_cp02-c1
	$(call Device/FitImageLzma)
	DEVICE_VENDOR := Qualcomm Technologies, Inc.
	DEVICE_MODEL := AP-CP02-C1
	DEVICE_VARIANT :=
	BOARD_NAME := ap-cp02-c1
	BUILD_DTS_ipq6018-cp02-c1 := 1
	SOC := ipq6018
	KERNEL_INSTALL := 1
	KERNEL_SIZE := $(if $(CONFIG_DEBUG),8680k,6500k)
	IMAGE_SIZE := 25344k
	IMAGE/sysupgrade.bin := append-kernel | pad-to $$$$(KERNEL_SIZE) | append-rootfs | pad-rootfs | append-metadata
endef
TARGET_DEVICES += qcom_cp02-c1

define Device/qcom_cp03-c1
	$(call Device/FitImageLzma)
	DEVICE_VENDOR := Qualcomm Technologies, Inc.
	DEVICE_MODEL := DP-CP03-C1
	DEVICE_VARIANT :=
	BOARD_NAME := ap-cp03-c1
	BUILD_DTS_ipq6018-cp03-c1 := 1
	SOC := ipq6018
	KERNEL_INSTALL := 1
	KERNEL_SIZE := $(if $(CONFIG_DEBUG),8680k,6500k)
	IMAGE_SIZE := 25344k
	IMAGE/sysupgrade.bin := append-kernel | pad-to $$$$(KERNEL_SIZE) | append-rootfs | pad-rootfs | append-metadata
endef
TARGET_DEVICES += qcom_cp03-c1
