#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/interrupt.h>
#include<linux/smp.h>
#include<linux/keyboard.h>
#include<linux/input.h>
#include<asm/io.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("AFREEN");
MODULE_DESCRIPTION("Add & SUb using keyboard interrupt and tasklet");

#define KBD_IRQ 1

static struct tasklet_struct my_tasklet;
static int ope=0;
static int a=5,b=2;

static void my_tasklet_func(unsigned long data){
	int c;
	if(ope==1){
		c=a+b;
		pr_info("tasklet:addition result=%d+%d=%d\n",a,b,c);
	}
	else if(ope==2){
		c=a-b;
		pr_info("tasklet:sub result=%d -%d=%d\n",a,b,c);
	}
	else{
		pr_info("tasklet: invalied operations\n");
	}
}

static irqreturn_t keyboard_irq_handler(int irq,void *dev_id){
	unsigned char scancode;
	scancode=inb(0x60); //read scancode from keyboard 
       
	if(scancode==0x1E){ // 'a' key
		ope=1;
		tasklet_schedule(&my_tasklet);
	}
	else if(scancode==0x1F){ // 's' key
        ope=2;
	tasklet_schedule(&my_tasklet);
	}
	return IRQ_HANDLED;
}

static int __init my_module_init(void){
	pr_info("loading keyboard irq tasklet module\n");

	if(request_irq(KBD_IRQ,keyboard_irq_handler,IRQF_SHARED, "kbd_tasklet", (void *)(keyboard_irq_handler))) {
        pr_err("Failed to register IRQ handler\n");
        return -1;
    }

    tasklet_init(&my_tasklet, my_tasklet_func, 0);

    pr_info("Module loaded. Press 'a' for add, 's' for subtract.\n");
    return 0;
}


static void __exit my_module_exit(void)
{
    free_irq(KBD_IRQ, (void *)(keyboard_irq_handler));
    tasklet_kill(&my_tasklet);
    pr_info("Keyboard tasklet module unloaded\n");
}

module_init(my_module_init);
module_exit(my_module_exit);


