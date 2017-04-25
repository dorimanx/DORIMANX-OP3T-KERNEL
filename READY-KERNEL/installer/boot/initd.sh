#!/system/xbin/busybox sh

export PATH=${PATH}:/system/bin:/system/xbin

while [ "$(mount | grep dm-0 | wc -l)" -eq "0" ]; do
	sleep 3;
done;

if [ ! -e /data/init.d ]; then
	mkdir /data/init.d
fi;

chmod -R 777 /data/init.d/
logwrapper busybox run-parts /data/init.d/

