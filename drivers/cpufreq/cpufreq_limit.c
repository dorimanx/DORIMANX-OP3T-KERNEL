/*
 * QCOM CPU Frequency Limiter Driver
 *
 * Copyright (c) 2012-2014, Paul Reioux Faux123 <reioux@gmail.com>
 * Copyright (c) 2014, Pranav Vashi <neobuddy89@gmail.com>
 * Copyright (c) 2014-2017, Dorimanx <yuri@bynet.co.il>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/module.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/workqueue.h>
#include <linux/cpu.h>
#include <linux/cpufreq.h>
#include <soc/qcom/socinfo.h>

#define QCOM_CPUFREQ_LIMIT_MAJOR		1
#define QCOM_CPUFREQ_LIMIT_MINOR		0

#define QCOM_LIMIT			"qcom_cpufreq_limit"

static struct mutex qcom_limiter_mutex;

#define multi_cpu(cpu)					\
static ssize_t store_qcom_cpufreq_max_limit_cpu##cpu	\
(struct kobject *kobj, 					\
 struct kobj_attribute *attr, 				\
 const char *buf, size_t count)				\
{							\
	int ret;					\
	unsigned int val;				\
	ret = sscanf(buf, "%u\n", &val);		\
	if (ret != 1)					\
		return -EINVAL;				\
							\
	mutex_lock(&qcom_limiter_mutex);		\
	set_cpu_max_lock(cpu, val);				\
	mutex_unlock(&qcom_limiter_mutex);		\
	return count;					\
}							\
static ssize_t show_qcom_cpufreq_max_limit_cpu##cpu	\
(struct kobject *kobj,					\
 struct kobj_attribute *attr, char *buf)		\
{							\
	return sprintf(buf, "%u\n",			\
			get_cpu_max_lock(cpu));		\
}							\
static ssize_t store_qcom_cpufreq_min_limit_cpu##cpu	\
(struct kobject *kobj,					\
 struct kobj_attribute *attr,				\
 const char *buf, size_t count)				\
{							\
	int ret;					\
	unsigned int val;				\
	ret = sscanf(buf, "%u\n", &val);		\
	if (ret != 1)					\
		return -EINVAL;				\
							\
	mutex_lock(&qcom_limiter_mutex);		\
	set_cpu_min_lock(cpu, val);			\
	mutex_unlock(&qcom_limiter_mutex);		\
	return count;					\
}							\
static ssize_t show_qcom_cpufreq_min_limit_cpu##cpu	\
(struct kobject *kobj,					\
 struct kobj_attribute *attr, char *buf)		\
{							\
	return sprintf(buf, "%u\n",			\
		get_cpu_min_lock(cpu));			\
}							\
static struct kobj_attribute qcom_cpufreq_max_limit_cpu##cpu =	\
	__ATTR(cpufreq_max_limit_cpu##cpu, 0644,		\
		show_qcom_cpufreq_max_limit_cpu##cpu,		\
		store_qcom_cpufreq_max_limit_cpu##cpu);		\
static struct kobj_attribute qcom_cpufreq_min_limit_cpu##cpu =	\
	__ATTR(cpufreq_min_limit_cpu##cpu, 0644,		\
		show_qcom_cpufreq_min_limit_cpu##cpu,		\
		store_qcom_cpufreq_min_limit_cpu##cpu);		\

multi_cpu(0);
multi_cpu(1);
multi_cpu(2);
multi_cpu(3);

static ssize_t qcom_cpufreq_limit_version_show(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "version: %u.%u\n",
			QCOM_CPUFREQ_LIMIT_MAJOR, QCOM_CPUFREQ_LIMIT_MINOR);
}

static struct kobj_attribute qcom_cpufreq_limit_version_attribute =
	__ATTR(qcom_cpufreq_limit_version, 0444,
		qcom_cpufreq_limit_version_show,
		NULL);

static struct attribute *qcom_cpufreq_limit_attrs[] =
	{
		&qcom_cpufreq_max_limit_cpu0.attr,
		&qcom_cpufreq_max_limit_cpu1.attr,
		&qcom_cpufreq_max_limit_cpu2.attr,
		&qcom_cpufreq_max_limit_cpu3.attr,
		&qcom_cpufreq_min_limit_cpu0.attr,
		&qcom_cpufreq_min_limit_cpu1.attr,
		&qcom_cpufreq_min_limit_cpu2.attr,
		&qcom_cpufreq_min_limit_cpu3.attr,
		&qcom_cpufreq_limit_version_attribute.attr,
		NULL,
	};

static struct attribute_group qcom_cpufreq_limit_attr_group =
	{
		.attrs = qcom_cpufreq_limit_attrs,
	};

static struct kobject *qcom_cpufreq_limit_kobj;

static int qcom_cpufreq_limit_init(void)
{
	int ret;

	qcom_cpufreq_limit_kobj =
		kobject_create_and_add(QCOM_LIMIT, kernel_kobj);
	if (!qcom_cpufreq_limit_kobj) {
		pr_err("%s: kobject create failed!\n",
			QCOM_LIMIT);
		return -ENOMEM;
        }

	ret = sysfs_create_group(qcom_cpufreq_limit_kobj,
			&qcom_cpufreq_limit_attr_group);

        if (ret) {
		pr_err("%s: create failed!\n",
			QCOM_LIMIT);
		goto err_dev;
	}

	mutex_init(&qcom_limiter_mutex);

	return ret;
err_dev:
	if (qcom_cpufreq_limit_kobj != NULL)
		kobject_put(qcom_cpufreq_limit_kobj);
	return ret;
}

static void qcom_cpufreq_limit_exit(void)
{
	if (qcom_cpufreq_limit_kobj != NULL)
		kobject_put(qcom_cpufreq_limit_kobj);

	mutex_destroy(&qcom_limiter_mutex);
}

module_init(qcom_cpufreq_limit_init);
module_exit(qcom_cpufreq_limit_exit);

MODULE_AUTHOR("Paul Reioux <reioux@gmail.com>, \
		Dorimanx <yuri@bynet.co.il>, \
		Pranav Vashi <neobuddy89@gmail.com>");
MODULE_DESCRIPTION("QCOM CPU Frequency Limiter Driver");
MODULE_LICENSE("GPL v2");
