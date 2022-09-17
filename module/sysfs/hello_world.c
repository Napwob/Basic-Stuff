#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/module.h>
#include <linux/init.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vlad Rusmanov");
MODULE_DESCRIPTION("\"SYSFS WRITE!\" module");
MODULE_VERSION("0.0.3");


static int foo;
static int baz;
static int bar;

static ssize_t b_show(struct kobject *kobj, struct kobj_attribute *attr,
		      char *buf)
{
	int var;
	printk("Read something");
	if (strcmp(attr->attr.name, "baz") == 0)
		var = baz;
	if (strcmp(attr->attr.name, "bar") == 0)
		var = bar;
	if (strcmp(attr->attr.name, "foo") == 0)
		var = foo;
		
	return sysfs_emit(buf, "%d\n", var);
}

static ssize_t b_store(struct kobject *kobj, struct kobj_attribute *attr,
		       const char *buf, size_t count)
{
	int var, ret;
	printk("Write something");
	ret = kstrtoint(buf, 10, &var);
	if (ret < 0)
		return ret;

	if (strcmp(attr->attr.name, "baz") == 0)
		baz = var;
	if (strcmp(attr->attr.name, "bar") == 0)
		bar = var;
	if (strcmp(attr->attr.name, "foo") == 0)
		foo = var;
			
	return count;
}

static struct kobj_attribute foo_attribute =
	__ATTR(foo, 0664, b_show, b_store);
static struct kobj_attribute baz_attribute =
	__ATTR(baz, 0664, b_show, b_store);
static struct kobj_attribute bar_attribute =
	__ATTR(bar, 0664, b_show, b_store);


static struct attribute *attrs[] = {
	&foo_attribute.attr,
	&baz_attribute.attr,
	&bar_attribute.attr,
	NULL,	
};


static struct attribute_group attr_group = {
	.attrs = attrs,
};

static struct kobject *example_kobj;

static int __init example_init(void)
{
	int retval;
	printk("LOAD KERNEL MODULE");
	example_kobj = kobject_create_and_add("kobject_example", kernel_kobj);
	if (!example_kobj)
		return -ENOMEM;

	retval = sysfs_create_group(example_kobj, &attr_group);
	if (retval)
		kobject_put(example_kobj);
	printk("LOADING COMPLETE\n\n");
	return retval;
}

static void __exit example_exit(void)
{
	kobject_put(example_kobj);
	printk("KERNEL MODULE UNLOADED");
}

module_init(example_init);
module_exit(example_exit);


