#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/rtc.h>
#include <linux/io.h>

#define DRV_NAME "my_rtc_driver"

/* Register offsets */
#define SECONDS_REG   0x00
#define MINUTES_REG   0x04
#define HOURS_REG     0x08
#define DAYS_REG      0x0C
#define MONTHS_REG    0x10
#define YEARS_REG     0x14
#define CTRL_REG      0x40

struct my_rtc_dev {
    struct rtc_device *rtc;
    void __iomem *base;
};

/* BCD <-> BIN helpers */
static inline unsigned int bcd2bin(unsigned int val)
{
    return ((val >> 4) * 10) + (val & 0xF);
}

static inline unsigned int bin2bcd(unsigned int val)
{
    return ((val / 10) << 4) | (val % 10);
}

/* Read RTC time from registers */
static int my_rtc_read_time(struct device *dev, struct rtc_time *tm)
{
    struct my_rtc_dev *myrtc = dev_get_drvdata(dev);

    tm->tm_sec  = bcd2bin(readl(myrtc->base + SECONDS_REG));
    tm->tm_min  = bcd2bin(readl(myrtc->base + MINUTES_REG));
    tm->tm_hour = bcd2bin(readl(myrtc->base + HOURS_REG));
    tm->tm_mday = bcd2bin(readl(myrtc->base + DAYS_REG));
    tm->tm_mon  = bcd2bin(readl(myrtc->base + MONTHS_REG)) - 1; // RTC = 1–12, tm = 0–11
    tm->tm_year = bcd2bin(readl(myrtc->base + YEARS_REG)) + 100; // RTC = YY, tm = years since 1900

    pr_info(DRV_NAME ": Read time: %d-%02d-%02d %02d:%02d:%02d\n",
            tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
            tm->tm_hour, tm->tm_min, tm->tm_sec);

    return 0;
}

/* Write RTC time to registers */
static int my_rtc_set_time(struct device *dev, struct rtc_time *tm)
{
    struct my_rtc_dev *myrtc = dev_get_drvdata(dev);

    pr_info(DRV_NAME ": Setting time: %d-%02d-%02d %02d:%02d:%02d\n",
            tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
            tm->tm_hour, tm->tm_min, tm->tm_sec);

    writel(bin2bcd(tm->tm_sec),  myrtc->base + SECONDS_REG);
    writel(bin2bcd(tm->tm_min),  myrtc->base + MINUTES_REG);
    writel(bin2bcd(tm->tm_hour), myrtc->base + HOURS_REG);
    writel(bin2bcd(tm->tm_mday), myrtc->base + DAYS_REG);
    writel(bin2bcd(tm->tm_mon + 1), myrtc->base + MONTHS_REG);
    writel(bin2bcd(tm->tm_year % 100), myrtc->base + YEARS_REG);

    return 0;
}

static const struct rtc_class_ops my_rtc_ops = {
    .read_time  = my_rtc_read_time,
    .set_time   = my_rtc_set_time,
};

static int my_rtc_probe(struct platform_device *pdev)
{
    struct my_rtc_dev *myrtc;
    struct resource *res;

    pr_info(DRV_NAME ": probe called\n");

    myrtc = devm_kzalloc(&pdev->dev, sizeof(*myrtc), GFP_KERNEL);
    if (!myrtc)
        return -ENOMEM;

    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    myrtc->base = devm_ioremap_resource(&pdev->dev, res);
    if (IS_ERR(myrtc->base))
        return PTR_ERR(myrtc->base);

    myrtc->rtc = devm_rtc_device_register(&pdev->dev, DRV_NAME,
                                          &my_rtc_ops, THIS_MODULE);
    if (IS_ERR(myrtc->rtc)) {
        dev_err(&pdev->dev, "Failed to register RTC\n");
        return PTR_ERR(myrtc->rtc);
    }

    platform_set_drvdata(pdev, myrtc);

    /* Enable RTC in control register (bit 0) */
    writel(0x1, myrtc->base + CTRL_REG);

    pr_info(DRV_NAME ": RTC registered successfully\n");
    return 0;
}

static int my_rtc_remove(struct platform_device *pdev)
{
    pr_info(DRV_NAME ": remove called\n");
    return 0;
}

static const struct of_device_id my_rtc_dt_ids[] = {
    { .compatible = "ti,am3352-rtc" },
    { /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, my_rtc_dt_ids);

static struct platform_driver my_rtc_driver = {
    .driver = {
        .name = DRV_NAME,
        .of_match_table = my_rtc_dt_ids,
    },
    .probe  = my_rtc_probe,
    .remove = my_rtc_remove,
};

module_platform_driver(my_rtc_driver);

MODULE_AUTHOR("afreen");
MODULE_DESCRIPTION("Custom RTC Platform Driver with AM335x Registers");
MODULE_LICENSE("GPL");

