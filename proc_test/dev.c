#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>

static ssize_t dev_read(struct file *file, char __user *buffer, size_t size, loff_t *offset);
static ssize_t dev_write(struct file *file, const char __user *buffer, size_t size, loff_t *offset);

static struct proc_dir_entry *d_entry;
static struct proc_dir_entry *f_entry;

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.read = dev_read,
	.write = dev_write,
};


static ssize_t dev_read(struct file *file, char __user *buffer, size_t size, loff_t *offset)
{
	pr_alert("proc read\n");

	return 0;
}


static ssize_t dev_write(struct file *file, const char __user *buffer, size_t size, loff_t *offset)
{
	pr_alert("proc write\n");

	return -1;
}


static int __init dev_init(void)
{
	d_entry = proc_mkdir("mydir", NULL);

	if (!d_entry)
	{
		pr_alert("proc dir create fail");
		return -1;
	}

	f_entry = proc_create("myproc", 0666, d_entry, &fops);

	if (!f_entry)
	{
		pr_alert("proc file create fail");
		proc_remove(d_entry);
		return -1;
	}

	pr_alert("proc test init\n");

	return 0;
}


static void __exit dev_exit(void)
{
	proc_remove(d_entry);
	proc_remove(f_entry);
	pr_alert("proc test exit\n");
}


module_init(dev_init);
module_exit(dev_exit);


MODULE_AUTHOR("clip");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("proc test");
