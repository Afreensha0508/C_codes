#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/delay.h>

#define UART1_BASE 0xFE201400
#define UART2_BASE 0xFE201800
#define UART_REG_SIZE 0x90

#define UART_DR  0x00
#define UART_FR  0x18

static void __iomem *uart1_base;
static void __iomem *uart2_base;

static int __init uart_comm_init(void)
{
    u32 fr;
    char ch = 'A';

    pr_info("UART communication kernel module loaded\n");

    // Map UART physical addresses to virtual addresses
    uart1_base = ioremap(UART1_BASE, UART_REG_SIZE);
    uart2_base = ioremap(UART2_BASE, UART_REG_SIZE);

    if (!uart1_base || !uart2_base) {
        pr_err("Failed to ioremap UART bases\n");
        return -ENOMEM;
    }

    // Wait until UART1 TX FIFO is not full
    do {
        fr = readl(uart1_base + UART_FR);
    } while (fr & (1 << 5)); // TXFF

    // Send character from UART1
    writel(ch, uart1_base + UART_DR);
    pr_info("UART1 sent: %c\n", ch);

    // Wait until UART2 RX FIFO is not empty
    do {
        fr = readl(uart2_base + UART_FR);
    } while (fr & (1 << 4)); // RXFE

    // Read character from UART2
    ch = readl(uart2_base + UART_DR) & 0xFF;
    pr_info("UART2 received: %c\n", ch);

    return 0;
}

static void __exit uart_comm_exit(void)
{
    iounmap(uart1_base);
    iounmap(uart2_base);
    pr_info("UART communication kernel module unloaded\n");
}

module_init(uart_comm_init);
module_exit(uart_comm_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("afreen");
MODULE_DESCRIPTION("Communication between two UARTs using ioremap");

