// File: mouse_irq_driver.c
#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>

#define MOUSE_IRQ 12      // PS/2 mouse IRQ on x86

static int irq = MOUSE_IRQ;
module_param(irq, int, 0444);
MODULE_PARM_DESC(irq, "IRQ number for the mouse");

static irqreturn_t mouse_irq_handler(int irq, void *dev_id)
{
    printk(KERN_INFO "Mouse IRQ handler called! IRQ=%d\n", irq);
    return IRQ_HANDLED;
}

static int __init mouse_irq_init(void)
{
    int ret;

    printk(KERN_INFO "Registering Mouse IRQ handler for IRQ %d\n", irq);

    // Request IRQ 12
    ret = request_irq(irq, mouse_irq_handler,
                      IRQF_SHARED, "mouse_irq_handler", (void *)(mouse_irq_handler));

    if (ret) {
        printk(KERN_ERR "Failed to register IRQ %d\n", irq);
        return ret;
    }

    printk(KERN_INFO "Mouse IRQ handler registered successfully.\n");
    return 0;
}

static void __exit mouse_irq_exit(void)
{
    free_irq(irq, (void *)(mouse_irq_handler));
    printk(KERN_INFO "Mouse IRQ handler unregistered.\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("TechDhaba - Prashant");
MODULE_DESCRIPTION("Simple IRQ handler for PS/2 Mouse (IRQ 12)");
MODULE_VERSION("1.0");

module_init(mouse_irq_init);
module_exit(mouse_irq_exit);

