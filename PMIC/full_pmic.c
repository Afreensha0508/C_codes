#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/mutex.h>

#define PMIC_NAME "my_pmic"

// Define virtual PMIC registers
enum {
    REG_VOLTAGE = 0x01,
    REG_MODE    = 0x02,
    REG_ENABLE  = 0x03,
    REG_SEQ     = 0x04,
    REG_MAX
};

// Simulated register storage
static u8 pmic_regs[REG_MAX];
static DEFINE_MUTEX(pmic_lock);

// Conversion helpers
static u16 reg_to_mv(u8 reg_val)
{
    // Example: each LSB = 10 mV
    return reg_val * 10;
}

static u8 mv_to_reg(u16 mv)
{
    // Clamp max 2550 mV
    if (mv > 2550)
        mv = 2550;
    return mv / 10;
}

// Regulator APIs
static int pmic_enable(void)
{
    mutex_lock(&pmic_lock);
    pmic_regs[REG_ENABLE] = 1;
    pr_info("%s: Regulator enabled\n", PMIC_NAME);
    mutex_unlock(&pmic_lock);
    return 0;
}

static int pmic_disable(void)
{
    mutex_lock(&pmic_lock);
    pmic_regs[REG_ENABLE] = 0;
    pr_info("%s: Regulator disabled\n", PMIC_NAME);
    mutex_unlock(&pmic_lock);
    return 0;
}

static int pmic_set_voltage(u16 mv)
{
    mutex_lock(&pmic_lock);
    pmic_regs[REG_VOLTAGE] = mv_to_reg(mv);
    pr_info("%s: Set voltage %d mV (reg 0x%02x)\n", PMIC_NAME, mv, pmic_regs[REG_VOLTAGE]);
    mutex_unlock(&pmic_lock);
    return 0;
}

static u16 pmic_get_voltage(void)
{
    u16 mv;
    mutex_lock(&pmic_lock);
    mv = reg_to_mv(pmic_regs[REG_VOLTAGE]);
    mutex_unlock(&pmic_lock);
    pr_info("%s: Get voltage %d mV\n", PMIC_NAME, mv);
    return mv;
}

// Power sequencing simulation
static void pmic_power_on_sequence(void)
{
    pr_info("%s: Starting power-on sequence\n", PMIC_NAME);
    pmic_set_voltage(1000);  // 1V
    msleep(50);               // delay
    pmic_enable();
    pr_info("%s: Power-on sequence completed\n", PMIC_NAME);
}

static void pmic_power_off_sequence(void)
{
    pr_info("%s: Starting power-off sequence\n", PMIC_NAME);
    pmic_disable();
    pmic_set_voltage(0);
    pr_info("%s: Power-off sequence completed\n", PMIC_NAME);
}

// Probe / Remove
static int my_pmic_probe(struct i2c_client *client)
                         
{
    pr_info("%s: Probing PMIC at 0x%02x on bus %d\n",
            PMIC_NAME, client->addr, client->adapter->nr);

    // Initialize virtual registers
    mutex_lock(&pmic_lock);
    memset(pmic_regs, 0, sizeof(pmic_regs));
    mutex_unlock(&pmic_lock);

    pmic_power_on_sequence();

    return 0;
}

static void my_pmic_remove(struct i2c_client *client)
{
    pmic_power_off_sequence();
    pr_info("%s: Removed from bus %d\n",
            PMIC_NAME, client->adapter->nr);
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
MODULE_DESCRIPTION("Fully simulated PMIC with regulators, conversions, sequencing");

