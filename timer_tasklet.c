#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/smp.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Afreensha");
MODULE_DESCRIPTION("Timer and Tasklet Calculator Example");
MODULE_VERSION("1.0");

static struct tasklet_struct my_tasklet;
static struct timer_list my_timer;

// Calculator inputs
static int x = 8;
static int y = 4;
static char op = '+';  
static int result; 


static void my_tasklet_func(struct tasklet_struct *tasklet)
{
    pr_info("tasklet: executing on CPU %u\n", smp_processor_id());

    switch (op) {
        case '+': result = x + y;
		  break;
        case '-': result = x - y;
		  break;
        case '*': result = x * y;
		  break;
        case '/': result = (y != 0) ? x / y : 0;
		  break;
        default:  result = 0;
    }

    pr_info("tasklet: %d %c %d = %d\n", x, op, y, result);
}


static void my_timer_handler(struct timer_list *t)
{
    pr_info("timer: firing, simulating interrupt -> scheduling tasklet\n");

    tasklet_schedule(&my_tasklet);

    mod_timer(&my_timer, jiffies + msecs_to_jiffies(2000));
}


static int __init tasklet_init_module(void)
{
    pr_info("tasklet module loaded\n");

    tasklet_setup(&my_tasklet, my_tasklet_func);

    timer_setup(&my_timer, my_timer_handler, 0);
    mod_timer(&my_timer, jiffies + msecs_to_jiffies(2000));

    return 0;
}


static void __exit tasklet_cleanup_module(void)
{
    pr_info("tasklet module unloaded\n");

    del_timer_sync(&my_timer);
    tasklet_kill(&my_tasklet);
}


module_init(tasklet_init_module);
module_exit(tasklet_cleanup_module);

