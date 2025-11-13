#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/init.h>

#define PMIC_NAME "my_pmic"

// Example "registers" for simulation
#define REG_VOLTAGE 0x01
#define REG_MODE    0x02

// Probe function with correct signature
static int my_pmic_probe(struct i2c_client *client,
                         const struct i2c_device_id *id)
{
    u8 val;

    pr_info("%s: Probing PMIC at 0x%02x on bus %d\n",
            PMIC_NAME, client->addr, client->adapter->nr);

    // Simulate reading a register
    val = i2c_smbus_read_byte_data(client, REG_VOLTAGE);
    pr_info("%s: Read REG_VOLTAGE = 0x%02x\n", PMIC_NAME, val);

    // Simulate writing a register
    i2c_smbus_write_byte_data(client, REG_MODE, 0x1);
    pr_info("%s: Wrote 0x01 to REG_MODE\n", PMIC_NAME);

    return 0;
}

static int my_pmic_remove(struct i2c_client *client)
{
    pr_info("%s: Removing PMIC driver from bus %d\n",
            PMIC_NAME, client->adapter->nr);
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
MODULE_DESCRIPTION("Fake PMIC driver for i2c-stub testing");

