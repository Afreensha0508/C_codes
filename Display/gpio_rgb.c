#include <gpiod.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

// --- Configuration ---
#define SPI_DEVICE    "/dev/spidev0.0"
#define SPI_SPEED     32000000   // 32 MHz
#define GPIO_CHIP     "gpiochip0"
#define LCD_RST_PIN   25         // BCM GPIO25
#define LCD_RS_PIN    24         // BCM GPIO24
#define LCD_WIDTH     176
#define LCD_HEIGHT    220

// Global handles
struct gpiod_chip *chip;
struct gpiod_line *lcd_rst_line;
struct gpiod_line *lcd_rs_line;
int spi_fd;

// --- Low-Level SPI Write ---
void spi_write(const void* data, size_t len) {
    if (write(spi_fd, data, len) != len) {
        perror("SPI write failed");
    }
}

// --- Command/Data ---
void lcd_cmd(uint16_t cmd) {
    uint8_t d[2] = { cmd >> 8, cmd & 0xFF };
    gpiod_line_set_value(lcd_rs_line, 0); // RS low = command
    spi_write(d, 2);
}

void lcd_data(uint16_t data) {
    uint8_t d[2] = { data >> 8, data & 0xFF };
    gpiod_line_set_value(lcd_rs_line, 1); // RS high = data
    spi_write(d, 2);
}

// --- Init GPIO ---
int gpio_init() {
    chip = gpiod_chip_open_by_name(GPIO_CHIP);
    if (!chip) {
        perror("chip open failed");
        return -1;
    }
    lcd_rst_line = gpiod_chip_get_line(chip, LCD_RST_PIN);
    lcd_rs_line  = gpiod_chip_get_line(chip, LCD_RS_PIN);
    if (!lcd_rst_line || !lcd_rs_line) {
        perror("get line failed");
        return -1;
    }
    if (gpiod_line_request_output(lcd_rst_line, "ili9225", 0) < 0 ||
        gpiod_line_request_output(lcd_rs_line, "ili9225", 0) < 0) {
        perror("request output failed");
        return -1;
    }
    return 0;
}

// --- Init SPI ---
int spi_init() {
    spi_fd = open(SPI_DEVICE, O_RDWR);
    if (spi_fd < 0) {
        perror("SPI open failed");
        return -1;
    }
    uint8_t mode = SPI_MODE_0;
    uint32_t speed = SPI_SPEED;
    if (ioctl(spi_fd, SPI_IOC_WR_MODE, &mode) == -1 ||
        ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) == -1) {
        perror("SPI setup failed");
        return -1;
    }
    return 0;
}

// --- LCD Init (from your WiringPi code) ---
void lcd_init() {
    // Hardware reset
    gpiod_line_set_value(lcd_rst_line, 1);
    usleep(10000);
    gpiod_line_set_value(lcd_rst_line, 0);
    usleep(50000);
    gpiod_line_set_value(lcd_rst_line, 1);
    usleep(100000);

    // --- Init sequence 
    lcd_cmd(0x01); usleep(50000); // Software reset

    lcd_cmd(0x10); lcd_data(0x0000);
    lcd_cmd(0x11); lcd_data(0x0018);
    lcd_cmd(0x12); lcd_data(0x6121);
    lcd_cmd(0x13); lcd_data(0x006F);
    lcd_cmd(0x14); lcd_data(0x495F);
    lcd_cmd(0x10); lcd_data(0x0800);
    usleep(10000);
    lcd_cmd(0x11); lcd_data(0x103B);
    usleep(50000);
    lcd_cmd(0x01); lcd_data(0x011C);
    lcd_cmd(0x02); lcd_data(0x0100);
    lcd_cmd(0x03); lcd_data(0x1030);
    lcd_cmd(0x08); lcd_data(0x0808);
    lcd_cmd(0x0B); lcd_data(0x1100);
    lcd_cmd(0x0C); lcd_data(0x0000);
    lcd_cmd(0x0F); lcd_data(0x0801);

    // Gamma (simple version like yours)
    lcd_cmd(0x30); lcd_data(0x0000);
    lcd_cmd(0x31); lcd_data(0x0000);
    lcd_cmd(0x32); lcd_data(0x0000);
    lcd_cmd(0x35); lcd_data(0x0000);
    lcd_cmd(0x36); lcd_data(0x0000);
    lcd_cmd(0x37); lcd_data(0x0000);

    lcd_cmd(0x07); lcd_data(0x1017); // Display ON
}

// --- Drawing ---
void lcd_setWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    lcd_cmd(0x36); lcd_data(x2);
    lcd_cmd(0x37); lcd_data(x1);
    lcd_cmd(0x38); lcd_data(y2);
    lcd_cmd(0x39); lcd_data(y1);
    lcd_cmd(0x20); lcd_data(x1);
    lcd_cmd(0x21); lcd_data(y1);
}

void lcd_fillScreen(uint16_t color) {
    lcd_setWindow(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
    lcd_cmd(0x22);
    for (int i = 0; i < LCD_WIDTH * LCD_HEIGHT; i++) {
        lcd_data(color);
    }
}

// RGB888 → RGB565
uint16_t color565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) |
           ((g & 0xFC) << 3) |
           ((b & 0xF8) >> 3);
}

// --- Main ---
int main() {
    if (gpio_init() != 0) return -1;
    if (spi_init() != 0) return -1;

    lcd_init();

    printf("Displaying colors...\n");
    lcd_fillScreen(color565(255,0,0)); sleep(1);
    lcd_fillScreen(color565(0,255,0)); sleep(1);
    lcd_fillScreen(color565(0,0,255)); sleep(1);
    lcd_fillScreen(color565(255,255,255)); sleep(1);
    lcd_fillScreen(color565(0,0,0)); sleep(1);

    gpiod_line_release(lcd_rst_line);
    gpiod_line_release(lcd_rs_line);
    gpiod_chip_close(chip);
    close(spi_fd);
    return 0;
}

