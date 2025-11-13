#include <linux/module.h>
#include <linux/watchdog.h>
#include <linux/io.h>

#define PM_BASE          0xFE100000
#define PM_RSTC_OFFSET   0x1C
#define PM_WDOG_OFFSET   0x24
#define PM_PASSWORD      (0x5A << 24)
#define PM_RSTC_WRCFG_MASK   0x30
#define PM_RSTC_WRCFG_FULL_RESET 0x20

#define WDT_DEFAULT_TIMEOUT 10  // seconds

static void __iomem *pm_base;
static int timeout = WDT_DEFAULT_TIMEOUT;

static int bcm2711_wdt_start(struct watchdog_device *wdd)
{
    u32 ticks = timeout * 16;  // 1 tick = ~1/16s
    writel(PM_PASSWORD | (ticks & 0xFFFF), pm_base + PM_WDOG_OFFSET);
    writel(PM_PASSWORD | PM_RSTC_WRCFG_FULL_RESET, pm_base + PM_RSTC_OFFSET);
    pr_info("BCM2711 WDT started (%d s)\n", timeout);
    return 0;
}

static int bcm2711_wdt_stop(struct watchdog_device *wdd)
{
    writel(PM_PASSWORD, pm_base + PM_WDOG_OFFSET);
    writel(PM_PASSWORD, pm_base + PM_RSTC_OFFSET);
    pr_info("BCM2711 WDT stopped\n");
    return 0;
}

static int bcm2711_wdt_ping(struct watchdog_device *wdd)
{
    u32 ticks = timeout * 16;
    writel(PM_PASSWORD | (ticks & 0xFFFF), pm_base + PM_WDOG_OFFSET);
    pr_debug("BCM2711 WDT ping\n");
    return 0;
}

static const struct watchdog_ops bcm2711_wdt_ops = {
    .owner = THIS_MODULE,
    .start = bcm2711_wdt_start,
    .stop  = bcm2711_wdt_stop,
    .ping  = bcm2711_wdt_ping,
};

static struct watchdog_device bcm2711_wdt_dev = {
    .info = &(struct watchdog_info){
        .options = WDIOF_SETTIMEOUT | WDIOF_KEEPALIVEPING,
        .identity = "BCM2711 PM Watchdog",
    },
    .ops = &bcm2711_wdt_ops,
    .timeout = WDT_DEFAULT_TIMEOUT,
};

static int __init bcm2711_wdt_init(void)
{
    pm_base = ioremap(PM_BASE, 0x100);
    watchdog_register_device(&bcm2711_wdt_dev);
    pr_info("BCM2711 Watchdog driver loaded\n");
    return 0;
}

static void __exit bcm2711_wdt_exit(void)
{
    bcm2711_wdt_stop(&bcm2711_wdt_dev);
    watchdog_unregister_device(&bcm2711_wdt_dev);
    iounmap(pm_base);
    pr_info("BCM2711 Watchdog driver unloaded\n");
}

module_init(bcm2711_wdt_init);
module_exit(bcm2711_wdt_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("BCM2711 Power Management Watchdog Driver");

