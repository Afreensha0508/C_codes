#include <linux/module.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/kobject.h>
#include <linux/device.h>   // devm_*

static void *km_ptr;
static void *vm_ptr;
static struct kobject *leaky_kobj;

static int __init leak_demo_fixed_init(void)
{
    pr_info("leak_demo_fixed: loading (no leaks)\n");

    km_ptr = kmalloc(4096, GFP_KERNEL);
    if (!km_ptr)
	    return -ENOMEM;

    vm_ptr = vmalloc(8192);
    if (!vm_ptr)
    {
	    kfree(km_ptr);
	    return -ENOMEM;
    }

    leaky_kobj = kobject_create_and_add("leaky_kobj_fixed", kernel_kobj);
    if (!leaky_kobj)
    {
	    vfree(vm_ptr);
	    kfree(km_ptr);
	    return -ENOMEM;
    }

    pr_info("leak_demo_fixed: allocated resources successfully\n");
    return 0;
}

static void __exit leak_demo_fixed_exit(void)
{
    if (leaky_kobj)
	    kobject_put(leaky_kobj);
    if (vm_ptr) 
	    vfree(vm_ptr);
    if (km_ptr) 
	    kfree(km_ptr);
    pr_info("leak_demo_fixed: cleaned up without leaks\n");
}

module_init(leak_demo_fixed_init);
module_exit(leak_demo_fixed_exit);


MODULE_AUTHOR("afreen");
MODULE_DESCRIPTION("Intentional memory leak demo for kmemleak");
MODULE_LICENSE("GPL");

