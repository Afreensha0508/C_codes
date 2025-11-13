#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/timer.h>

#define PMIC_NAME "my_pmic"

// Virtual PMIC registers
enum {
    REG_VCORE   = 0x01,
    REG_VIO    = 0x02,
    REG_VDDS   = 0x03,
    REG_ENABLE = 0x04,
    REG_STATUS = 0x05,
    REG_MAX
};

// Simulated registers
static u8 pmic_regs[REG_MAX];
static DEFINE_MUTEX(pmic_lock);

// Workqueue and timer for fake interrupts
static struct workqueue_struct *pmic_wq;
static struct work_struct pg_work;

// Conversion helpers
static u16 reg_to_mv(u8 reg_val)
{
    return reg_val * 10; // 10 mV per LSB
}

static u8 mv_to_reg(u16 mv)
{
    if (mv > 2550)
        mv = 2550;
    return mv / 10;
}

// Fake “power-good” handler
static void pg_work_func(struct work_struct *work)
{
    pr_info("%s: Power-good interrupt fired\n", PMIC_NAME);
}

// Regulator APIs
static void pmic_enable(u8 rail)
{
    mutex_lock(&pmic_lock);
    pmic_regs[REG_ENABLE] |= (1 << rail);
    pr_info("%s: Rail %d enabled\n", PMIC_NAME, rail);
    mutex_unlock(&pmic_lock);

    // Schedule fake power-good interrupt after delay
    queue_work(pmic_wq, &pg_work);
}

static void pmic_disable(u8 rail)
{
    mutex_lock(&pmic_lock);
    pmic_regs[REG_ENABLE] &= ~(1 << rail);
    pr_info("%s: Rail %d disabled\n", PMIC_NAME, rail);
    mutex_unlock(&pmic_lock);
}

static void pmic_set_voltage(u8 rail, u16 mv)
{
    mutex_lock(&pmic_lock);
    switch (rail) {
        case 0: pmic_regs[REG_VCORE] = mv_to_reg(mv); break;
        case 1: pmic_regs[REG_VIO]   = mv_to_reg(mv); break;
        case 2: pmic_regs[REG_VDDS]  = mv_to_reg(mv); break;
    }
    pr_info("%s: Rail %d set voltage %d mV\n", PMIC_NAME, rail, mv);
    mutex_unlock(&pmic_lock);
}

static u16 pmic_get_voltage(u8 rail)
{
    u16 mv = 0;
    mutex_lock(&pmic_lock);
    switch (rail) {
        case 0: mv = reg_to_mv(pmic_regs[REG_VCORE]); break;
        case 1: mv = reg_to_mv(pmic_regs[REG_VIO]);   break;
        case 2: mv = reg_to_mv(pmic_regs[REG_VDDS]);  break;
    }
    mutex_unlock(&pmic_lock);
    return mv;
}

// Power sequencing
static void pmic_power_on_sequence(void)
{
    pr_info("%s: Power-on sequence start\n", PMIC_NAME);

    // Enable VCORE first
    pmic_set_voltage(0, 1000);
    pmic_enable(0);
    msleep(50);

    // Then VIO
    pmic_set_voltage(1, 1800);
    pmic_enable(1);
    msleep(50);

    // Then VDDS
    pmic_set_voltage(2, 3300);
    pmic_enable(2);
    msleep(50);

    pr_info("%s: Power-on sequence completed\n", PMIC_NAME);
}

static void pmic_power_off_sequence(void)
{
    pr_info("%s: Power-off sequence start\n", PMIC_NAME);
    pmic_disable(2);
    pmic_disable(1);
    pmic_disable(0);
    pr_info("%s: Power-off sequence completed\n", PMIC_NAME);
}

// Probe / Remove
static int my_pmic_probe(struct i2c_client *client,
                         const struct i2c_device_id *id)
{
    pr_info("%s: Probing PMIC at 0x%02x on bus %d\n",
            PMIC_NAME, client->addr, client->adapter->nr);

    // Initialize registers
    mutex_lock(&pmic_lock);
    memset(pmic_regs, 0, sizeof(pmic_regs));
    mutex_unlock(&pmic_lock);

    // Create workqueue for fake PG interrupts
    pmic_wq = create_singlethread_workqueue("pmic_pg_wq");
    INIT_WORK(&pg_work, pg_work_func);

    pmic_power_on_sequence();
    return 0;
}

static int my_pmic_remove(struct i2c_client *client)
{
    pmic_power_off_sequence();
    if (pmic_wq) {
        flush_workqueue(pmic_wq);
        destroy_workqueue(pmic_wq);
    }
    pr_info("%s: Removed from bus %d\n", PMIC_NAME, client->adapter->nr);
    return 0;
}

// Device ID table
static const struct i2c_device_id my_pmic_id[] = {
    { PMIC_NAME, 0 },
    { }
};
MODULE_DEVICE_TABLE(i2c, my_pmic_id);

// Driver definition
static struct i2c_driver my_pmic_driver = {
    .driver = {
        .name = PMIC_NAME,
    },
    .probe = my_pmic_probe,
    .remove = my_pmic_remove,
    .id_table = my_pmic_id,
};

module_i2c_driver(my_pmic_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("afreen");
MODULE_DESCRIPTION("Simulated PMIC with multiple regulators, sequencing, and fake power-good interrupts");

