#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/interrupt.h>
#include<linux/init.h>
#include<linux/delay.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("AFREEN");
MODULE_DESCRIPTION("keyboard IRQ with threaded handler");
MODULE_VERSION("1.0");

#define KBD_IRQ 1
static int dev_id=1;

static irqreturn_t kb_top_handler(int irq,void *dev_id){
	pr_info("kbd_threaded:[TOP HALF] IRQ %d received\n",irq);
	return IRQ_WAKE_THREAD;
}

static irqreturn_t kb_thread_handler(int irq,void *dev_id){
	pr_info("kbd_threaded:[BOTTOM HALF] done processing\n");
	msleep(500);
	pr_info("kbd_threaded:[BOTTOM HALF] done processing\n");
	return IRQ_HANDLED;
}

static int __init kbd_irq_init(void){
	int ret;
	pr_info("kbd_threaded:loading module\n");
	ret=request_threaded_irq(KBD_IRQ, 
			         kb_top_handler,
				 kb_thread_handler,
				 IRQF_SHARED,
				 "kbd_threaded_irq",
				 (void *)&dev_id);

	if(ret){
		pr_err("kbd_threaded:failed to request IRQ %d\n",KBD_IRQ);
		return ret;
	}

	pr_info("kbd_threaded:IRQ %d registered successfully\n",KBD_IRQ);
	return 0;
}

static void __exit kbd_irq_exit(void){
	free_irq(KBD_IRQ,(void *)&dev_id);
	pr_info("kbd_threaded:IRQ %d freed,module unloaded\n",KBD_IRQ);
}

module_init(kbd_irq_init);
module_exit(kbd_irq_exit);


