#!/sbin/sh

mkdir /tmp/ramdisk
cp /tmp/boot.img-ramdisk.gz /tmp/ramdisk/
cd /tmp/ramdisk/
gunzip -c /tmp/ramdisk/boot.img-ramdisk.gz | cpio -i
rm /tmp/ramdisk/boot.img-ramdisk.gz
rm /tmp/boot.img-ramdisk.gz

# remove dorimanx script
if [ $(grep -c "import /init.dorimanx.rc" /tmp/ramdisk/init.rc) == 1 ]; then
	sed -i '/import \/init.dorimanx.rc/d" /tmp/ramdisk/init.rc
fi

# Remove my modules from ramdisk
if [ -e /tmp/ramdisk/modules.img ]; then
	rm /tmp/ramdisk/modules.img
fi;

# remove mount modules image and reload modules
# first cleanup init.qcom.rc
if [ $(grep -c "rmmod wlan.ko" /tmp/ramdisk/init.qcom.rc) == 1 ]; then
	sed -i '/rmmod wlan.ko/d' /tmp/ramdisk/init.qcom.rc
fi
if [ $(grep -c "rmmod crpl.ko" /tmp/ramdisk/init.qcom.rc) == 1 ]; then
	sed -i '/rmmod crpl.ko/d' /tmp/ramdisk/init.qcom.rc
fi
if [ $(grep -c "modules.img" /tmp/ramdisk/init.qcom.rc) == 1 ]; then
	sed -i '/mount ext4 loop\@\/modules\.img \/system\/lib\/modules noatime ro/d' /tmp/ramdisk/init.qcom.rc
fi
if [ $(grep -c "insmod /system/lib/modules/crpl.ko" /tmp/ramdisk/init.qcom.rc) == 1 ]; then
	sed -i '/insmod \/system\/lib\/modules\/crpl\.ko/d' /tmp/ramdisk/init.qcom.rc
fi
if [ $(grep -c "insmod /system/lib/modules/wlan.ko" /tmp/ramdisk/init.qcom.rc) == 1 ]; then
	sed -i '/insmod \/system\/lib\/modules\/wlan\.ko/d' /tmp/ramdisk/init.qcom.rc
fi

# pack ramdisk
find . | cpio -o -H newc | gzip > /tmp/boot.img-ramdisk.gz
rm -r /tmp/ramdisk
