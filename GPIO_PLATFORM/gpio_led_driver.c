#include<linux/module.h>
#include<linux/platform_device.h>
#include<linux/of.h>
#include<linux/gpio/consumer.h>
#include<linux/delay.h>


static struct gpio_desc *led_gpio;

static struct gpio_led_probe(struct platform_device *pdev)
{
	pr_info("GPIO LED: Probing device\n");
	
	led_gpio = gpio_get(&pdev->dev,"led",GPIOD_OUT_LOW);
	if(IS_ERR(led_gpio))
	{
		pr_err("GPIO LED: Failed to get GPIO\n");
		return PTR_ERR(led_gpio);
	}
	
	pr_info("GPIO LED: Successfully acquired GPIO\n");
	
	gpiod_set_value(led_gpio,1);
	msleep(500);
	gpiod_set_value(led_gpio,0);
	pr_info("GPIO LED: LED Blink Done\n");
	
	return 0;
}

static struct gpio_led_remove(struct platform_device *pdev)
{
	pr_info("GPIO LED : Removing device\n");
	
	gpiod_set_value(led_gpio,0);
	gpiod_put(led_gpio);
	return 0;
}


static const struct of_device_id gpio_led_of_match[] = {
	{ .compatible = "rpi,gpio-led-blink", },
	{ },
};

static struct platform_driver gpio_led_driver = {
	.probe = gpio_led_probe,
	.remove = gpio_led_remove,
	.driver = {
		.name = "gpio_led_driver",
		.of_match_table = gpio_led_of_match,
	},
};

module_platform_driver(gpio_led_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("afreen");
MODULE_DESCRIPTION("GPIO Platform driver for LED Blinking");
		

