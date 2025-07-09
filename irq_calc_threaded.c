#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("AFREEN");
MODULE_DESCRIPTION("Calculator using Keyboard IRQ with Threaded Handler");
MODULE_VERSION("1.0");

#define KBD_IRQ 1
static int dev_id = 42;

// Hardcoded operands and operator
static int a = 10;
static int b = 5;
static char op = '*';  // You can change this to '-', '*', '/'

// Top half: Acknowledge IRQ
static irqreturn_t kb_top_handler(int irq, void *dev)
{
    pr_info("calc_threaded:[TOP HALF] IRQ %d received\n", irq);
    return IRQ_WAKE_THREAD;
}

// Bottom half: Perform calculation
static irqreturn_t kb_thread_handler(int irq, void *dev)
{
    int result = 0;

    pr_info("calc_threaded:[BOTTOM HALF] Performing calculation\n");

    switch (op) {
        case '+': result = a + b; break;
        case '-': result = a - b; break;
        case '*': result = a * b; break;
        case '/': 
            if (b != 0)
                result = a / b;
            else
                pr_warn("calc_threaded: Division by zero!\n");
            break;
        default:
            pr_warn("calc_threaded: Unsupported operator '%c'\n", op);
            return IRQ_HANDLED;
    }

    pr_info("calc_threaded: %d %c %d = %d\n", a, op, b, result);
    msleep(500); // Simulate time-consuming work
    pr_info("calc_threaded:[BOTTOM HALF] done\n");

    return IRQ_HANDLED;
}

static int __init calc_threaded_init(void)
{
    int ret;

    pr_info("calc_threaded: Loading module\n");

    ret = request_threaded_irq(KBD_IRQ,
                               kb_top_handler,
                               kb_thread_handler,
                               IRQF_SHARED,
                               "calc_threaded_irq",
                               (void *)&dev_id);

    if (ret) {
        pr_err("calc_threaded: Failed to request IRQ %d\n", KBD_IRQ);
        return ret;
    }

    pr_info("calc_threaded: IRQ %d registered successfully\n", KBD_IRQ);
    return 0;
}

static void __exit calc_threaded_exit(void)
{
    free_irq(KBD_IRQ, (void *)&dev_id);
    pr_info("calc_threaded: IRQ %d freed, module unloaded\n", KBD_IRQ);
}

module_init(calc_threaded_init);
module_exit(calc_threaded_exit);

