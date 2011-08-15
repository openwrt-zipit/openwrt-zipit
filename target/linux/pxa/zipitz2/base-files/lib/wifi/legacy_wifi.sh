#!/bin/sh
append DRIVERS "legacy"

# "legacy" is for old/incomplete drivers that don't do mac80211 properly
# (ie libertas_spi)

scan_legacy() {
	local device="$1"
	local adhoc sta ap monitor mesh

	config_get vifs "$device" vifs
	for vif in $vifs; do
		config_get mode "$vif" mode
		case "$mode" in
			adhoc|sta|ap|monitor|mesh)
				append $mode "$vif"
			;;
			*) echo "$device($vif): Invalid mode, ignored."; continue;;
		esac
	done

	config_set "$device" vifs "${ap:+$ap }${adhoc:+$adhoc }${sta:+$sta }${monitor:+$monitor }${mesh:+$mesh}"
}

disable_legacy() {
	local device="$1"

	config_get wdev "$device" interface
	set_wifi_down "$device"

	include /lib/network
	[ -f "/var/run/$wdev.pid" ] && kill $(cat /var/run/$wdev.pid) >&/dev/null 2>&1
	for pid in `pidof wpa_supplicant`; do
		 grep "$wdev" /proc/$pid/cmdline >/dev/null && \
			  kill $pid
	done
	ifconfig "$wdev" down 2>/dev/null
	unbridge "$dev"

	return 0
}

enable_legacy() {
	local device="$1"
	config_get channel "$device" channel
	config_get txpower "$device" txpower
	config_get country "$device" country

	config_get vifs "$device" vifs

	local i=0
	local macidx=0
	local apidx=0
	fixed=""

	[ -n "$country" ] && iw reg set "$country"
	[ "$channel" = "auto" -o "$channel" = "0" ] || {
		fixed=1
	}

	wifi_fixup_hwmode "$device" "g"

	for vif in $vifs; do
	    echo "VIF $vif"
	
	    while [ -d "/sys/class/net/wlan$i" ]; do
		i=$(($i + 1))
	    done

	    config_get ifname "$vif" ifname
	    [ -n "$ifname" ] || {
		ifname="wlan$i"
	    }
	    config_set "$vif" ifname "$ifname"

	    config_get mode "$vif" mode
	    config_get ssid "$vif" ssid

	    [ "$mode" = "sta" ] || {
		 echo "Legacy hardware only supports sta mode, not mode $mode"
		 return 1
	} 
	local wdsflag
	config_get_bool wds "$device" wds 0
	[ "$wds" -gt 0 ] && wdsflag="4addr on"
	iw phy "$phy" interface add "$ifname" type managed $wdsflag
	config_get_bool powersave "$device" powersave 0
	[ "$powersave" -gt 0 ] && powersave="on" || powersave="off"
	iwconfig "$ifname" power "$powersave"
	
	# All interfaces must have unique mac addresses
	# which can either be explicitly set in the device
	# section, or automatically generated
	config_get macaddr "$device" macaddr
	local mac_1="${macaddr%%:*}"
	local mac_2="${macaddr#*:}"
	
	    config_get vif_mac "$vif" macaddr
	    [ -n "$vif_mac" ] || {
	    if [ "$macidx" -gt 0 ]; then
		offset="$(( 2 + $macidx * 4 ))"
	    else
		offset="0"
	    fi
		 vif_mac="$( printf %02x $((0x$mac_1 + $offset)) ):$mac_2"
		 macidx="$(($macidx + 1))"
	}
	    ifconfig "$ifname" hw ether "$vif_mac"
	    config_set "$vif" macaddr "$vif_mac"
	
	    config_get vif_txpower "$vif" txpower
	# use vif_txpower (from wifi-iface) to override txpower (from
	# wifi-device) if the latter doesn't exist
	    txpower="${txpower:-$vif_txpower}"
	[ -z "$txpower" ] || iw dev "$ifname" set txpower fixed "${txpower%%.*}00"
	
	if eval "type wpa_supplicant_setup_vif" 2>/dev/null >/dev/null; then
		wpa_supplicant_setup_vif "$vif" nl80211 "${hostapd_ctrl:+-H $hostapd_ctrl}" || {
		    echo "enable_legacy($vif): Failed to set up wpa_supplicant for interface $ifname" >&2
	        # make sure this wifi interface won't accidentally stay open without encryption
		ifconfig "$ifname" down
		    continue
	    }
	fi		
	done
}