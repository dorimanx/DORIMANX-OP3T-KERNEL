#!/sbin/busybox sh

export PATH=${PATH}:/system/bin:/sbin

while [ "$(mount | grep dm-0 | wc -l)" -eq "0" ]; do
	sleep 5;
done;

# Enable force Fast Charge
echo 1 > /sys/kernel/fast_charge/force_fast_charge;

# Make Sure touch boost is enabled.
echo 1 > /sys/module/cpu_boost/parameters/input_boost_enabled;
echo 40 > /sys/module/cpu_boost/parameters/input_boost_ms;
project=`getprop ro.boot.project_name`
case "$project" in
	"15811")
		# input boost configuration
		echo "0:1286400 2:1363200" > /sys/module/cpu_boost/parameters/input_boost_freq
	;;
esac

case "$project" in
	"15801")
		# input boost configuration
		echo "0:1363200 2:1363200" > /sys/module/cpu_boost/parameters/input_boost_freq
	;;
esac

# Set lowmemkiller settings
echo 1 > /sys/module/lowmemorykiller/parameters/enable_adaptive_lmk
echo "18432,23040,27648,51256,150296,200640" > /sys/module/lowmemorykiller/parameters/minfree
echo 202640 > /sys/module/lowmemorykiller/parameters/vmpressure_file_min

# Set sched core settings
echo 0 > /proc/sys/kernel/sched_boost
echo 45 > /proc/sys/kernel/sched_downmigrate
echo 45 > /proc/sys/kernel/sched_upmigrate
echo 400000 > /proc/sys/kernel/sched_freq_inc_notify
echo 400000 > /proc/sys/kernel/sched_freq_dec_notify
echo 3 > /proc/sys/kernel/sched_spill_nr_run
echo 100 > /proc/sys/kernel/sched_init_task_load

# Set DEVFREQ core
for cpubw in /sys/class/devfreq/*qcom,cpubw*; do
	echo "bw_hwmon" > $cpubw/governor
	echo 1525 > $cpubw/min_freq
	echo "1525 5195 11863 13763" > $cpubw/bw_hwmon/mbps_zones
	echo 4 > $cpubw/bw_hwmon/sample_ms
	echo 34 > $cpubw/bw_hwmon/io_percent
	echo 20 > $cpubw/bw_hwmon/hist_memory
	echo 10 > $cpubw/bw_hwmon/hyst_length
	echo 0 > $cpubw/bw_hwmon/low_power_ceil_mbps
	echo 34 > $cpubw/bw_hwmon/low_power_io_percent
	echo 20 > $cpubw/bw_hwmon/low_power_delay
	echo 0 > $cpubw/bw_hwmon/guard_band_mbps
	echo 250 > $cpubw/bw_hwmon/up_scale
	echo 1600 > $cpubw/bw_hwmon/idle_mbps
done

for memlat in /sys/class/devfreq/*qcom,memlat-cpu*; do
	echo "mem_latency" > $memlat/governor
	echo 10 > $memlat/polling_interval
done
echo "cpufreq" > /sys/class/devfreq/soc:qcom,mincpubw/governor

chmod 644 /sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq
chmod 644 /sys/devices/system/cpu/cpu2/cpufreq/scaling_min_freq
echo 307200 > /sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq
echo 307200 > /sys/devices/system/cpu/cpu2/cpufreq/scaling_min_freq

# Set default CPU0+1 GOV to interactive and tune it.
chmod 644 /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
echo "interactive" > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
chmod 644 /sys/devices/system/cpu/cpu0/cpufreq/interactive/*
chmod 200 /sys/devices/system/cpu/cpu0/cpufreq/interactive/boostpulse
echo 1 > /sys/devices/system/cpu/cpu0/cpufreq/interactive/use_sched_load
echo 1 > /sys/devices/system/cpu/cpu0/cpufreq/interactive/use_migration_notif
echo 10000 > /sys/devices/system/cpu/cpu0/cpufreq/interactive/above_hispeed_delay
echo 80 > /sys/devices/system/cpu/cpu0/cpufreq/interactive/go_hispeed_load
echo 20000 > /sys/devices/system/cpu/cpu0/cpufreq/interactive/timer_rate
echo 1286400 > /sys/devices/system/cpu/cpu0/cpufreq/interactive/hispeed_freq
echo 99000 > /sys/devices/system/cpu/cpu0/cpufreq/interactive/max_freq_hysteresis
echo 0 > /sys/devices/system/cpu/cpu0/cpufreq/interactive/io_is_busy
echo 80 > /sys/devices/system/cpu/cpu0/cpufreq/interactive/target_loads
echo 80000 > /sys/devices/system/cpu/cpu0/cpufreq/interactive/min_sample_time
echo 80000 > /sys/devices/system/cpu/cpu0/cpufreq/interactive/timer_slack
echo 1 > /sys/devices/system/cpu/cpu0/cpufreq/interactive/ignore_hispeed_on_notif

# Set default CPU2+3 GOV to interactive and tune it.
chmod 644 /sys/devices/system/cpu/cpu2/cpufreq/scaling_governor
echo "interactive" > /sys/devices/system/cpu/cpu2/cpufreq/scaling_governor
chmod 644 /sys/devices/system/cpu/cpu2/cpufreq/interactive/*
chmod 200 /sys/devices/system/cpu/cpu2/cpufreq/interactive/boostpulse
echo 1 > /sys/devices/system/cpu/cpu2/cpufreq/interactive/use_sched_load
echo 1 > /sys/devices/system/cpu/cpu2/cpufreq/interactive/use_migration_notif
echo "20000 1440000:20000 1747200:20000 2150400:39000" > /sys/devices/system/cpu/cpu2/cpufreq/interactive/above_hispeed_delay
echo 80 > /sys/devices/system/cpu/cpu2/cpufreq/interactive/go_hispeed_load
echo 20000 > /sys/devices/system/cpu/cpu2/cpufreq/interactive/timer_rate
echo 1440000 > /sys/devices/system/cpu/cpu2/cpufreq/interactive/hispeed_freq
echo 99000 > /sys/devices/system/cpu/cpu2/cpufreq/interactive/max_freq_hysteresis
echo 0 > /sys/devices/system/cpu/cpu2/cpufreq/interactive/io_is_busy
echo "80 1516800:90 1824000:80 2150400:95" > /sys/devices/system/cpu/cpu2/cpufreq/interactive/target_loads
echo 80000 > /sys/devices/system/cpu/cpu2/cpufreq/interactive/min_sample_time
echo 80000 > /sys/devices/system/cpu/cpu2/cpufreq/interactive/timer_slack
echo 1 > /sys/devices/system/cpu/cpu2/cpufreq/interactive/ignore_hispeed_on_notif


TIME_NOW=$(date)
echo "$TIME_NOW" > /data/dori_boot.txt

INITD_MODE=$(cat /data/initd_mode)
if [ "$INITD_MODE" -eq "1" ]; then
	if [ -e /data/init.d ]; then
		/sbin/busybox run-parts /data/init.d/ > /dev/null 2>&1
	fi
	echo "initd finished" > /data/initd_status;
fi

# Restore selinux
echo "1" > /sys/fs/selinux/enforce

