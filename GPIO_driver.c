// GPIO_driver.c
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include <linux/timer.h>
#include <linux/device.h>

#define DEVICE_NAME "virtualtemp"
#define CLASS_NAME  "virt_temp"

// IOCTL commands
#define IOCTL_SET_THRESHOLD _IOW('t', 1, int)
#define IOCTL_GET_THRESHOLD _IOR('t', 2, int)
#define IOCTL_GET_ALERTS_COUNT    _IOR('t', 3, int)

static dev_t dev_number;
static struct cdev gpio_cdev;
static struct class *gpio_class;

static struct timer_list temp_timer;
static int current_temp = 25;
static int temp_threshold = 40;
static int alert_count = 0;
static bool alert_triggered = false;

static void temp_timer_callback(struct timer_list *t) {
    current_temp++;
    printk(KERN_INFO "gpio_driver: Current Temp = %d\n", current_temp);

    if (current_temp > temp_threshold && !alert_triggered) {
        alert_count++;
        alert_triggered = true;
        printk(KERN_ALERT "gpio_driver: ALERT! Temperature exceeded threshold!\n");
    }
    mod_timer(&temp_timer, jiffies + msecs_to_jiffies(2000));
}

static int dev_open(struct inode *inode, struct file *file) {
    printk(KERN_INFO "gpio_driver: Device opened\n");
    return 0;
}

static int dev_release(struct inode *inode, struct file *file) {
    printk(KERN_INFO "gpio_driver: Device closed\n");
    return 0;
}

static ssize_t dev_read(struct file *file, char __user *buf, size_t len, loff_t *offset) {
    int ret;
    if(*offset!=0)
	    return 0;
    if(len<sizeof(int))
	    return -EINVAL;

	if (copy_to_user(buf, &current_temp, sizeof(current_temp)))
      
		return -EFAULT;

    *offset+=sizeof(current_temp);
    return sizeof(current_temp);
}

static long dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    int val;
    switch (cmd) {
        case IOCTL_SET_THRESHOLD:
            if (copy_from_user(&val, (int __user *)arg, sizeof(val)))
                return -EFAULT;
            temp_threshold = val;
            alert_triggered = false; // reset alert flag
            printk(KERN_INFO "gpio_driver: Threshold set to %d\n", temp_threshold);
            break;

        case IOCTL_GET_THRESHOLD:
            if (copy_to_user((int __user *)arg, &temp_threshold, sizeof(temp_threshold)))
                return -EFAULT;
            break;

        case IOCTL_GET_ALERTS_COUNT:
            if (copy_to_user((int __user *)arg, &alert_count, sizeof(alert_count)))
                return -EFAULT;
            break;

        default:
            return -EINVAL;
    }
    return 0;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = dev_open,
    .release = dev_release,
    .read = dev_read,
    .unlocked_ioctl = dev_ioctl,
};

static int __init gpio_driver_init(void) {
    int ret;

    ret = alloc_chrdev_region(&dev_number, 0, 1, DEVICE_NAME);
    if (ret < 0) {
        printk(KERN_ERR "gpio_driver: Failed to allocate char device region\n");
        return ret;
    }

    cdev_init(&gpio_cdev, &fops);
    ret = cdev_add(&gpio_cdev, dev_number, 1);
    if (ret < 0) {
        unregister_chrdev_region(dev_number, 1);
        printk(KERN_ERR "gpio_driver: Failed to add cdev\n");
        return ret;
    }

    gpio_class = class_create(CLASS_NAME);
    if (IS_ERR(gpio_class)) {
        cdev_del(&gpio_cdev);
        unregister_chrdev_region(dev_number, 1);
        printk(KERN_ERR "gpio_driver: Failed to create class\n");
        return PTR_ERR(gpio_class);
    }

    device_create(gpio_class, NULL, dev_number, NULL, DEVICE_NAME);

    timer_setup(&temp_timer, temp_timer_callback, 0);
    mod_timer(&temp_timer, jiffies + msecs_to_jiffies(2000));

    printk(KERN_INFO "gpio_driver: Module loaded\n");
    return 0;
}

static void __exit gpio_driver_exit(void) {
    del_timer_sync(&temp_timer);
    device_destroy(gpio_class, dev_number);
    class_destroy(gpio_class);
    cdev_del(&gpio_cdev);
    unregister_chrdev_region(dev_number, 1);
    printk(KERN_INFO "gpio_driver: Module unloaded\n");
}

module_init(gpio_driver_init);
module_exit(gpio_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Afreensha");
MODULE_DESCRIPTION("Virtual Temperature Sensor Driver with IOCTL and Timer");


