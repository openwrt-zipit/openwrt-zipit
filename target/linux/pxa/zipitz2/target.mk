BOARDNAME:=ZipIt Z2

define Target/Description
	Build firmware images for ZipIt Z2 PXA-based handeld messenger device
endef

FEATURES:=targz
DEFAULT_PACKAGES += kmod-libertas-spi hostapd wpa-supplicant
DEPENDS := +@BUSYBOX_CONFIG_LOADFONT +@BUSYBOX_CONFIG_LOADKMAP +@BUSYBOX_CONFIG_FEATURE_LOADFONT_PSF2



