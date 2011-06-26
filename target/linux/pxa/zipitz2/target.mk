BOARDNAME:=ZipIt Z2

define Target/Description
	Build firmware images for ZipIt Z2 PXA-based handeld messenger device
endef

FEATURES:=targz
DEFAULT_PACKAGES += kmod-libertas-spi


