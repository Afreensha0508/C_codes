#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/cdev.h>
#include <linux/device.h>

#define DEVICE_NAME "custom_wdt"
#define CLASS_NAME  "custom"

// BCM2835 watchdog registers
#define BCM2835_WDOG_BASE   0xFE100000
#define PM_RSTC_OFFSET      0x1c
#define PM_WDOG_OFFSET      0x24
#define PM_PASSWORD         0x5A000000
#define PM_RSTC_WRCFG_FULL_RESET 0x00000020

// Custom IOCTL commands
#define WDT_MAGIC       'W'
#define WDT_START       _IO(WDT_MAGIC, 0)
#define WDT_STOP        _IO(WDT_MAGIC, 1)
#define WDT_PING        _IO(WDT_MAGIC, 2)

#define WDOG_TIMEOUT    10

static void __iomem *wdog_base;
static int major;
static struct class *wdt_class;
static struct cdev wdt_cdev;

static void bcm2835_wdt_hw_start(void)
{
    iowrite32(PM_PASSWORD | (WDOG_TIMEOUT * 16), wdog_base + PM_WDOG_OFFSET);
    iowrite32(PM_PASSWORD | PM_RSTC_WRCFG_FULL_RESET, wdog_base + PM_RSTC_OFFSET);
    pr_info("custom_wdt: started watchdog\n");
}

static void bcm2835_wdt_hw_stop(void)
{
    iowrite32(PM_PASSWORD | 0, wdog_base + PM_RSTC_OFFSET);
    pr_info("custom_wdt: stopped watchdog\n");
}

static void bcm2835_wdt_hw_ping(void)
{
    iowrite32(PM_PASSWORD | (WDOG_TIMEOUT * 16), wdog_base + PM_WDOG_OFFSET);
    pr_info("custom_wdt: watchdog pinged\n");
}

// ------------------ File operations ------------------
static long wdt_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    switch (cmd) {
    case WDT_START:
        bcm2835_wdt_hw_start();
        break;
    case WDT_STOP:
        bcm2835_wdt_hw_stop();
        break;
    case WDT_PING:
        bcm2835_wdt_hw_ping();
        break;
    default:
        return -EINVAL;
    }
    return 0;
}

static int wdt_open(struct inode *inode, struct file *file)
{
    pr_info("custom_wdt: device opened\n");
    return 0;
}

static int wdt_release(struct inode *inode, struct file *file)
{
    pr_info("custom_wdt: device closed\n");
    return 0;
}

static const struct file_operations wdt_fops = {
    .owner          = THIS_MODULE,
    .unlocked_ioctl = wdt_ioctl,
    .open           = wdt_open,
    .release        = wdt_release,
};

// ------------------ Module Init/Exit ------------------
static int __init wdt_init(void)
{
    dev_t dev;
    int ret;

    wdog_base = ioremap(BCM2835_WDOG_BASE, 0x100);
    if (!wdog_base)
        return -ENOMEM;

    ret = alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME);
    if (ret < 0) {
        iounmap(wdog_base);
        return ret;
    }

    major = MAJOR(dev);
    cdev_init(&wdt_cdev, &wdt_fops);
    cdev_add(&wdt_cdev, dev, 1);

    wdt_class = class_create(THIS_MODULE, CLASS_NAME);
    device_create(wdt_class, NULL, dev, NULL, DEVICE_NAME);

    pr_info("custom_wdt: module loaded (major %d)\n", major);
    return 0;
}

static void __exit wdt_exit(void)
{
    dev_t dev = MKDEV(major, 0);
    device_destroy(wdt_class, dev);
    class_destroy(wdt_class);
    cdev_del(&wdt_cdev);
    unregister_chrdev_region(dev, 1);
    iounmap(wdog_base);
    pr_info("custom_wdt: module unloaded\n");
}

module_init(wdt_init);
module_exit(wdt_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("afreen");
MODULE_DESCRIPTION("Custom BCM2835 Watchdog with IOCTL control");

