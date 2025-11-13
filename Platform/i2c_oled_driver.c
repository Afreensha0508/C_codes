#include<linux/module.h>
#include<linux/i2c.h>
#include<linux/of.h>
#include<linux/delay.h>

#define SSD1306_CMD 0x00
#define SSD1306_DATA 0x40

static u8 oled_init_seq[] = {
    0xAE,       // Display OFF
    0xD5, 0x80, // Set display clock divide ratio
    0xA8, 0x3F, // Set multiplex ratio (1 to 64)
    0xD3, 0x00, // Display offset
    0x40,       // Start line address
    0x8D, 0x14, // Charge Pump
    0x20, 0x00, // Memory addressing mode: Horizontal
    0xA1,       // Segment remap
    0xC8,       // COM output scan direction
    0xDA, 0x12, // COM pins
    0x81, 0xCF, // Contrast
    0xD9, 0xF1, // Pre-charge
    0xDB, 0x40, // VCOMH
    0xA4,       // Resume RAM display
    0xA6,       // Normal display
    0xAF        // Display ON
};


static int ssd1306_write_command(struct i2c_client *client, u8 cmd)
{
    u8 buffer[2] = { SSD1306_CMD, cmd };
    return i2c_master_send(client, buffer, 2);
}

static void ssd1306_init_display(struct i2c_client *client)
{
    int i;
    dev_info(&client->dev, "Sending OLED init sequence...\n");
    msleep(100);

    for (i = 0; i < ARRAY_SIZE(oled_init_seq); i++) {
        if (ssd1306_write_command(client, oled_init_seq[i]) < 0) {
            dev_err(&client->dev, "OLED init failed at cmd index %d\n", i);
            return;
        }
    }
    dev_info(&client->dev, "OLED init sequence completed\n");
}

static int ssd1306_probe(struct i2c_client *client)
{
    dev_info(&client->dev, "SSD1306 OLED device detected\n");
    ssd1306_init_display(client);
    return 0;
}

static void ssd1306_remove(struct i2c_client *client)
{
    dev_info(&client->dev, "SSD1306 OLED removed\n");
}

static const struct of_device_id ssd1306_dt_ids[] = {
    { .compatible = "my_i2c,ssd1306" },
    {}
};

MODULE_DEVICE_TABLE(of, ssd1306_dt_ids);

static struct i2c_driver ssd1306_driver = {
    .driver = {
        .name = "ssd1306_platform",
        .of_match_table = ssd1306_dt_ids,
    },
    .probe = ssd1306_probe,
    .remove = ssd1306_remove,
};

module_i2c_driver(ssd1306_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("afreen");
MODULE_DESCRIPTION("SSD1306 OLED I2C Platform Driver");

