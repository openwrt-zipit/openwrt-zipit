#
# Copyright (C) 2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

WPAN_MENU:=WPAN

define KernelPackage/ieee802154
  SUBMENU:=$(WPAN_MENU)
  TITLE:=IEEE802.15.4 support
  KCONFIG:= \
	CONFIG_IEEE802154 \
	CONFIG_IEEE802154_PROTO_DEBUG=y
  DEPENDS:=+kmod-crc-ccitt
  FILES:= \
	$(LINUX_DIR)/net/ieee802154/ieee802154.ko \
	$(LINUX_DIR)/net/ieee802154/af_802154.ko
  AUTOLOAD:=$(call AutoLoad,90,ieee802154 af_802154)
endef

define KernelPackage/ieee802154/description
  IEEE Std 802.15.4 defines a low data rate, low power and low
  complexity short range wireless personal area networks. It was
  designed to organise networks of sensors, switches, etc automation
  devices. Maximum allowed data rate is 250 kb/s and typical personal
  operating space around 10m.
endef

$(eval $(call KernelPackage,ieee802154))

define KernelPackage/mac802154
  SUBMENU:=$(WPAN_MENU)
  TITLE:=MAC802.15.4 support
  KCONFIG:= \
	CONFIG_MAC802154 \
	CONFIG_MAC802154_PROTO_DEBUG=y \
	CONFIG_IEEE802154_DRIVERS=y \
	CONFIG_IEEE802154_DRIVERS_DEBUG=y
  DEPENDS:=+kmod-ieee802154
  FILES:=$(LINUX_DIR)/net/mac802154/mac802154.ko
  AUTOLOAD:=$(call AutoLoad,91,mac802154)
endef

define KernelPackage/mac802154/description
  This option enables the hardware independent IEEE 802.15.4
  networking stack for SoftMAC devices (the ones implementing
  only PHY level of IEEE 802.15.4 standard).

  Note: this implementation is neither certified, nor feature
  complete! We do not guarantee that it is compatible w/ other
  implementations, etc.
endef

$(eval $(call KernelPackage,mac802154))

define KernelPackage/fakehard
  SUBMENU:=$(WPAN_MENU)
  TITLE:=Fake LR-WPAN driver
  KCONFIG:=CONFIG_IEEE802154_FAKEHARD
  DEPENDS:=+kmod-ieee802154
  FILES:=$(LINUX_DIR)/drivers/ieee802154/fakehard.ko
  AUTOLOAD:=$(call AutoLoad,92,fakehard)
endef

define KernelPackage/fakehard/description
  Say Y here to enable the fake driver that serves as an example
  of HardMAC device driver.
endef

$(eval $(call KernelPackage,fakehard))

define KernelPackage/fakelb
  SUBMENU:=$(WPAN_MENU)
  TITLE:=Fake LR-WPAN driver
  KCONFIG:=CONFIG_IEEE802154_FAKELB
  DEPENDS:=+kmod-mac802154
  FILES:=$(LINUX_DIR)/drivers/ieee802154/fakelb.ko
  AUTOLOAD:=$(call AutoLoad,92,fakelb)
endef

define KernelPackage/fakehard/description
  Say Y here to enable the fake driver that can emulate a net
  of several interconnected radio devices.
endef

$(eval $(call KernelPackage,fakelb))

define KernelPackage/at86rf230
  SUBMENU:=$(WPAN_MENU)
  TITLE:=AT86RF230 transceiver driver
  KCONFIG:=CONFIG_IEEE802154_AT86RF230 \
	CONFIG_SPI=y \
	CONFIG_SPI_MASTER=y
  DEPENDS:=+kmod-mac802154
  FILES:=$(LINUX_DIR)/drivers/ieee802154/at86rf230.ko
endef

$(eval $(call KernelPackage,at86rf230))

define KernelPackage/spi_atusb
  SUBMENU:=$(WPAN_MENU)
  TITLE:=ATUSB SPI interface
  KCONFIG:=CONFIG_SPI_ATUSB
  DEPENDS:=+kmod-at86rf230 +kmod-usb-core
  FILES:=$(LINUX_DIR)/drivers/ieee802154/spi_atusb.ko
  AUTOLOAD:=$(call AutoLoad,93,spi_atusb)
endef

define KernelPackage/fakehard/description
  SPI-over-USB driver for the ATUSB IEEE 802.15.4 board.
endef

$(eval $(call KernelPackage,spi_atusb))

define KernelPackage/spi_atben
  SUBMENU:=$(WPAN_MENU)
  TITLE:=ATBEN 8:10 SPI interface
  KCONFIG:=CONFIG_SPI_ATBEN
  DEPENDS:=+kmod-at86rf230 @TARGET_xburst
  FILES:=$(LINUX_DIR)/drivers/ieee802154/spi_atben.ko
endef

define KernelPackage/fakehard/description
  Bit-banging SPI driver for the 8:10 interface of the Ben NanoNote
  when equipped with an ATBEN board.
endef

$(eval $(call KernelPackage,spi_atben))

