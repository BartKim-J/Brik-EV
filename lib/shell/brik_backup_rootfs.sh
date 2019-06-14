#!/bin/bash

VERSION_MAJOR=1
VERSION_MINOR=0
echo "Brik Backup roofts Script ( $VERSION_MAJOR.$VERSION_MINOR )"

SRC=/
DST=/media/BRIK_USB/Workspace/Brik/image/backup
DATE="$(date +%m%d)"
TIME="$(date +%H%M%S)"

sudo mkdir -p ${DST}

echo "Backup Start. $(date +%H%M%S)"
sudo dd if=/dev/mmcblk1p5 of=${DST}/rootfs_${DATE}_${TIME}.img bs=512 count=7634910 | pv -s 2G
echo "Backup Done. $(date +%H%M%S)"

