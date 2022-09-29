#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#include <linux/module.h>

#define DRIVER_NAME     "drvled"

#define LED_ON              1
#define LED_OFF             0

static struct {
    dev_t devnum;
    struct cdev cdev;
    unsigned int led_status;
} drvled_data;


static void drvled_setled(unsigned int status)
{
    drvled_data.led_status = status;
}

static ssize_t drvled_read(struct file *file, char __user *buff, size_t size, loff_t *offset)
{
    static const char *const msg[] = { "OFF\n", "ON\n" };

    size_t len = strlen(msg[drvled_data.led_status]);
    ssize_t count = min((size_t)(len - *offset), size);

    /* check if EOF */
    if (count <= 0)
        return 0;

    if (copy_to_user(buff, msg[drvled_data.led_status], count))
        return -EFAULT;

    *offset += count;

    return count;
}

static ssize_t drvled_write(struct file *file, const char __user *buff, size_t size, loff_t *offset)
{
    char kbuff = 0;

    if (copy_from_user(&kbuff, buff, 1))
        return -EFAULT;

    if (kbuff == '1')
    {
        drvled_setled(LED_ON);
        pr_info("LED_ON\n");
    }
    else if (kbuff == '0')
    {
        drvled_setled(LED_OFF);
        pr_info("LED_OFF\n");
    }

    return size;
}

static struct file_operations drvled_fops = {
    .owner = THIS_MODULE,
    .write = drvled_write,
    .read = drvled_read,
};

static int __init drvled_init(void)
{
    int err;

    err = alloc_chrdev_region(&drvled_data.devnum, 0, 1, DRIVER_NAME);

    if (err)
    {
        pr_err("%s: Failed to allocate device number!\n", DRIVER_NAME);
        return err;
    }

    drvled_data.cdev.owner = THIS_MODULE;
    cdev_init(&drvled_data.cdev, &drvled_fops);
    err = cdev_add(&drvled_data.cdev, drvled_data.devnum, 1);

    if (err)
    {
        pr_err("%s: Char device registration failed!\n", DRIVER_NAME);
        unregister_chrdev_region(drvled_data.devnum, 1);
        return err;
    }

    drvled_setled(LED_OFF);

    pr_info("%s: Initialized.\n", DRIVER_NAME);

    return 0;
}

static void __exit drvled_exit(void)
{
    cdev_del(&drvled_data.cdev);
    unregister_chrdev_region(drvled_data.devnum, 1);
    pr_info("%s exiting.\n", DRIVER_NAME);
}


module_init(drvled_init);
module_exit(drvled_exit);

MODULE_AUTHOR("clip");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("led driver");
