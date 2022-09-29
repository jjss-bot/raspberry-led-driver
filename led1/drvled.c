#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#include <linux/module.h>
#include <linux/ioport.h>
#include <linux/io.h>

#define DRIVER_NAME     "drvled"

#define LED_ON              1
#define LED_OFF             0

#define GPIO_BASE       0x20200000
#define GPIO_SIZE           48

#define GPIO_GPFSEL          8
#define GPIO_GPSET          28
#define GPIO_GPCLR          40

#define GPIO_BIT         (1 << 22)


static struct {
    dev_t devnum;
    struct cdev cdev;
    unsigned int led_status;
    void __iomem *regbase;
} drvled_data;


static void drvled_setled(unsigned int status)
{
    u32 val = GPIO_BIT;
    /* Set value */
    if (status == LED_ON)
    {
        writel(GPIO_BIT, drvled_data.regbase + GPIO_GPSET);
    }
    else if (status == LED_OFF)
    {
        writel(GPIO_BIT, drvled_data.regbase + GPIO_GPCLR);
    }

    /* Update status */
    drvled_data.led_status = status;
}


static void drvled_setdirection(void)
{
    u32 val;

    val = readl(drvled_data.regbase + GPIO_GPFSEL);
    val &= ~(7 << 6);
    val |= (1 << 6);
    writel(val, drvled_data.regbase + GPIO_GPFSEL);
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

    /*if (!request_mem_region(GPIO_BASE, GPIO_SIZE, DRIVER_NAME))
    {
        pr_err("%s: Error requesting I/O!\n", DRIVER_NAME);
        err = -ENODEV;
        goto err_request_mem;
    }*/

    drvled_data.regbase = ioremap(GPIO_BASE, GPIO_SIZE);

    if (!drvled_data.regbase)
    {
        pr_err("%s: Error mapping I/O!\n", DRIVER_NAME);
        err = -ENOMEM;
        goto err_ioremap;
    }

    err = alloc_chrdev_region(&drvled_data.devnum, 0, 1, DRIVER_NAME);

    if (err)
    {
        pr_err("%s: Failed to allocate device number!\n", DRIVER_NAME);
        goto err_chrdev;
    }

    drvled_data.cdev.owner = THIS_MODULE;
    cdev_init(&drvled_data.cdev, &drvled_fops);
    err = cdev_add(&drvled_data.cdev, drvled_data.devnum, 1);

    if (err)
    {
        pr_err("%s: Char device registration failed!\n", DRIVER_NAME);
        goto err_cdev;
    }

    drvled_setdirection();

    drvled_setled(LED_OFF);

    pr_info("%s: Initialized.\n", DRIVER_NAME);

    return 0;

    err_cdev:
        unregister_chrdev_region(drvled_data.devnum, 1);
    err_chrdev:
        iounmap(drvled_data.regbase);
    err_ioremap:
       /* release_mem_region(GPIO_BASE, GPIO_SIZE);*/
    err_request_mem:
        return err;
}

static void __exit drvled_exit(void)
{
    cdev_del(&drvled_data.cdev);
    unregister_chrdev_region(drvled_data.devnum, 1);
    iounmap(drvled_data.regbase);
    /*release_mem_region(GPIO_BASE, GPIO_SIZE);*/
    pr_info("%s exiting.\n", DRIVER_NAME);
}


module_init(drvled_init);
module_exit(drvled_exit);

MODULE_AUTHOR("clip");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("led driver");
