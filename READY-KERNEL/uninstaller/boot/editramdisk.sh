#!/sbin/sh

mkdir /tmp/ramdisk
cp /tmp/boot.img-ramdisk.gz /tmp/ramdisk/
cd /tmp/ramdisk/
gunzip -c /tmp/ramdisk/boot.img-ramdisk.gz | cpio -i
rm /tmp/ramdisk/boot.img-ramdisk.gz
rm /tmp/boot.img-ramdisk.gz

# remove dorimanx script
if [ $(grep -c "import /init.dorimanx.rc" /tmp/ramdisk/init.rc) == 1 ]; then
	sed -i "/import \/init.dorimanx.rc/d" /tmp/ramdisk/init.rc
fi

# Remove my modules from ramdisk
if [ -e /tmp/ramdisk/dori_modules ]; then
	rm -rf /tmp/ramdisk/dori_modules
fi

if [ -e /tmp/ramdisk/fs_onboot.sh ]; then
	rm /tmp/ramdisk/fs_onboot.sh
	if [ -e /tmp/ramdisk/fs_gov.sh ]; then
		rm /tmp/ramdisk/fs_gov.sh
	fi
	if [ -e /tmp/ramdisk/dori_sec.info ]; then
		rm /tmp/ramdisk/dori_sec.info
	fi
	if [ -e /tmp/ramdisk/initd.sh ]; then
		rm /tmp/ramdisk/initd.sh
	fi
fi

if [ -e /tmp/ramdisk/sbin/busybox ]; then
	rm /tmp/ramdisk/sbin/busybox
	if [ -e /tmp/ramdisk/sbin/mount ]; then
		rm -f /tmp/ramdisk/sbin/mount
	fi
fi

if [ -e /tmp/ramdisk/init.dorimanx.rc ]; then
	rm /tmp/ramdisk/init.dorimanx.rc
fi

# pack ramdisk
find . | cpio -o -H newc | gzip > /tmp/boot.img-ramdisk.gz
rm -r /tmp/ramdisk

