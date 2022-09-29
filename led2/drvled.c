#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/ioport.h>
#include <linux/leds.h>
#include <linux/slab.h>
#include <linux/init.h>
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


struct drvled_data_st {
    void __iomem *regbase;
    struct led_classdev led_cdev;
};

static struct drvled_data_st *drvled_data;


static void drvled_setled(unsigned int status)
{
    /* Set value */
    if (status == LED_ON)
    {
       writel(GPIO_BIT, drvled_data->regbase + GPIO_GPSET);
    }
    else if (status == LED_OFF)
    {
        writel(GPIO_BIT, drvled_data->regbase + GPIO_GPCLR);
    }
}


static void drvled_setdirection(void)
{
    u32 val;

    val = readl(drvled_data->regbase + GPIO_GPFSEL);
    val &= ~(7 << 6);
    val |= (1 << 6);
    writel(val, drvled_data->regbase + GPIO_GPFSEL);
}


static void drvled_change_state(struct led_classdev *led_cdev,
                                enum led_brightness brightness)
{
    if (brightness)
    {
        drvled_setled(LED_ON);
    }
    else
    {
        drvled_setled(LED_OFF);
    }
}


static int __init drvled_init(void)
{
    int err;

    drvled_data = kzalloc(sizeof(*drvled_data), GFP_KERNEL);

    if (!drvled_data)
    {
        err = -ENOMEM;
        goto err_kzalloc;
    }
    /*
    if (!request_mem_region(GPIO_BASE, GPIO_SIZE, DRIVER_NAME))
    {
        pr_err("%s: Error requesting I/O!\n", DRIVER_NAME);
        err = -ENODEV;
        goto err_request_mem;
    }
    */

    drvled_data->regbase = ioremap(GPIO_BASE, GPIO_SIZE);

    if (!drvled_data->regbase)
    {
        pr_err("%s: Error mapping I/O!\n", DRIVER_NAME);
        err = -ENOMEM;
        goto err_ioremap;
    }

    drvled_data->led_cdev.name = "ipe:green:user";
    drvled_data->led_cdev.brightness_set = drvled_change_state;

    err = led_classdev_register(NULL, &drvled_data->led_cdev);

    if (err)
    {
        pr_err("%s: Failed to register led!\n", DRIVER_NAME);
        goto err_led_reg;
    }

    drvled_setdirection();

    drvled_setled(LED_OFF);

    pr_info("%s: Initialized.\n", DRIVER_NAME);

    return 0;

    err_led_reg:
        iounmap(drvled_data->regbase);
    err_ioremap:
      /* release_mem_region(GPIO_BASE, GPIO_SIZE); */
    err_request_mem:
        kfree(drvled_data);
    err_kzalloc:
        return err;
}


static void __exit drvled_exit(void)
{
    led_classdev_unregister(&drvled_data->led_cdev);
    iounmap(drvled_data->regbase);
    /* release_mem_region(GPIO_BASE, GPIO_SIZE); */
    kfree(drvled_data);
    pr_info("%s exiting.\n", DRIVER_NAME);
}


module_init(drvled_init);
module_exit(drvled_exit);

MODULE_AUTHOR("clip");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("led driver");
