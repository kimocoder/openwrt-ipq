define Device/EmmcImage
        IMAGES += factory.bin sysupgrade.bin
        IMAGE/factory.bin := append-rootfs | pad-rootfs | pad-to 64k
        IMAGE/sysupgrade.bin/squashfs := append-rootfs | pad-to 64k | sysupgrade-tar rootfs=$$$$@ | append-metadata
endef

define Device/redmi_ax3000
  $(call Device/FitImage)
  $(call Device/UbiFit)
  SOC := ipq5000
  DEVICE_VENDOR := Redmi
  DEVICE_MODEL := AX3000
  DEVICE_ALT0_VENDOR := Xiaomi
  DEVICE_ALT0_MODEL := CR880X
  DEVICE_ALT0_VARIANT := (M81 version)
  DEVICE_ALT1_VENDOR := Xiaomi
  DEVICE_ALT1_MODEL := CR880X
  DEVICE_ALT1_VARIANT := (M79 version)
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  DEVICE_DTS_CONFIG := config@mp02.1
  DEVICE_PACKAGES := ipq-wifi-redmi_ax3000
endef
TARGET_DEVICES += redmi_ax3000

define Device/wallys_dr5018
  $(call Device/FitImage)
  $(call Device/EmmcImage)
  SOC := ipq5018
  DEVICE_VENDOR := Wallys
  DEVICE_MODEL := DR5018
  BLOCKSIZE := 64k
  PAGESIZE := 2048
  KERNEL_SIZE := 6144k
  DEVICE_DTS_CONFIG := config@mp03.5-c1
  DEVICE_PACKAGES := ipq-wifi-wallys_dr5018
  IMAGE/factory.bin := append-kernel | pad-to $${KERNEL_SIZE}  |  append-rootfs | append-metadata
endef
TARGET_DEVICES += wallys_dr5018

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
TARGET_DEVICES += qcom_mp03.5-c1
