#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/leds.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/gpio/consumer.h>

#define DRIVER_NAME     "drvled"

#define LED_ON              1
#define LED_OFF             0

#define GPIO_NUM            22


struct drvled_data_st {
    struct gpio_desc *desc;
    struct led_classdev led_cdev;
};

static struct drvled_data_st *drvled_data;


static void drvled_setled(unsigned int status)
{
    /* Set value */
    if (status == LED_ON)
    {
        gpiod_set_value(drvled_data->desc, 1);
    }
    else if (status == LED_OFF)
    {
        gpiod_set_value(drvled_data->desc, 0);
    }
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

    err = gpio_request_one(GPIO_NUM, GPIOF_DIR_OUT | GPIOF_INIT_LOW, "green led");

    if (err)
    {
        pr_err("%s: Error requesting GPIO\n", DRIVER_NAME);
        goto err_gpio_request;
    }

    drvled_data->desc = gpio_to_desc(GPIO_NUM);

    drvled_data->led_cdev.name = "ipe:green:user";
    drvled_data->led_cdev.brightness_set = drvled_change_state;

    err = led_classdev_register(NULL, &drvled_data->led_cdev);

    if (err)
    {
        pr_err("%s: Failed to register led!\n", DRIVER_NAME);
        goto err_led_reg;
    }

    pr_info("%s: Initialized.\n", DRIVER_NAME);

    return 0;

    err_led_reg:
        gpio_free(GPIO_NUM);
    err_gpio_request:
        kfree(drvled_data);
    err_kzalloc:
        return err;
}


static void __exit drvled_exit(void)
{
    led_classdev_unregister(&drvled_data->led_cdev);
    gpio_free(GPIO_NUM);
    kfree(drvled_data);
    pr_info("%s exiting.\n", DRIVER_NAME);
}


module_init(drvled_init);
module_exit(drvled_exit);

MODULE_AUTHOR("clip");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("led driver");
