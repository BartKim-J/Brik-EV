#!/bin/bash

VERSION_MAJOR=1
VERSION_MINOR=0
echo "Brik Back Script ( $VERSION_MAJOR.$VERSION_MINOR )"

SRC=/
DST=/media/BRIK_USB/Workspace/Brik/backup
DATE="$(date +%m%d)"
TIME="$(date +%H%M%S)"

sudo fdisk -l
read -p"dst disk : "  DST_DISK

echo ${DST_DISK}

echo "Mount USB Disk to /mnt directory."
sudo mount ${DST_DISK} /mnt

sudo mkdir -p ${DST}

echo "Backup Start. $(date +%H%M%S)"
sudo tar -cvpzf ${DST}/brik_${DATE}_${TIME}.tar.gz --exclude=/mnt --one-file-system ${SRC} 
echo "Backup Done. $(date +%H%M%S)"

