#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H

void oled_init(void);
void oled_print(const char *msg);
void oled_close(void);
void oled_set_cursor(uint8_t col, uint8_t page);
void oled_draw_char(char c);
void oled_draw_char_scaled(char c, int scale, int x, int y);
extern const uint8_t font5x7[][5];
void oled_draw_pixel(int x, int y);

#endif // OLED_DISPLAY_H

