#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/interrupt.h>
#include<linux/timer.h>
#include<linux/workqueue.h>
#include<linux/kthread.h>
#include<linux/delay.h>
#include<linux/sched.h>
#include<linux/wait.h>



MODULE_LICENSE("GPL");
MODULE_AUTHOR("Afreen");
MODULE_DESCRIPTION("IRQ TIMER TASKET WORKQUEUE KTHREAD");

#define KEYBOARD_IRQ 1

static struct timer_list my_timer;
static struct tasklet_struct tasklet_add;
static struct tasklet_struct tasklet_div;
static struct workqueue_struct *my_wq;
static struct work_struct work_mul;
static struct work_struct work_mod;

static struct task_struct *sub_kthread;
static DECLARE_WAIT_QUEUE_HEAD(kthread_wq);
static int kthread_flag=0;

int a=15,b=5; //variable shared 
int add=0,sub=0,mul=1,div=1,mod=0;

static void timer_tasklet_func(unsigned long data){
	add=a+b;
	pr_info("timer tasklet:%d\n",add);
}
static void timer_work_func(struct work_struct *work){
	mul=a*b;
	pr_info("timer workqueue :%d\n",mul);
}

static void timer_handler(struct timer_list *t){
	pr_info("timer top half triggered\n");

	tasklet_schedule(&tasklet_add);
pr_info(">>> [TIMER] Tasklet scheduled <<<\n");
	queue_work(my_wq,&work_mul);

	 pr_info(">>> [TIMER] Workqueue scheduled <<<\n");

	mod_timer(&my_timer,jiffies+msecs_to_jiffies(5000));
}


static void keyboard_tasklet_func(unsigned long data){
	div=a/b;
	pr_info("keyboard tasklet :%d\n",div);
}

static void keyboard_work_func(struct work_struct *work){
	mod=a%b;
	pr_info("keyboard workqueue :%d\n",mod);
}

static irqreturn_t keyboard_irq_handler(int irq,void *Dev_id){
	pr_info("keyboard irq top half\n");

	tasklet_schedule(&tasklet_div);
	queue_work(my_wq,&work_mod);

	kthread_flag=1; // kthread for sub fun wakeup
	wake_up_interruptible(&kthread_wq);

	return IRQ_HANDLED;
}

static int subtraction_thread_fn(void *data){
	pr_info("Sub kernel thread started\n");

	while(!kthread_should_stop()){
		wait_event_interruptible(kthread_wq,kthread_flag!=0);

		if(kthread_should_stop())
			break;

		sub=a-b;
		pr_info("kthread:%d\n",sub);

		kthread_flag=0;
	}
	return 0;
}



static int __init irq_kthread_demo_init(void)
{
    int ret;

    pr_info("Loading Module with IRQ + KThread\n");

   
    timer_setup(&my_timer, timer_handler, 0);
    mod_timer(&my_timer, jiffies + msecs_to_jiffies(5000));

   
    tasklet_init(&tasklet_add, timer_tasklet_func, 0);
    tasklet_init(&tasklet_div, keyboard_tasklet_func, 0);

   
    my_wq = alloc_workqueue("irq_wq", WQ_UNBOUND, 0);
    INIT_WORK(&work_mul, timer_work_func);
    INIT_WORK(&work_mod, keyboard_work_func);

   
    sub_kthread = kthread_run(subtraction_thread_fn, NULL, "sub_kthread");
    if (IS_ERR(sub_kthread)) {
        pr_err("Failed to create subtraction kthread\n");
        destroy_workqueue(my_wq);
        del_timer_sync(&my_timer);
        return PTR_ERR(sub_kthread);
    }


    ret = request_irq(KEYBOARD_IRQ, keyboard_irq_handler, IRQF_SHARED,
                      "keyboard_irq", (void *)&keyboard_irq_handler);
    if (ret) {
        pr_err("Failed to request keyboard IRQ\n");
        kthread_stop(sub_kthread);
        destroy_workqueue(my_wq);
        del_timer_sync(&my_timer);
        return ret;
    }

    return 0;
}

static void __exit irq_kthread_demo_exit(void)
{
    free_irq(KEYBOARD_IRQ, (void *)&keyboard_irq_handler);

    if (sub_kthread)
        kthread_stop(sub_kthread);

    tasklet_kill(&tasklet_add);
    tasklet_kill(&tasklet_div);

    flush_workqueue(my_wq);
    destroy_workqueue(my_wq);

    del_timer_sync(&my_timer);

    pr_info("Module Unloaded. Final Results:\n");
    pr_info("Addition (tasklet): %d\n", add);
    pr_info("Subtraction (kthread): %d\n", sub);
    pr_info("Multiplication (wq): %d\n", mul);
    pr_info("Division (tasklet): %d\n", div);
    pr_info("Modulo (wq): %d\n", mod);
}

module_init(irq_kthread_demo_init);
module_exit(irq_kthread_demo_exit);

