#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/init.h>

#define DRIVER_NAME "led_gpio_driver"
#define DEVICE_NAME "led_gpio"
#define CLASS_NAME  "led"

#define GPIO_LED 529   // BCM GPIO 17

static int major;
static struct class*  led_class  = NULL;
static struct device* led_device = NULL;


// Function is called when user space application calls open()
static int led_open(struct inode *inodep, struct file *filep) 
{
    pr_info("LED: Device opened\n");
    return 0;
}


// Function is called when user space application calls close()
static int led_release(struct inode *inodep, struct file *filep) 
{
    pr_info("LED: Device closed\n");
    return 0;
}

// Function to read user space data and control LED
static ssize_t led_write(struct file *filep, const char *buffer, size_t len, loff_t *offset) 
{
    char msg[2] = {0};

    if (len > 2)
        return -EINVAL;

    if (copy_from_user(msg, buffer, 1)) // Safely copies from user space buffer to kernel space buffer
    {
        pr_err("LED: Failed to receive data from user\n");
        return -EFAULT;
    }

    if (msg[0] == '1') 
    {
        gpio_set_value(GPIO_LED, 1);
        pr_info("LED: ON\n");
    } 
    else if (msg[0] == '0') 
    {
        gpio_set_value(GPIO_LED, 0);
        pr_info("LED: OFF\n");
    }
    else 
    {
        pr_warn("LED: Invalid write value. Use '1' or '0'\n");
        return -EINVAL;
    }

    return len;
}

// Structure defining file functions with respect to their user space operations 
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = led_open,
    .write = led_write,
    .release = led_release,
};


// Function is called when the kernel module is loaded
static int __init led_init(void) 
{
    int ret;

    pr_info("LED: Initializing driver...\n");

    if (!gpio_is_valid(GPIO_LED)) // Checks gpio pin is valid or not
    {
        pr_err("LED: Invalid GPIO %d\n", GPIO_LED);
        return -ENODEV;
    }

    ret = gpio_request(GPIO_LED, "LED_GPIO"); // Requests ownership of gpio pin
    if (ret) 
    {
        pr_err("LED: GPIO request failed\n");
        return ret;
    }

    gpio_direction_output(GPIO_LED, 0);   // Sets direction to output

    major = register_chrdev(0, DEVICE_NAME, &fops); // Registers a character device driver with dynamically allocated major number
    if (major < 0) 
    {
        pr_err("LED: Failed to register char device\n");
        gpio_free(GPIO_LED);
        return major;
    }

    led_class = class_create(CLASS_NAME);  // Creates class with given class name
    if (IS_ERR(led_class)) 
    {
        unregister_chrdev(major, DEVICE_NAME);
        gpio_free(GPIO_LED);
        pr_err("LED: Failed to create class\n");
        return PTR_ERR(led_class);
    }

    led_device = device_create(led_class, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);  // Creates device file in /dev/ 
    if (IS_ERR(led_device)) 
    {
        class_destroy(led_class);
        unregister_chrdev(major, DEVICE_NAME);
        gpio_free(GPIO_LED);
        pr_err("LED: Failed to create device\n");
        return PTR_ERR(led_device);
    }

    pr_info("LED: Driver loaded successfully. Use /dev/%s\n", DEVICE_NAME);
    return 0;
}


// Function is called when module is unloaded
static void __exit led_exit(void) 
{
    gpio_set_value(GPIO_LED, 0);  // Sets gpio pin value to 0
    gpio_free(GPIO_LED);          // Frees gpio pin

    device_destroy(led_class, MKDEV(major, 0));  // Destroys the device and its associated major and minor number
    class_unregister(led_class);  // De-registers class
    class_destroy(led_class);     // Destroys class
    unregister_chrdev(major, DEVICE_NAME); // Unregisters character device driver

    pr_info("LED: Driver unloaded\n");
}

module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Batch5");
MODULE_DESCRIPTION("GPIO LED Driver for Raspberry Pi - Kernel 6.12+");


