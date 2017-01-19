#!/sbin/sh

# Script originaly created by flar2@github.com
# https://github.com/flar2/android_kernel_oneplus_msm8996

#Build config file
CONFIGFILE="/tmp/init.dorimanx.rc"
BACKUP="/sdcard/.dorimanx.backup"

echo "on boot" >> $CONFIGFILE
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

echo "" >> $CONFIGFILE
echo "on property:sys.boot_completed=1" >> $CONFIGFILE
echo "" >> $CONFIGFILE

# i/o scheduler
SCHED=`grep selected.1 /tmp/aroma/disk.prop | cut -d '=' -f2`
if [ $SCHED = 1 ]; then
  echo "write /sys/block/dm-0/queue/scheduler cfq"  >> $CONFIGFILE
  echo "write /sys/block/sda/queue/scheduler cfq"  >> $CONFIGFILE
elif [ $SCHED = 2 ]; then
  echo "write /sys/block/dm-0/queue/scheduler deadline"  >> $CONFIGFILE
  echo "write /sys/block/sda/queue/scheduler deadline"  >> $CONFIGFILE
elif [ $SCHED = 3 ]; then
  echo "write /sys/block/dm-0/queue/scheduler fiops"  >> $CONFIGFILE
  echo "write /sys/block/sda/queue/scheduler fiops"  >> $CONFIGFILE
elif [ $SCHED = 4 ]; then
  echo "write /sys/block/dm-0/queue/scheduler sio"  >> $CONFIGFILE
  echo "write /sys/block/sda/queue/scheduler sio"  >> $CONFIGFILE
elif [ $SCHED = 5 ]; then
  echo "write /sys/block/dm-0/queue/scheduler bfq"  >> $CONFIGFILE
  echo "write /sys/block/sda/queue/scheduler bfq"  >> $CONFIGFILE
elif [ $SCHED = 6 ]; then
  echo "write /sys/block/dm-0/queue/scheduler noop"  >> $CONFIGFILE
  echo "write /sys/block/sda/queue/scheduler noop"  >> $CONFIGFILE
elif [ $SCHED = 7 ]; then
  echo "write /sys/block/dm-0/queue/scheduler zen"  >> $CONFIGFILE
  echo "write /sys/block/sda/queue/scheduler zen"  >> $CONFIGFILE
fi

# set readahead to 128
echo "write /sys/block/sda/queue/read_ahead_kb 128" >> $CONFIGFILE

# reinstall options
echo -e "##### Reinstall Options #####" > $BACKUP
echo -e "# These settings are only applied if you run the express installer" >> $BACKUP
