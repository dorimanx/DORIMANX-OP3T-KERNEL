#!/sbin/sh

# Script originaly created by flar2@github.com
# https://github.com/flar2/android_kernel_oneplus_msm8996

#Build config file
CONFIGFILE="/tmp/init.dorimanx.rc"
FS_DATA_GOV="/tmp/fs_gov.sh"
BACKUP="/sdcard/.dorimanx.backup"

cd /tmp/dori_modules/qca_cld
ln -s /system/lib/modules/wlan.ko qca_cld_wlan.ko
cd /

echo "on boot" >> $CONFIGFILE
# Mount my kernel modules as expected.
echo "rmmod wlan.ko" >> $CONFIGFILE
echo "mount none /dori_modules /system/lib/modules/ bind" >> $CONFIGFILE
echo "" >> $CONFIGFILE

# fsync
FSYNC=`grep "item.0.1" /tmp/aroma/mods.prop | cut -d '=' -f2`
if [ $FSYNC = 1 ]; then
  echo "write /sys/module/sync/parameters/fsync_enabled 0" >> $CONFIGFILE
fi

# backlight dimmer
BLDIM=`grep "item.0.2" /tmp/aroma/mods.prop | cut -d '=' -f2`
if [ $BLDIM = 1 ]; then
  echo "write /sys/module/mdss_fb/parameters/backlight_dimmer 1" >> $CONFIGFILE
fi

# Tocuh Leds delay
TLCTL=`grep "item.0.3" /tmp/aroma/mods.prop | cut -d '=' -f2`
if [ $TLCTL = 1 ]; then
  echo "write /sys/class/misc/btk_control/btkc_mode 1" >> $CONFIGFILE
else
  echo "write /sys/class/misc/btk_control/btkc_mode 0" >> $CONFIGFILE
fi

TLTIME1=`grep item.0.4 /tmp/aroma/mods.prop | cut -d '=' -f2`
TLTIME2=`grep item.0.5 /tmp/aroma/mods.prop | cut -d '=' -f2`
TLTIME3=`grep item.0.6 /tmp/aroma/mods.prop | cut -d '=' -f2`
if [ $TLTIME1 = 1 ]; then
  echo "write /sys/class/misc/btk_control/btkc_timeout 2000" >> $CONFIGFILE
fi
if [ $TLTIME2 = 1 ]; then
  echo "write /sys/class/misc/btk_control/btkc_timeout 5000" >> $CONFIGFILE
fi
if [ $TLTIME3 = 1 ]; then
  echo "write /sys/class/misc/btk_control/btkc_timeout 7000" >> $CONFIGFILE
fi

# S2W
SR=`grep "item.1.1" /tmp/aroma/gest.prop | cut -d '=' -f2`
SL=`grep "item.1.2" /tmp/aroma/gest.prop | cut -d '=' -f2`
SU=`grep "item.1.3" /tmp/aroma/gest.prop | cut -d '=' -f2`
SD=`grep "item.1.4" /tmp/aroma/gest.prop | cut -d '=' -f2`

if [ $SL = 1 ]; then
  SL=2
fi
if [ $SU == 1 ]; then
  SU=4
fi
if [ $SD == 1 ]; then
  SD=8
fi  

S2W=$(( SL + SR + SU + SD ))
echo "write /sys/android_touch/sweep2wake " $S2W >> $CONFIGFILE

# DT2W
DT2W=`grep "item.1.5" /tmp/aroma/gest.prop | cut -d '=' -f2`
echo "write /sys/android_touch/doubletap2wake " $DT2W >> $CONFIGFILE


# S2S
S2S=`grep selected.0 /tmp/aroma/s2s.prop | cut -d '=' -f2`
if [ $S2S = 2 ]; then
  echo "write /sys/sweep2sleep/sweep2sleep 1" >> $CONFIGFILE
elif [ $S2S = 3 ]; then
  echo "write /sys/sweep2sleep/sweep2sleep 2" >> $CONFIGFILE
elif [ $S2S = 4 ]; then
  echo "write /sys/sweep2sleep/sweep2sleep 3" >> $CONFIGFILE
else
  echo "write /sys/sweep2sleep/sweep2sleep 0" >> $CONFIGFILE
fi


# Wakelocks
WAKE1=`grep "item.0.1" /tmp/aroma/wakes.prop | cut -d '=' -f2`
WAKE2=`grep "item.0.2" /tmp/aroma/wakes.prop | cut -d '=' -f2`
WAKE3=`grep "item.0.3" /tmp/aroma/wakes.prop | cut -d '=' -f2`
WAKE4=`grep "item.0.4" /tmp/aroma/wakes.prop | cut -d '=' -f2`
WAKE5=`grep "item.0.5" /tmp/aroma/wakes.prop | cut -d '=' -f2`
WAKE6=`grep "item.0.6" /tmp/aroma/wakes.prop | cut -d '=' -f2`

if [ $WAKE1 = 1 ]; then
  echo "write /sys/module/wakeup/parameters/enable_ipa_ws N" >> $CONFIGFILE
fi
if [ $WAKE2 = 1 ]; then
  echo "write /sys/module/wakeup/parameters/enable_qcom_rx_wakelock_ws N" >> $CONFIGFILE
fi
if [ $WAKE3 = 1 ]; then
  echo "write /sys/module/wakeup/parameters/enable_wlan_extscan_wl_ws N" >> $CONFIGFILE
fi
if [ $WAKE4 = 1 ]; then
  echo "write /sys/module/wakeup/parameters/enable_wlan_ws N" >> $CONFIGFILE
fi
if [ $WAKE5 = 1 ]; then
  echo "write /sys/module/wakeup/parameters/enable_timerfd_ws N" >> $CONFIGFILE
fi
if [ $WAKE6 = 1 ]; then
  echo "write /sys/module/wakeup/parameters/enable_netlink_ws N" >> $CONFIGFILE
fi

echo "" >> $CONFIGFILE
echo "on post-fs-data" >> $CONFIGFILE
echo "" >> $CONFIGFILE

# i/o scheduler
SCHED=`grep selected.1 /tmp/aroma/disk.prop | cut -d '=' -f2`
if [ $SCHED = 1 ]; then
  echo "cfq" > $FS_DATA_GOV
  echo "write /sys/block/sda/queue/scheduler cfq"  >> $CONFIGFILE
  echo "write /sys/block/sdb/queue/scheduler cfq"  >> $CONFIGFILE
  echo "write /sys/block/sdc/queue/scheduler cfq"  >> $CONFIGFILE
  echo "write /sys/block/sdd/queue/scheduler cfq"  >> $CONFIGFILE
  echo "write /sys/block/sde/queue/scheduler cfq"  >> $CONFIGFILE
  echo "write /sys/block/sdf/queue/scheduler cfq"  >> $CONFIGFILE
elif [ $SCHED = 2 ]; then
  echo "deadline" > $FS_DATA_GOV
  echo "write /sys/block/sda/queue/scheduler deadline"  >> $CONFIGFILE
  echo "write /sys/block/sdb/queue/scheduler deadline"  >> $CONFIGFILE
  echo "write /sys/block/sdc/queue/scheduler deadline"  >> $CONFIGFILE
  echo "write /sys/block/sdd/queue/scheduler deadline"  >> $CONFIGFILE
  echo "write /sys/block/sde/queue/scheduler deadline"  >> $CONFIGFILE
  echo "write /sys/block/sdf/queue/scheduler deadline"  >> $CONFIGFILE
elif [ $SCHED = 3 ]; then
  echo "fiops" > $FS_DATA_GOV
  echo "write /sys/block/sda/queue/scheduler fiops"  >> $CONFIGFILE
  echo "write /sys/block/sdb/queue/scheduler fiops"  >> $CONFIGFILE
  echo "write /sys/block/sdc/queue/scheduler fiops"  >> $CONFIGFILE
  echo "write /sys/block/sdd/queue/scheduler fiops"  >> $CONFIGFILE
  echo "write /sys/block/sde/queue/scheduler fiops"  >> $CONFIGFILE
  echo "write /sys/block/sdf/queue/scheduler fiops"  >> $CONFIGFILE
elif [ $SCHED = 4 ]; then
  echo "sio" > $FS_DATA_GOV
  echo "write /sys/block/sda/queue/scheduler sio"  >> $CONFIGFILE
  echo "write /sys/block/sdb/queue/scheduler sio"  >> $CONFIGFILE
  echo "write /sys/block/sdc/queue/scheduler sio"  >> $CONFIGFILE
  echo "write /sys/block/sdd/queue/scheduler sio"  >> $CONFIGFILE
  echo "write /sys/block/sde/queue/scheduler sio"  >> $CONFIGFILE
  echo "write /sys/block/sdf/queue/scheduler sio"  >> $CONFIGFILE
elif [ $SCHED = 5 ]; then
  echo "bfq" > $FS_DATA_GOV
  echo "write /sys/block/sda/queue/scheduler bfq"  >> $CONFIGFILE
  echo "write /sys/block/sdb/queue/scheduler bfq"  >> $CONFIGFILE
  echo "write /sys/block/sdc/queue/scheduler bfq"  >> $CONFIGFILE
  echo "write /sys/block/sdd/queue/scheduler bfq"  >> $CONFIGFILE
  echo "write /sys/block/sde/queue/scheduler bfq"  >> $CONFIGFILE
  echo "write /sys/block/sdf/queue/scheduler bfq"  >> $CONFIGFILE
elif [ $SCHED = 6 ]; then
  echo "noop" > $FS_DATA_GOV
  echo "write /sys/block/sda/queue/scheduler noop"  >> $CONFIGFILE
  echo "write /sys/block/sdb/queue/scheduler noop"  >> $CONFIGFILE
  echo "write /sys/block/sdc/queue/scheduler noop"  >> $CONFIGFILE
  echo "write /sys/block/sdd/queue/scheduler noop"  >> $CONFIGFILE
  echo "write /sys/block/sde/queue/scheduler noop"  >> $CONFIGFILE
  echo "write /sys/block/sdf/queue/scheduler noop"  >> $CONFIGFILE
elif [ $SCHED = 7 ]; then
  echo "zen" > $FS_DATA_GOV
  echo "write /sys/block/sda/queue/scheduler zen"  >> $CONFIGFILE
  echo "write /sys/block/sdb/queue/scheduler zen"  >> $CONFIGFILE
  echo "write /sys/block/sdc/queue/scheduler zen"  >> $CONFIGFILE
  echo "write /sys/block/sdd/queue/scheduler zen"  >> $CONFIGFILE
  echo "write /sys/block/sde/queue/scheduler zen"  >> $CONFIGFILE
  echo "write /sys/block/sdf/queue/scheduler zen"  >> $CONFIGFILE
fi

# set readahead to 128
# With SSD, there are no mechanical rotational latency issues so the SSD storage uses a small 4k read-ahead. but i will set 128 for now.
echo "write /sys/block/sda/queue/read_ahead_kb 128" >> $CONFIGFILE
echo "write /sys/block/sdb/queue/read_ahead_kb 128" >> $CONFIGFILE
echo "write /sys/block/sdc/queue/read_ahead_kb 128" >> $CONFIGFILE
echo "write /sys/block/sdd/queue/read_ahead_kb 128" >> $CONFIGFILE
echo "write /sys/block/sde/queue/read_ahead_kb 128" >> $CONFIGFILE
echo "write /sys/block/sdf/queue/read_ahead_kb 128" >> $CONFIGFILE

# charge control
CHARGE=`grep selected.1 /tmp/aroma/charge.prop | cut -d '=' -f2`
if [ $CHARGE = 1 ]; then
  echo "write /sys/module/qpnp_smbcharger/parameters/default_hvdcp_icl_ma 1800" >> $CONFIGFILE
  echo "write /sys/module/qpnp_smbcharger/parameters/default_dcp_icl_ma 1800" >> $CONFIGFILE
  echo "write /sys/module/dwc3_msm/parameters/dcp_max_current 1500" >> $CONFIGFILE
  echo "write /sys/module/dwc3_msm/parameters/hvdcp_max_current 1800" >> $CONFIGFILE
elif [ $CHARGE = 2 ]; then
  echo "write /sys/module/qpnp_smbcharger/parameters/default_hvdcp_icl_ma 1800" >> $CONFIGFILE
  echo "write /sys/module/qpnp_smbcharger/parameters/default_dcp_icl_ma 1800" >> $CONFIGFILE
  echo "write /sys/module/dwc3_msm/parameters/dcp_max_current 1800" >> $CONFIGFILE
  echo "write /sys/module/dwc3_msm/parameters/hvdcp_max_current 1800" >> $CONFIGFILE
elif [ $CHARGE = 3 ]; then
  echo "write /sys/module/qpnp_smbcharger/parameters/default_hvdcp_icl_ma 2000" >> $CONFIGFILE
  echo "write /sys/module/qpnp_smbcharger/parameters/default_dcp_icl_ma 2000" >> $CONFIGFILE
  echo "write /sys/module/dwc3_msm/parameters/dcp_max_current 2000" >> $CONFIGFILE
  echo "write /sys/module/dwc3_msm/parameters/hvdcp_max_current 2000" >> $CONFIGFILE
elif [ $CHARGE = 4 ]; then
  echo "write /sys/module/qpnp_smbcharger/parameters/default_hvdcp_icl_ma 2500" >> $CONFIGFILE
  echo "write /sys/module/qpnp_smbcharger/parameters/default_dcp_icl_ma 2500" >> $CONFIGFILE
  echo "write /sys/module/dwc3_msm/parameters/dcp_max_current 2500" >> $CONFIGFILE
  echo "write /sys/module/dwc3_msm/parameters/hvdcp_max_current 2500" >> $CONFIGFILE
fi

echo "" >> $CONFIGFILE

echo "# launch dorimanx kernel boot script" >> $CONFIGFILE
echo "service doriscript /system/bin/sh /fs_onboot.sh" >> $CONFIGFILE
echo "    class late_start" >> $CONFIGFILE
echo "    user root" >> $CONFIGFILE
echo "    group root" >> $CONFIGFILE
echo "    disabled" >> $CONFIGFILE
echo "    oneshot" >> $CONFIGFILE

echo "" >> $CONFIGFILE

# init.d support in /data/init.d/
INITD_SET=`grep item.0.7 /tmp/aroma/mods.prop | cut -d '=' -f2`
if [ $INITD_SET = 1 ]; then
	echo 1 > /data/initd_mode
	if [ ! -e /data/init.d ]; then
		mkdir /data/init.d
	fi
	chmod 0777 /data/init.d/*
else
	echo 0 > /data/initd_mode
fi
chmod 777 /data/initd_mode
echo "" >> $CONFIGFILE

echo "on property:sys.boot_completed=1" >> $CONFIGFILE
echo "    write /sys/fs/selinux/enforce 5" >> $CONFIGFILE
echo "    start doriscript" >> $CONFIGFILE

echo "" >> $CONFIGFILE

# reinstall options
echo -e "##### Reinstall Options #####" > $BACKUP
echo -e "# These settings are only applied if you run the express installer" >> $BACKUP
