/* drivers/cpufreq/qcom-cpufreq.c
 *
 * MSM architecture cpufreq driver
 *
 * Copyright (C) 2007 Google, Inc.
 * Copyright (c) 2007-2015, The Linux Foundation. All rights reserved.
 * Author: Mike A. Chan <mikechan@google.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/cpufreq.h>
#include <linux/cpu.h>
#include <linux/cpumask.h>
#include <linux/suspend.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <trace/events/power.h>
#include <soc/qcom/socinfo.h>

// AP: Default startup frequencies
#define CONFIG_CPU_FREQ_MIN_CLUSTER1	307200
#define CONFIG_CPU_FREQ_MAX_CLUSTER1	1593600
#define CONFIG_CPU_FREQ_MIN_CLUSTER2	307200
#define CONFIG_CPU_FREQ_MAX_CLUSTER2	2150400
#define CONFIG_CPU_FREQ_MAX_CLUSTER2PRO	2342400

#define LITTLE_CPU_NUM 0
#define BIG_CPU_NUM 2
static DEFINE_MUTEX(l2bw_lock);

static struct clk *cpu_clk[NR_CPUS];
static struct clk *l2_clk;
static DEFINE_PER_CPU(struct cpufreq_frequency_table *, freq_table);
static bool hotplug_ready;

static unsigned int max_two_freqs[NR_CPUS][2];

struct cpufreq_suspend_t {
	struct mutex suspend_mutex;
	int device_suspended;
};

static DEFINE_PER_CPU(struct cpufreq_suspend_t, suspend_data);
#ifdef CONFIG_CPU_FREQ_LIMIT_BOOT_CURRENT
unsigned int cluster1_first_cpu = 0;
#endif

#ifdef CONFIG_QCOM_CPUFREQ_LIMITER
static unsigned int upper_limit_freq_pro[NR_CPUS] = {0, 0, 0, 0};
static unsigned int upper_limit_freq[NR_CPUS] = {0, 0, 0, 0};
static unsigned int lower_limit_freq[NR_CPUS] = {0, 0, 0, 0};
#define CPU_MAX_OC_FREQ_PRO_BC	2419200
#define CPU_MAX_OC_FREQ_PRO_LC	2188800
#define CPU_MAX_OC_FREQ_BC	2265600
#define CPU_MAX_OC_FREQ_LC	1728000
#define CPU_MIN_DEFAULT_FREQ	307200

unsigned int get_cpu_min_lock(unsigned int cpu)
{
	if (cpu >= 0 && cpu < NR_CPUS)
		return lower_limit_freq[cpu];
	else
		return 0;
}
EXPORT_SYMBOL(get_cpu_min_lock);

void set_cpu_min_lock(unsigned int cpu, int freq)
{
	if (cpu >= 0 && cpu < NR_CPUS) {
		/*
		 * OP3/T device has 2 cpu clusters,
		 * each 2 cores are linked with same freq and gov
		 * make sure they get set with same lock freq per cluster.
		 */
		if (socinfo_get_id() == 305) {
			if (cpu <= 1) {
				if (freq <= CPU_MIN_DEFAULT_FREQ ||
						freq > CPU_MAX_OC_FREQ_PRO_LC) {
					lower_limit_freq[0] = 0;
					lower_limit_freq[1] = 0;
				} else {
					lower_limit_freq[0] = freq;
					lower_limit_freq[1] = freq;
				}
			} else if (cpu >= 2) {
				if (freq <= CPU_MIN_DEFAULT_FREQ ||
						freq > CPU_MAX_OC_FREQ_PRO_BC) {
					lower_limit_freq[2] = 0;
					lower_limit_freq[3] = 0;
				} else {
					lower_limit_freq[2] = freq;
					lower_limit_freq[3] = freq;
				}
			}
		} else {
			if (cpu <= 1) {
				if (freq <= CPU_MIN_DEFAULT_FREQ ||
						freq > CPU_MAX_OC_FREQ_LC) {
					lower_limit_freq[0] = 0;
					lower_limit_freq[1] = 0;
				} else {
					lower_limit_freq[0] = freq;
					lower_limit_freq[1] = freq;
				}
			} else if (cpu >= 2) {
				if (freq <= CPU_MIN_DEFAULT_FREQ ||
						freq > CPU_MAX_OC_FREQ_BC) {
					lower_limit_freq[2] = 0;
					lower_limit_freq[3] = 0;
				} else {
					lower_limit_freq[2] = freq;
					lower_limit_freq[3] = freq;
				}
			}
		}
	}
}
EXPORT_SYMBOL(set_cpu_min_lock);

unsigned int get_cpu_max_lock(unsigned int cpu)
{
	if (cpu >= 0 && cpu < NR_CPUS) {
		if (socinfo_get_id() == 305)
			return upper_limit_freq_pro[cpu];
		else
			return upper_limit_freq[cpu];
	} else
		return 0;
}
EXPORT_SYMBOL(get_cpu_max_lock);

void set_cpu_max_lock(unsigned int cpu, unsigned int freq)
{
	if (cpu >= 0 && cpu <= NR_CPUS) {
		/*
		 * OP3/T device has 2 cpu clusters,
		 * each 2 cores are linked with same freq and gov
		 * make sure they get set with same lock freq per cluster.
		 */
		if (freq == 0) {
			if (socinfo_get_id() == 305) {
				if (cpu <= 1) {
					upper_limit_freq_pro[0] = 0;
					upper_limit_freq_pro[1] = 0;
				} else if (cpu >= 2) {
					upper_limit_freq_pro[2] = 0;
					upper_limit_freq_pro[3] = 0;
				}
			} else {
				if (cpu <= 1) {
					upper_limit_freq[0] = 0;
					upper_limit_freq[1] = 0;
				} else if (cpu >= 2) {
					upper_limit_freq[2] = 0;
					upper_limit_freq[3] = 0;
				}
			}
		} else if (socinfo_get_id() == 305) {
			if (cpu <= 1) {
				if (freq < CPU_MIN_DEFAULT_FREQ ||
						freq > CPU_MAX_OC_FREQ_PRO_LC) {
					upper_limit_freq_pro[0] = CONFIG_CPU_FREQ_MAX_CLUSTER1;
					upper_limit_freq_pro[1] = CONFIG_CPU_FREQ_MAX_CLUSTER1;
				} else {
					upper_limit_freq_pro[0] = freq;
					upper_limit_freq_pro[1] = freq;
				}
			} else if (cpu >= 2) {
				if (freq < CPU_MIN_DEFAULT_FREQ ||
						freq > CPU_MAX_OC_FREQ_PRO_BC) {
					upper_limit_freq_pro[2] = CONFIG_CPU_FREQ_MAX_CLUSTER2PRO;
					upper_limit_freq_pro[3] = CONFIG_CPU_FREQ_MAX_CLUSTER2PRO;
				} else {
					upper_limit_freq_pro[2] = freq;
					upper_limit_freq_pro[3] = freq;
				}
			}
		} else if (cpu <= 1) {
			if (freq < CPU_MIN_DEFAULT_FREQ || freq > CPU_MAX_OC_FREQ_LC) {
				upper_limit_freq[0] = CONFIG_CPU_FREQ_MAX_CLUSTER1;
				upper_limit_freq[1] = CONFIG_CPU_FREQ_MAX_CLUSTER1;
			} else {
				upper_limit_freq[0] = freq;
				upper_limit_freq[1] = freq;
			}
		} else if (cpu >= 2) {
			if (freq < CPU_MIN_DEFAULT_FREQ || freq > CPU_MAX_OC_FREQ_BC) {
				upper_limit_freq[2] = CONFIG_CPU_FREQ_MAX_CLUSTER2;
				upper_limit_freq[3] = CONFIG_CPU_FREQ_MAX_CLUSTER2;
			} else {
				upper_limit_freq[2] = freq;
				upper_limit_freq[3] = freq;
			}
		}
	}
}
EXPORT_SYMBOL(set_cpu_max_lock);
#endif

static int set_cpu_freq(struct cpufreq_policy *policy, unsigned int new_freq,
			unsigned int index)
{
	int ret = 0;
	struct cpufreq_freqs freqs;
	unsigned long rate;
#ifdef CONFIG_QCOM_CPUFREQ_LIMITER
	unsigned int ll_freq = lower_limit_freq[policy->cpu];
	unsigned int ul_freq_pro = upper_limit_freq_pro[policy->cpu];
	unsigned int ul_freq = upper_limit_freq[policy->cpu];

	if (ll_freq || ul_freq || ul_freq_pro) {
		unsigned int t_freq = new_freq;

		if (ll_freq && new_freq < ll_freq)
			t_freq = ll_freq;

		if (socinfo_get_id() == 305) {
			if (ul_freq_pro && new_freq > ul_freq_pro)
				t_freq = ul_freq_pro;
		} else {
			if (ul_freq && new_freq > ul_freq)
				t_freq = ul_freq;
		}

		new_freq = t_freq;

		if (new_freq < policy->min)
			new_freq = policy->min;
		if (new_freq > policy->max)
			new_freq = policy->max;
	}
#endif

	freqs.old = policy->cur;
	freqs.new = new_freq;
	freqs.cpu = policy->cpu;

	trace_cpu_frequency_switch_start(freqs.old, freqs.new, policy->cpu);
	cpufreq_freq_transition_begin(policy, &freqs);

	rate = new_freq * 1000;
	rate = clk_round_rate(cpu_clk[policy->cpu], rate);
	ret = clk_set_rate(cpu_clk[policy->cpu], rate);
	cpufreq_freq_transition_end(policy, &freqs, ret);
	if (!ret)
		trace_cpu_frequency_switch_end(policy->cpu);

	return ret;
}

static int msm_cpufreq_target(struct cpufreq_policy *policy,
				unsigned int target_freq,
				unsigned int relation)
{
	int ret = 0;
	int index;
	struct cpufreq_frequency_table *table;

	mutex_lock(&per_cpu(suspend_data, policy->cpu).suspend_mutex);

	if (target_freq == policy->cur)
		goto done;

	if (per_cpu(suspend_data, policy->cpu).device_suspended) {
		pr_debug("cpufreq: cpu%d scheduling frequency change "
				"in suspend.\n", policy->cpu);
		ret = -EFAULT;
		goto done;
	}

	table = cpufreq_frequency_get_table(policy->cpu);
	if (!table) {
		pr_err("cpufreq: Failed to get frequency table for CPU%u\n",
		       policy->cpu);
		ret = -ENODEV;
		goto done;
	}

	if (policy->cpu >= BIG_CPU_NUM) {
		target_freq = max((unsigned int)pm_qos_request(PM_QOS_BIG_CPU_FREQ_MIN), target_freq);
                target_freq = min((unsigned int)pm_qos_request(PM_QOS_BIG_CPU_FREQ_MAX), target_freq);
	 } else {
                target_freq = max((unsigned int)pm_qos_request(PM_QOS_LITTLE_CPU_FREQ_MIN), target_freq);
                target_freq = min((unsigned int)pm_qos_request(PM_QOS_LITTLE_CPU_FREQ_MAX), target_freq);
        }

	if (cpufreq_frequency_table_target(policy, table, target_freq, relation,
			&index)) {
		pr_err("cpufreq: invalid target_freq: %d\n", target_freq);
		ret = -EINVAL;
		goto done;
	}

	pr_debug("CPU[%d] target %d relation %d (%d-%d) selected %d\n",
		policy->cpu, target_freq, relation,
		policy->min, policy->max, table[index].frequency);

	ret = set_cpu_freq(policy, target_freq,
			   table[index].driver_data);

	/* save current frequency */
	policy->cur = target_freq;
done:
	mutex_unlock(&per_cpu(suspend_data, policy->cpu).suspend_mutex);
	return ret;
}

static int msm_cpufreq_verify(struct cpufreq_policy *policy)
{
	cpufreq_verify_within_limits(policy, policy->cpuinfo.min_freq,
			policy->cpuinfo.max_freq);
	return 0;
}

static unsigned int msm_cpufreq_get_freq(unsigned int cpu)
{
	return clk_get_rate(cpu_clk[cpu]) / 1000;
}

static int msm_cpufreq_init(struct cpufreq_policy *policy)
{
	int cur_freq;
	int index;
	int ret = 0;
	struct cpufreq_frequency_table *table =
			per_cpu(freq_table, policy->cpu);
	int cpu;

	/*
	 * In some SoC, some cores are clocked by same source, and their
	 * frequencies can not be changed independently. Find all other
	 * CPUs that share same clock, and mark them as controlled by
	 * same policy.
	 */
	for_each_possible_cpu(cpu)
		if (cpu_clk[cpu] == cpu_clk[policy->cpu])
			cpumask_set_cpu(cpu, policy->cpus);

	if (cpufreq_frequency_table_cpuinfo(policy, table))
	{
		// AP: set default frequencies to prevent overclocking or underclocking during start
		if (policy->cpu <= 1)
		{
			policy->cpuinfo.min_freq = CONFIG_CPU_FREQ_MIN_CLUSTER1;
			policy->cpuinfo.max_freq = CONFIG_CPU_FREQ_MAX_CLUSTER1;
		}

		if (policy->cpu >= 2)
		{
			policy->cpuinfo.min_freq = CONFIG_CPU_FREQ_MIN_CLUSTER2;
			if (socinfo_get_id() == 305)
				policy->cpuinfo.max_freq = CONFIG_CPU_FREQ_MAX_CLUSTER2PRO;
			else
				policy->cpuinfo.max_freq = CONFIG_CPU_FREQ_MAX_CLUSTER2;
		}

		pr_err("cpufreq: failed to get policy min/max\n");
	}

	// AP: set default frequencies to prevent overclocking or underclocking during start
	if (policy->cpu <= 1)
	{
		policy->min = CONFIG_CPU_FREQ_MIN_CLUSTER1;
		policy->max = CONFIG_CPU_FREQ_MAX_CLUSTER1;
	}

	if (policy->cpu >= 2)
	{
		policy->min = CONFIG_CPU_FREQ_MIN_CLUSTER2;
		if (socinfo_get_id() == 305)
			policy->max = CONFIG_CPU_FREQ_MAX_CLUSTER2PRO;
		else
			policy->max = CONFIG_CPU_FREQ_MAX_CLUSTER2;
	}

	cur_freq = clk_get_rate(cpu_clk[policy->cpu])/1000;

	if (cpufreq_frequency_table_target(policy, table, cur_freq,
	    CPUFREQ_RELATION_H, &index) &&
	    cpufreq_frequency_table_target(policy, table, cur_freq,
	    CPUFREQ_RELATION_L, &index)) {
		pr_info("cpufreq: cpu%d at invalid freq: %d\n",
				policy->cpu, cur_freq);
		return -EINVAL;
	}
	/*
	 * Call set_cpu_freq unconditionally so that when cpu is set to
	 * online, frequency limit will always be updated.
	 */
	ret = set_cpu_freq(policy, table[index].frequency,
			   table[index].driver_data);
	if (ret)
		return ret;
	pr_debug("cpufreq: cpu%d init at %d switching to %d\n",
			policy->cpu, cur_freq, table[index].frequency);
	policy->cur = table[index].frequency;
	policy->freq_table = table;

	return 0;
}

static void set_cpu_freq_pure(unsigned int cpu, unsigned int new_freq)
{
	unsigned long rate;

	rate = new_freq * 1000;
	rate = clk_round_rate(cpu_clk[cpu], rate);
	clk_set_rate(cpu_clk[cpu], rate);
}

static int msm_cpufreq_cpu_callback(struct notifier_block *nfb,
		unsigned long action, void *hcpu)
{
	unsigned int cpu = (unsigned long)hcpu;
	int rc;

	/* Fail hotplug until this driver can get CPU clocks */
	if (!hotplug_ready)
		return NOTIFY_BAD;

	switch (action & ~CPU_TASKS_FROZEN) {

	case CPU_DYING:
		clk_disable(cpu_clk[cpu]);
		clk_disable(l2_clk);
		break;
	/*
	 * Scale down clock/power of CPU that is dead and scale it back up
	 * before the CPU is brought up.
	 */
	case CPU_DEAD:
		clk_unprepare(cpu_clk[cpu]);
		clk_unprepare(l2_clk);
		break;
	case CPU_UP_CANCELED:
		clk_unprepare(cpu_clk[cpu]);
		clk_unprepare(l2_clk);
		break;
	case CPU_UP_PREPARE:
		rc = clk_prepare(l2_clk);
		if (rc < 0)
			return NOTIFY_BAD;
		rc = clk_prepare(cpu_clk[cpu]);
		if (rc < 0) {
			clk_unprepare(l2_clk);
			return NOTIFY_BAD;
		}
		break;

	case CPU_STARTING:
		rc = clk_enable(l2_clk);
		if (rc < 0)
			return NOTIFY_BAD;
		rc = clk_enable(cpu_clk[cpu]);
		if (rc) {
			clk_disable(l2_clk);
			return NOTIFY_BAD;
		}
		/*
		 * After a CPU comes online, it refuses to change its frequency
		 * to the frequency it was running at before going offline. The
		 * CPU runs at its minimum frequency when coming online, so in
		 * order to prevent the CPU from getting stuck at its minimum
		 * frequency for a prolonged amount of time, change the CPU's
		 * frequency twice to two different settings to make it respond
		 * to frequency changes again. This will make the CPU run at its
		 * maximum frequency when coming online, until the governor
		 * kicks in and changes it.
		 */
		if (max_two_freqs[cpu][1]) {
			set_cpu_freq_pure(cpu, max_two_freqs[cpu][0]);
			set_cpu_freq_pure(cpu, max_two_freqs[cpu][1]);
		}
		break;

	default:
		break;
	}

	return NOTIFY_OK;
}

static struct notifier_block __refdata msm_cpufreq_cpu_notifier = {
	.notifier_call = msm_cpufreq_cpu_callback,
};

static int msm_cpufreq_suspend(void)
{
	int cpu;

	for_each_possible_cpu(cpu) {
		mutex_lock(&per_cpu(suspend_data, cpu).suspend_mutex);
		per_cpu(suspend_data, cpu).device_suspended = 1;
		mutex_unlock(&per_cpu(suspend_data, cpu).suspend_mutex);
	}

	return NOTIFY_DONE;
}

static int msm_cpufreq_resume(void)
{
	int cpu, ret;
	struct cpufreq_policy policy;

	for_each_possible_cpu(cpu) {
		per_cpu(suspend_data, cpu).device_suspended = 0;
	}

	/*
	 * Freq request might be rejected during suspend, resulting
	 * in policy->cur violating min/max constraint.
	 * Correct the frequency as soon as possible.
	 */
	get_online_cpus();
	for_each_online_cpu(cpu) {
		ret = cpufreq_get_policy(&policy, cpu);
		if (ret)
			continue;
		if (policy.cur <= policy.max && policy.cur >= policy.min)
			continue;
		ret = cpufreq_update_policy(cpu);
		if (ret)
			pr_info("cpufreq: Current frequency violates policy min/max for CPU%d\n",
			       cpu);
		else
			pr_info("cpufreq: Frequency violation fixed for CPU%d\n",
				cpu);
	}
	put_online_cpus();

	return NOTIFY_DONE;
}

static int msm_cpufreq_pm_event(struct notifier_block *this,
				unsigned long event, void *ptr)
{
	switch (event) {
	case PM_POST_HIBERNATION:
	case PM_POST_SUSPEND:
		return msm_cpufreq_resume();
	case PM_HIBERNATION_PREPARE:
	case PM_SUSPEND_PREPARE:
		return msm_cpufreq_suspend();
	default:
		return NOTIFY_DONE;
	}
}

static struct notifier_block msm_cpufreq_pm_notifier = {
	.notifier_call = msm_cpufreq_pm_event,
};


static void msm_qos_nop(void *info)
{
}

static int msm_little_cpu_max_qos_handler(struct notifier_block *b, unsigned long val, void *v)
{
        int ret;
        unsigned long freq;
        struct cpufreq_policy *policy;
        int cpu = LITTLE_CPU_NUM;

        policy = cpufreq_cpu_get(cpu);

        if (!policy)
                goto bad;

        if (!policy->user_policy.governor) {
                cpufreq_cpu_put(policy);
                goto bad;
        }

        freq = policy->cur;  // Read the current frequency of cpu

        if (freq <= val) {
		cpufreq_cpu_put(policy);
                goto good;
	}

#if defined(CONFIG_CPU_FREQ_GOV_USERSPACE) || defined(CONFIG_CPU_FREQ_GOV_PERFORMANCE)
        if ((strcmp(policy->governor->name, "userspace") == 0)
                        || strcmp(policy->governor->name, "performance") == 0) {
                cpufreq_cpu_put(policy);
                goto good;
        }
#endif

        smp_call_function_single(cpu, msm_qos_nop, NULL, 0);

        ret = __cpufreq_driver_target(policy, val, CPUFREQ_RELATION_H);

        cpufreq_cpu_put(policy);

        if (ret < 0)
                goto bad;

good:
        return NOTIFY_OK;
bad:
        return NOTIFY_BAD;
}

static struct notifier_block msm_little_cpu_max_qos_notifier = {
        .notifier_call = msm_little_cpu_max_qos_handler,
        .priority = INT_MAX,
};

static int msm_little_cpu_min_qos_handler(struct notifier_block *b, unsigned long val, void *v)
{
        int ret;
        unsigned long freq;
        struct cpufreq_policy *policy;
        int cpu = LITTLE_CPU_NUM;

        policy = cpufreq_cpu_get(cpu);

        if (!policy)
                goto bad;

        if (!policy->user_policy.governor) {
                cpufreq_cpu_put(policy);
                goto bad;
        }

	freq = policy->cur;  // Read the current frequency of cpu

	if (freq >= val) {
		cpufreq_cpu_put(policy);
                goto good;
	}

#if defined(CONFIG_CPU_FREQ_GOV_USERSPACE) || defined(CONFIG_CPU_FREQ_GOV_PERFORMANCE)
        if ((strcmp(policy->governor->name, "userspace") == 0)
                        || strcmp(policy->governor->name, "performance") == 0) {
                cpufreq_cpu_put(policy);
                goto good;
        }
#endif

        smp_call_function_single(cpu, msm_qos_nop, NULL, 0);

        ret = __cpufreq_driver_target(policy, val, CPUFREQ_RELATION_H);

        cpufreq_cpu_put(policy);

        if (ret < 0)
                goto bad;

good:
        return NOTIFY_OK;
bad:
        return NOTIFY_BAD;
}

static struct notifier_block msm_little_cpu_min_qos_notifier = {
        .notifier_call = msm_little_cpu_min_qos_handler,
        .priority = INT_MAX,
};

static int msm_big_cpu_min_qos_handler(struct notifier_block *b, unsigned long val, void *v)
{
        int ret;
        unsigned long freq;
        struct cpufreq_policy *policy;
        int cpu = BIG_CPU_NUM;

        policy = cpufreq_cpu_get(cpu);

        if (!policy)
                goto bad;

        if (!policy->user_policy.governor) {
                cpufreq_cpu_put(policy);
                goto bad;
        }

        freq = policy->cur;  // Read the current frequency of cpu

        if (freq >= val) {
		cpufreq_cpu_put(policy);
		goto good;
	}

#if defined(CONFIG_CPU_FREQ_GOV_USERSPACE) || defined(CONFIG_CPU_FREQ_GOV_PERFORMANCE)
        if ((strcmp(policy->governor->name, "userspace") == 0)
                        || strcmp(policy->governor->name, "performance") == 0) {
                cpufreq_cpu_put(policy);
                goto good;
        }
#endif

        smp_call_function_single(cpu, msm_qos_nop, NULL, 0);

        ret = __cpufreq_driver_target(policy, val, CPUFREQ_RELATION_H);

        cpufreq_cpu_put(policy);

        if (ret < 0)
                goto bad;

good:
        return NOTIFY_OK;
bad:
        return NOTIFY_BAD;
}

static struct notifier_block msm_big_cpu_min_qos_notifier = {
        .notifier_call = msm_big_cpu_min_qos_handler,
        .priority = INT_MAX,
};

static int msm_big_cpu_max_qos_handler(struct notifier_block *b, unsigned long val, void *v)
{
        int ret;
        unsigned long freq;
        struct cpufreq_policy *policy;
        int cpu = BIG_CPU_NUM;

        policy = cpufreq_cpu_get(cpu);

        if (!policy)
                goto bad;

        if (!policy->user_policy.governor) {
                cpufreq_cpu_put(policy);
                goto bad;
        }

        freq = policy->cur;  // Read the current frequency of cpu

        if (freq <= val) {
		cpufreq_cpu_put(policy);
                goto good;
	}

#if defined(CONFIG_CPU_FREQ_GOV_USERSPACE) || defined(CONFIG_CPU_FREQ_GOV_PERFORMANCE)
        if ((strcmp(policy->governor->name, "userspace") == 0)
                        || strcmp(policy->governor->name, "performance") == 0) {
                cpufreq_cpu_put(policy);
                goto good;
        }
#endif

        smp_call_function_single(cpu, msm_qos_nop, NULL, 0);

        ret = __cpufreq_driver_target(policy, val, CPUFREQ_RELATION_H);

        cpufreq_cpu_put(policy);

        if (ret < 0)
                goto bad;

good:
        return NOTIFY_OK;
bad:
        return NOTIFY_BAD;
}

static struct notifier_block msm_big_cpu_max_qos_notifier = {
        .notifier_call = msm_big_cpu_max_qos_handler,
        .priority = INT_MAX,
};

static struct freq_attr *msm_freq_attr[] = {
	&cpufreq_freq_attr_scaling_available_freqs,
	NULL,
};

static struct cpufreq_driver msm_cpufreq_driver = {
	/* lps calculations are handled here. */
	.flags		= CPUFREQ_STICKY | CPUFREQ_CONST_LOOPS,
	.init		= msm_cpufreq_init,
	.verify		= msm_cpufreq_verify,
	.target		= msm_cpufreq_target,
	.get		= msm_cpufreq_get_freq,
	.name		= "msm",
	.attr		= msm_freq_attr,
};

static struct cpufreq_frequency_table *cpufreq_parse_dt(struct device *dev,
						char *tbl_name, int cpu)
{
	int ret, nf, i;
	u32 *data;
	struct cpufreq_frequency_table *ftbl;

	/* Parse list of usable CPU frequencies. */
	if (!of_find_property(dev->of_node, tbl_name, &nf))
		return ERR_PTR(-EINVAL);
	nf /= sizeof(*data);

	if (nf == 0)
		return ERR_PTR(-EINVAL);

	data = devm_kzalloc(dev, nf * sizeof(*data), GFP_KERNEL);
	if (!data)
		return ERR_PTR(-ENOMEM);

	ret = of_property_read_u32_array(dev->of_node, tbl_name, data, nf);
	if (ret)
		return ERR_PTR(ret);

	ftbl = devm_kzalloc(dev, (nf + 1) * sizeof(*ftbl), GFP_KERNEL);
	if (!ftbl)
		return ERR_PTR(-ENOMEM);

	for (i = 0; i < nf; i++) {
		unsigned long f;

		f = clk_round_rate(cpu_clk[cpu], data[i] * 1000);
		if (IS_ERR_VALUE(f))
			break;
		f /= 1000;

		/*
		 * Check if this is the last feasible frequency in the table.
		 *
		 * The table listing frequencies higher than what the HW can
		 * support is not an error since the table might be shared
		 * across CPUs in different speed bins. It's also not
		 * sufficient to check if the rounded rate is lower than the
		 * requested rate as it doesn't cover the following example:
		 *
		 * Table lists: 2.2 GHz and 2.5 GHz.
		 * Rounded rate returns: 2.2 GHz and 2.3 GHz.
		 *
		 * In this case, we can CPUfreq to use 2.2 GHz and 2.3 GHz
		 * instead of rejecting the 2.5 GHz table entry.
		 */
		if (i > 0 && f <= ftbl[i-1].frequency)
			break;

		ftbl[i].driver_data = i;
		ftbl[i].frequency = f;
	}

	max_two_freqs[cpu][0] = ftbl[i - 2].frequency;
	max_two_freqs[cpu][1] = ftbl[i - 1].frequency;

	ftbl[i].driver_data = i;
	ftbl[i].frequency = CPUFREQ_TABLE_END;

	devm_kfree(dev, data);

	return ftbl;
}

static int __init msm_cpufreq_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	char clk_name[] = "cpu??_clk";
	char tbl_name[] = "qcom,cpufreq-table-??";
	struct clk *c;
	int cpu;
	struct cpufreq_frequency_table *ftbl;

	l2_clk = devm_clk_get(dev, "l2_clk");
	if (IS_ERR(l2_clk))
		l2_clk = NULL;

	for_each_possible_cpu(cpu) {
		snprintf(clk_name, sizeof(clk_name), "cpu%d_clk", cpu);
		c = devm_clk_get(dev, clk_name);
		if (IS_ERR(c))
			return PTR_ERR(c);
		cpu_clk[cpu] = c;
	}
	hotplug_ready = true;

	/* Use per-policy governor tunable for some targets */
	if (of_property_read_bool(dev->of_node, "qcom,governor-per-policy"))
		msm_cpufreq_driver.flags |= CPUFREQ_HAVE_GOVERNOR_PER_POLICY;

	/* Parse commong cpufreq table for all CPUs */
	ftbl = cpufreq_parse_dt(dev, "qcom,cpufreq-table", 0);
	if (!IS_ERR(ftbl)) {
		for_each_possible_cpu(cpu)
			per_cpu(freq_table, cpu) = ftbl;
		return 0;
	}

	/*
	 * No common table. Parse individual tables for each unique
	 * CPU clock.
	 */
	for_each_possible_cpu(cpu) {
		snprintf(tbl_name, sizeof(tbl_name),
			 "qcom,cpufreq-table-%d", cpu);
		ftbl = cpufreq_parse_dt(dev, tbl_name, cpu);

		/* CPU0 must contain freq table */
		if (cpu == 0 && IS_ERR(ftbl)) {
			dev_err(dev, "Failed to parse CPU0's freq table\n");
			return PTR_ERR(ftbl);
		}
		if (cpu == 0) {
			per_cpu(freq_table, cpu) = ftbl;
			continue;
		}

		if (cpu_clk[cpu] != cpu_clk[cpu - 1] && IS_ERR(ftbl)) {
			dev_err(dev, "Failed to parse CPU%d's freq table\n",
				cpu);
			return PTR_ERR(ftbl);
		}

		/* Use previous CPU's table if it shares same clock */
		if (cpu_clk[cpu] == cpu_clk[cpu - 1]) {
			if (!IS_ERR(ftbl)) {
				dev_warn(dev, "Conflicting tables for CPU%d\n",
					 cpu);
				devm_kfree(dev, ftbl);
			}
			ftbl = per_cpu(freq_table, cpu - 1);
#ifdef CONFIG_CPU_FREQ_LIMIT_BOOT_CURRENT
		} else {
			if(!IS_ERR(ftbl))
				cluster1_first_cpu = cpu;
			//pr_info("cluster1_first_cpu: %d",cluster1_first_cpu);
		}
#else
		}
#endif
		per_cpu(freq_table, cpu) = ftbl;
	}

	pm_qos_add_notifier(PM_QOS_LITTLE_CPU_FREQ_MIN, &msm_little_cpu_min_qos_notifier);
	pm_qos_add_notifier(PM_QOS_LITTLE_CPU_FREQ_MAX, &msm_little_cpu_max_qos_notifier);
	pm_qos_add_notifier(PM_QOS_BIG_CPU_FREQ_MIN, &msm_big_cpu_min_qos_notifier);
	pm_qos_add_notifier(PM_QOS_BIG_CPU_FREQ_MAX, &msm_big_cpu_max_qos_notifier);

	return 0;
}

static struct of_device_id match_table[] = {
	{ .compatible = "qcom,msm-cpufreq" },
	{}
};

static struct platform_driver msm_cpufreq_plat_driver = {
	.driver = {
		.name = "msm-cpufreq",
		.of_match_table = match_table,
		.owner = THIS_MODULE,
	},
};

static int __init msm_cpufreq_register(void)
{
	int cpu, rc;

	for_each_possible_cpu(cpu) {
		mutex_init(&(per_cpu(suspend_data, cpu).suspend_mutex));
		per_cpu(suspend_data, cpu).device_suspended = 0;
	}

	rc = platform_driver_probe(&msm_cpufreq_plat_driver,
				   msm_cpufreq_probe);
	if (rc < 0) {
		/* Unblock hotplug if msm-cpufreq probe fails */
		unregister_hotcpu_notifier(&msm_cpufreq_cpu_notifier);
		for_each_possible_cpu(cpu)
			mutex_destroy(&(per_cpu(suspend_data, cpu).
					suspend_mutex));
		return rc;
	}
	register_pm_notifier(&msm_cpufreq_pm_notifier);
	return cpufreq_register_driver(&msm_cpufreq_driver);
}

subsys_initcall(msm_cpufreq_register);

static int __init msm_cpufreq_early_register(void)
{
	return register_hotcpu_notifier(&msm_cpufreq_cpu_notifier);
}
core_initcall(msm_cpufreq_early_register);
