#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/slab.h>
#include<linux/vmalloc.h>
#include<linux/string.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Afreen");
MODULE_DESCRIPTION("test mdoule using both kmalloc and vmalloc");

#define KMALLOC_SIZE 1024
#define VMALLOC_SIZE (1024*1024)

static char *kmalloc_buf;
static char *vmalloc_buf;

static int __init mem_test_init(void)
{
	pr_info("module loaded\n");

	kmalloc_buf=kmalloc(KMALLOC_SIZE,GFP_KERNEL); //kmalloc allocation
	if(!kmalloc_buf){
		pr_err("kmalloc failed\n");
		return -ENOMEM;
	}

	strcpy(kmalloc_buf,"this is kmalloc memory!");
	pr_info("kmalloc says:%s\n",kmalloc_buf);

vmalloc_buf=vmalloc(VMALLOC_SIZE); //vmallocallocation
if(!vmalloc_buf){
	pr_err("vmalloc failed\n");
	kfree(kmalloc_buf);
	return -ENOMEM;

}

strcpy(vmalloc_buf,"this is vmalloc memory!");
pr_info("vmalloc says:%s\n",vmalloc_buf);

return 0;
}

static void __exit mem_test_exit(void){
	if(kmalloc_buf){
		kfree(kmalloc_buf);
		pr_info("kmalloc memory freed\n");
	}

	if(vmalloc_buf){
		vfree(vmalloc_buf);
		pr_info("vmalloc memory freed\n");
	}
	pr_info("module unloaded\n");
}

module_init(mem_test_init);
module_exit(mem_test_exit);



