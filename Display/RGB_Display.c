#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#define SPI_CHANNEL 0
#define SPI_SPEED   32000000    // 32 MHz
#define LCD_RST     25          // GPIO25
#define LCD_RS      24          // GPIO24
#define LCD_WIDTH   176
#define LCD_HEIGHT  220

// Send command to LCD
void lcd_cmd(uint16_t cmd) {
    uint8_t d[2];
    d[0] = cmd >> 8;
    d[1] = cmd & 0xFF;
    digitalWrite(LCD_RS, LOW);
    wiringPiSPIDataRW(SPI_CHANNEL, d, 2);
}

// Send data to LCD
void lcd_data(uint16_t data) {
    uint8_t d[2];
    d[0] = data >> 8;
    d[1] = data & 0xFF;
    digitalWrite(LCD_RS, HIGH);
    wiringPiSPIDataRW(SPI_CHANNEL, d, 2);
}

// Initialize LCD with a typical, more complete sequence
void lcd_init() {
    pinMode(LCD_RST, OUTPUT);
    pinMode(LCD_RS, OUTPUT);

    digitalWrite(LCD_RST, HIGH);
    delay(10);
    digitalWrite(LCD_RST, LOW);
    delay(50);
    digitalWrite(LCD_RST, HIGH);
    delay(100);

    // --- Start of a typical ILI9225 init sequence ---
    // This is an example. You may need to modify it for your specific display module.
    lcd_cmd(0x01); // Software reset
    delay(50);
    
    // Power Control 1, VRH[5:0] & VC[2:0]
    lcd_cmd(0x10);
    lcd_data(0x0000); // Set GVDD to 2.5V
    
    // Power Control 2, VGLD[4:0] & VGS[4:0]
    lcd_cmd(0x11);
    lcd_data(0x0018); // Set VGL to -10V, VGH to 20V
    
    // Power Control 3, VcomL[2:0]
    lcd_cmd(0x12);
    lcd_data(0x6121); // Set VCOM to -1.5V

    // Power Control 4, VcomH[2:0]
    lcd_cmd(0x13);
    lcd_data(0x006F); // Set VCOM to 4V

    // Power Control 5, VGLS[2:0] & VGS[2:0]
    lcd_cmd(0x14);
    lcd_data(0x495F);
    lcd_cmd(0x10);
    lcd_data(0x0800);
    delay(10);
    lcd_cmd(0x11); lcd_data(0x103B); // Set APON,PON,AON,VCI1EN,VC
    delay(50);
    lcd_cmd(0x01); lcd_data(0x011C); // Set the display line number and display direction
    lcd_cmd(0x02); lcd_data(0x0100); // Set 1 line inversion
    lcd_cmd(0x03); lcd_data(0x1030); // Set GRAM write direction and BGR=1
    lcd_cmd(0x08); lcd_data(0x0808); // Set BP and FP
    lcd_cmd(0x0B); lcd_data(0x1100); // Set RGB mode
    lcd_cmd(0x0C); lcd_data(0x0000); // Set Power Control 1
    lcd_cmd(0x0F); lcd_data(0x0801); // Set Power Control 2
    
    // --- Gamma Curve ---
    lcd_cmd(0x30); lcd_data(0x0000); // Set Gamma Correction 1
    lcd_cmd(0x31); lcd_data(0x0000); // Set Gamma Correction 2
    lcd_cmd(0x32); lcd_data(0x0000); // Set Gamma Correction 3
    lcd_cmd(0x35); lcd_data(0x0000); // Set Gamma Correction 6
    lcd_cmd(0x36); lcd_data(0x0000); // Set Gamma Correction 7
    lcd_cmd(0x37); lcd_data(0x0000); // Set Gamma Correction 8
    
    lcd_cmd(0x07); lcd_data(0x1017); // Set Display On

    // --- End of typical init sequence ---
}

// Set window (for drawing area)
void lcd_setWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
	lcd_cmd(0x36);
	lcd_data(x2);
	lcd_cmd(0x37);
	lcd_data(x1);
	lcd_cmd(0x38);
	lcd_data(y2);
	lcd_cmd(0x39);
	lcd_data(y1);
	lcd_cmd(0x20);
	lcd_data(x1);
	lcd_cmd(0x21);
	lcd_data(y1);
}

// Fill screen with a color
void lcd_fillScreen(uint16_t color) {
    lcd_setWindow(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
    lcd_cmd(0x22);

    for (int i = 0; i < LCD_WIDTH * LCD_HEIGHT; i++) {
        lcd_data(color);
    }
}

// Convert RGB888 to RGB565
uint16_t color565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) |
           ((g & 0xFC) << 3) |
           ((b & 0xF8) >> 3);
}

int main() {
    wiringPiSetupGpio();
    if (wiringPiSPISetup(SPI_CHANNEL, SPI_SPEED) < 0) {
        printf("SPI Setup failed!\n");
        return -1;
    }

    lcd_init();

    printf("Displaying colors...\n");

    lcd_fillScreen(color565(255, 0, 0)); // Red
    delay(1000);
    lcd_fillScreen(color565(0, 255, 0)); // Green
    delay(1000);
    lcd_fillScreen(color565(0, 0, 255)); // Blue
    delay(1000);
    lcd_fillScreen(color565(255, 255, 255)); // White
    delay(1000);
    lcd_fillScreen(color565(0, 0, 0)); // Black
    delay(1000);

    return 0;
}

