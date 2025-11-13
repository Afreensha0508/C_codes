#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DRIVER_NAME "my_uart_driver"

static int uart_probe(struct platform_device *pdev)
{
    pr_info("UART platform driver probed\n");
    return 0;
}

static int uart_remove(struct platform_device *pdev)
{
    pr_info("UART platform driver removed\n");
    return 0;
}

static const struct of_device_id uart_of_match[] = {
    { .compatible = "my_uart_driver" },
    {},
};
MODULE_DEVICE_TABLE(of, uart_of_match);

static struct platform_driver uart_platform_driver = {
    .driver = {
        .name = DRIVER_NAME,
        .of_match_table = uart_of_match,
    },
    .probe = uart_probe,
    .remove = uart_remove,
};

module_platform_driver(uart_platform_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("afreen");
MODULE_DESCRIPTION("UART Platform Driver Example");

