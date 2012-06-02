BOARDNAME:=ZipIt Z2

define Target/Description
	Build firmware images for ZipIt Z2 PXA-based handeld messenger device
endef

FEATURES:=targz usb usbgadget
# packages we want for functional gui, wifi, sound, udc support
DEFAULT_PACKAGES += gmenu2x kmod-libertas-spi kmod-sound-zipit-z2 kmod-pxa27x-udc hostapd wpa-supplicant

# default packages we don't probably want (router stuff like, NAT, PPP, DHCP/DNS servers, etc.)
DEFAULT_PACKAGES += -iptables-mod-conntrack -iptables-mod-nat -ppp -ppp-mod-pppoe -libip4tc -libxtables -kmod-ipt-conntrack -kmod-ipt-nat -kmod-ipt-nathelper -kmod-ppp -kmod-pppoe -dnsmasq

# Z2 console support
DEPENDS := +@BUSYBOX_CONFIG_LOADFONT +@BUSYBOX_CONFIG_LOADKMAP +@BUSYBOX_CONFIG_FEATURE_LOADFONT_PSF2
