#!/bin/bash

# >> Development Mode
sudo ifconfig wlan0 up
/sbin/wpa_supplicant -B -i wlan0 -c /etc/wpa_supplicant/wpa_supplicant.conf
sleep 3
/sbin/dhclient

# >> Release Mode
#/usr/sbin/hostapd -B /etc/hostapd/hostapd.conf 
#sleep 1

