include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=qca-nss-phy
PKG_SOURCE_PROTO:=git
PKG_BRANCH:=master
PKG_RELEASE:=1

include $(INCLUDE_DIR)/local-development.mk
ifeq ($(DUMP)$(PKG_VERSION),)
  PKG_VERSION:=$(shell git ls-remote $(PKG_SOURCE_URL) $(PKG_BRANCH) | cut -b -7)
endif

include $(INCLUDE_DIR)/package.mk

define KernelPackage/qca-nss-phy
  SECTION:=kernel
  CATEGORY:=Kernel modules
  SUBMENU:=Network Devices
  TITLE:=QCA-NSS-PHY driver
  FILES:=$(PKG_BUILD_DIR)/nss_ext/qca-nss-phy.ko
  PROVIDES:=qca-nss-phy
  AUTOLOAD:=$(call AutoLoad,08,qca-nss-phy)
endef

define KernelPackage/qca-nss-phy/Description
This package contains drivers for QCA PHYs such as Manhattan PHY and NAPA PHY.
It also contains vendor specific extensions for QCA PHYs which are not supported
by upstream drivers.
endef

define KernelPackage/qca81xx
  SECTION:=kernel
  CATEGORY:=Kernel modules
  SUBMENU:=Network Devices
  TITLE:=QCA81XX PHY driver
  FILES:=$(PKG_BUILD_DIR)/linux_std/qca81xx/qca81xx-phy.ko
  PROVIDES:=qca81xx
  AUTOLOAD:=$(call AutoLoad,07,qca81xx-phy)
endef

define KernelPackage/qca81xx/Description
This package contains qca81xx linux phy driver, it will be upstream PHY driver
of LAGUNA PHY.
endef

define KernelPackage/qca8084
  SECTION:=kernel
  CATEGORY:=Kernel modules
  SUBMENU:=Network Devices
  TITLE:=QCA8084 PHY driver
  FILES:=$(PKG_BUILD_DIR)/linux_std/qca8084/qca8084-phy.ko
  PROVIDES:=qca8084
  AUTOLOAD:=$(call AutoLoad,07,qca8084-phy)
endef

define KernelPackage/qca8084/Description
This package contains qca8084 Linux PHY driver
endef

ifdef CONFIG_TARGET_NAME
QCA_NSS_PHY_CONFIG_OPTS+= \
	TARGET_NAME=$(CONFIG_TARGET_NAME)
endif

QCA_NSS_PHY_CONFIG_OPTS+= \
	LINUX_DIR=$(LINUX_DIR) \
	CROSS_COMPILE=$(TARGET_CROSS) \
	ARCH=$(LINUX_KARCH)

define Build/Prepare
	$(CP) $(LOCAL_SRC)/* $(PKG_BUILD_DIR)/  -a
endef

define Build/Compile
	$(KERNEL_MAKE) M=$(PKG_BUILD_DIR)/nss_ext $(strip $(QCA_NSS_PHY_CONFIG_OPTS))
	$(KERNEL_MAKE) M=$(PKG_BUILD_DIR)/linux_std/qca81xx $(strip $(QCA_NSS_PHY_CONFIG_OPTS))
	$(KERNEL_MAKE) M=$(PKG_BUILD_DIR)/linux_std/qca8084 $(strip $(QCA_NSS_PHY_CONFIG_OPTS))
endef

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR)/usr/include/qca-nss-phy
	$(CP) -rf $(PKG_BUILD_DIR)/nss_ext/*h $(STAGING_DIR)/usr/include/qca-nss-phy
endef

define KernelPackage/qca-nss-phy/install
endef
$(eval $(call KernelPackage,qca-nss-phy))
$(eval $(call KernelPackage,qca81xx))
$(eval $(call KernelPackage,qca8084))
