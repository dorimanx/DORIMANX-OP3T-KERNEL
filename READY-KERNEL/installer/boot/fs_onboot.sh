#!/sbin/busybox sh

export PATH=${PATH}:/system/bin:/sbin

while [ "$(mount | grep dm-0 | wc -l)" -eq "0" ]; do
	sleep 3;
done;


