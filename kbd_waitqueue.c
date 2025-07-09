#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/interrupt.h>
#include<linux/wait.h>
#include<linux/fs.h>
#include<linux/uaccess.h>
#include<linux/cdev.h>
#include<linux/device.h>

#define DEVICE_NAME "kbd_waitqueue"
#define KEYBOARD_IRQ 1

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Afreen");
MODULE_DESCRIPTION("keyboard IRQ with waitqueue");

static dev_t dev;
static struct cdev my_cdev;
static struct class *my_class;

static DECLARE_WAIT_QUEUE_HEAD(kbd_waitq);
static int key_pressed=0;

static irqreturn_t keyboard_irq_handler(int irq,void *dev_id){
	pr_info("kbd_wait:keyboard irq received\n");
	key_pressed=1;
	wake_up_interruptible(&kbd_waitq);
	return 	IRQ_HANDLED;
}

static ssize_t kbd_read(struct file *file,char __user *buf,size_t len, loff_t *off){
	char msg[]="key pressed!\n";
	pr_info("kbd_wait: read called,waiting for the key press\n");

	wait_event_interruptible(kbd_waitq,key_pressed!=0);

	key_pressed=0;
	if(copy_to_user(buf,msg,sizeof(msg)))
		return -EFAULT;

	return sizeof(msg);
}

static struct file_operations fops={
	.owner=THIS_MODULE,
	.read=kbd_read,
};

static int __init kbd_wait_init(void){
	if(alloc_chrdev_region(&dev,0,1,DEVICE_NAME)<0)
		return -1;

	cdev_init(&my_cdev,&fops);
	if(cdev_add(&my_cdev,dev,1)==-1)
		goto unregister_dev;

	my_class=class_create(DEVICE_NAME);
	if(IS_ERR(my_class))
		goto del_cdev;

	device_create(my_class,NULL,dev,NULL,DEVICE_NAME);

	if(request_irq(KEYBOARD_IRQ,keyboard_irq_handler,IRQF_SHARED,DEVICE_NAME,(void *)(keyboard_irq_handler)))	
		goto destroy_dev;

	pr_info("kd_wait:module loaded successfully\n");
	return 0;
	
destroy_dev:
	device_destroy(my_class,dev);
	class_destroy(my_class);
del_cdev:
	cdev_del(&my_cdev);
unregister_dev:
	unregister_chrdev_region(dev,1);
	return -1;
}

static void __exit kbd_wait_exit(void){
	 free_irq(KEYBOARD_IRQ, (void *)(keyboard_irq_handler));
    device_destroy(my_class, dev);
    class_destroy(my_class);
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev, 1);
    pr_info("kbd_wait: Module unloaded\n");
}

module_init(kbd_wait_init);
module_exit(kbd_wait_exit);




