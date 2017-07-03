#!/sbin/busybox sh

export PATH=${PATH}:/system/bin:/sbin

while [ "$(mount | grep dm-0 | wc -l)" -eq "0" ]; do
	sleep 3;
done;

if [ ! -e /data/init.d ]; then
	mkdir /data/init.d
fi;

chmod 0777 /data/init.d/*
exec /sbin/busybox run-parts /data/init.d/

