#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/io.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/of.h>
#include <linux/of_platform.h>

#define DEVICE_NAME "custom_wdt"
#define CLASS_NAME  "custom"

// Hardware Register Offsets
#define PM_RSTC_OFFSET      0x1c
#define PM_WDOG_OFFSET      0x24
#define PM_PASSWORD         0x5A000000
#define PM_RSTC_WRCFG_FULL_RESET 0x00000020
#define WDOG_TIMEOUT        10

static void __iomem *wdog_base;
static dev_t dev_num;
static struct class *wdt_class;
static struct cdev wdt_cdev;

// --- Hardware Access Functions (Same Logic) ---

static void bcm2711_wdt_hw_start(void) {
    iowrite32(PM_PASSWORD | (WDOG_TIMEOUT * 16), wdog_base + PM_WDOG_OFFSET);
    iowrite32(PM_PASSWORD | PM_RSTC_WRCFG_FULL_RESET, wdog_base + PM_RSTC_OFFSET);
    pr_info("%s: started watchdog (Timeout: %d sec)\n", DEVICE_NAME, WDOG_TIMEOUT);
}

static void bcm2711_wdt_hw_stop(void) {
    iowrite32(PM_PASSWORD | 0, wdog_base + PM_RSTC_OFFSET);
    pr_info("%s: stopped watchdog\n", DEVICE_NAME);
}

static void bcm2711_wdt_hw_ping(void) {
    iowrite32(PM_PASSWORD | (WDOG_TIMEOUT * 16), wdog_base + PM_WDOG_OFFSET);
    pr_info("%s: watchdog pinged\n", DEVICE_NAME);
}

// --- File Operations (For Shell Script Control) ---

static ssize_t wdt_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos) {
    bcm2711_wdt_hw_ping();
    return count;
}

static int wdt_open(struct inode *inode, struct file *file) {
    bcm2711_wdt_hw_start();
    pr_info("%s: device opened\n", DEVICE_NAME);
    return 0;
}

static int wdt_release(struct inode *inode, struct file *file) {
    bcm2711_wdt_hw_stop();
    pr_info("%s: device closed\n", DEVICE_NAME);
    return 0;
}

static const struct file_operations wdt_fops = {
    .owner          = THIS_MODULE,
    .write          = wdt_write,
    .open           = wdt_open,
    .release        = wdt_release,
};

// --- Platform Driver Functions ---

// 1. OF Match Table: Links the driver to the DTS node
static const struct of_device_id custom_wdt_of_match[] = {
    { .compatible = "snigdha,bcm2711-custom-wdt", },
    { /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, custom_wdt_of_match);

static int custom_wdt_probe(struct platform_device *pdev)
{
    struct resource *res;
    int ret;

    // A. GET RESOURCE AND IOREMAP
    // platform_get_resource uses the 'reg' property from the DTS
    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (!res) {
        dev_err(&pdev->dev, "Missing memory resource in Device Tree\n");
        return -EINVAL;
    }

    // devm_ioremap_resource claims the memory region and maps it.
    // This is the CRITICAL STEP that replaces your manual ioremap/request_mem_region.
    wdog_base = devm_ioremap_resource(&pdev->dev, res);
    if (IS_ERR(wdog_base)) {
        // If this fails, the resource is BCM_BUSY/EFAULT, returning the error code
        dev_err(&pdev->dev, "Failed to remap resource, check DTS conflicts.\n");
        return PTR_ERR(wdog_base);
    }
    
    // B. REGISTER CHAR DEVICE
    ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    if (ret < 0) {
        dev_err(&pdev->dev, "Failed to allocate char dev region.\n");
        return ret;
    }

    cdev_init(&wdt_cdev, &wdt_fops);
    cdev_add(&wdt_cdev, dev_num, 1);

    wdt_class = class_create(CLASS_NAME);
    if (IS_ERR(wdt_class)) {
        ret = PTR_ERR(wdt_class);
        goto err_unregister_chrdev;
    }

    device_create(wdt_class, NULL, dev_num, NULL, DEVICE_NAME);

    dev_info(&pdev->dev, "Platform driver loaded (major %d).\n", MAJOR(dev_num));
    return 0;

err_unregister_chrdev:
    unregister_chrdev_region(dev_num, 1);
    return ret;
}

static void custom_wdt_remove(struct platform_device *pdev)
{
    // Unregistration sequence (cleanup)
    device_destroy(wdt_class, dev_num);
    class_destroy(wdt_class);
    cdev_del(&wdt_cdev);
    unregister_chrdev_region(dev_num, 1);
    
    // devm_ioremap_resource automatically handles iounmap and release_mem_region

    dev_info(&pdev->dev, "Platform driver unloaded.\n");
}

// 3. Platform Driver Structure
static struct platform_driver custom_wdt_driver = {
    .probe          = custom_wdt_probe,
    .remove         = custom_wdt_remove,
    .driver     = {
        .name       = DEVICE_NAME,
        .owner      = THIS_MODULE,
        .of_match_table = custom_wdt_of_match,
    },
};

// 4. Register the driver
module_platform_driver(custom_wdt_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("afreen");
MODULE_DESCRIPTION("Custom BCM2711 Watchdog Platform Driver");
