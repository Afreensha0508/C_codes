#include <linux/module.h>
#include <linux/slab.h>     // kmalloc, kfree
#include <linux/vmalloc.h>  // vmalloc, vfree
#include <linux/kobject.h>  // kobject
#include <linux/delay.h>

static void *km_ptr;
static void *vm_ptr;
static struct kobject *leaky_kobj;

static int __init leak_demo_init(void)
{
    pr_info("leak_demo: loading and intentionally leaking memory\n");

    // 1) kmalloc leak
    km_ptr = kmalloc(4096, GFP_KERNEL);
    if (!km_ptr) {
        pr_err("leak_demo: kmalloc failed\n");
        return -ENOMEM;
    }
    // (no kfree on purpose)

    // 2) vmalloc leak
    vm_ptr = vmalloc(8192);
    if (!vm_ptr) {
        pr_err("leak_demo: vmalloc failed\n");
        // still leak km_ptr so we can test multiple objects
        return -ENOMEM;
    }
    // (no vfree on purpose)

    // 3) kobject ref leak (increment ref, never put)
    leaky_kobj = kobject_create_and_add("leaky_kobj", kernel_kobj);
    if (!leaky_kobj) {
        pr_err("leak_demo: kobject_create_and_add failed\n");
        return -ENOMEM;
    }
    // Normally you would kobject_put(leaky_kobj) on module exit, but we won't.

    pr_info("leak_demo: intentionally leaked kmalloc, vmalloc, and kobject\n");
    return 0;
}

static void __exit leak_demo_exit(void)
{
    pr_info("leak_demo: exiting (still leaked by design)\n");
    // No frees here to demonstrate detection.
}

module_init(leak_demo_init);
module_exit(leak_demo_exit);

MODULE_AUTHOR("afreen");
MODULE_DESCRIPTION("Intentional memory leak demo for kmemleak");
MODULE_LICENSE("GPL");

