// based on adafruit and sparkfun libraries

#include <string.h>
#include "ssd1306_HW5.h"
#include "hardware/i2c.h"
#include "pico/stdlib.h"

#define SSD1306_ADDRESS 0x3C

unsigned char ssd1306_buffer[1025];   // 128x64/8 + 1 control byte

void ssd1306_setup(void) {
    ssd1306_buffer[0] = 0x40;

    sleep_ms(20);

    ssd1306_command(SSD1306_DISPLAYOFF);
    ssd1306_command(SSD1306_SETDISPLAYCLOCKDIV);
    ssd1306_command(0x80);

    ssd1306_command(SSD1306_SETMULTIPLEX);
    ssd1306_command(0x3F);   // 64 pixels high -> 0x3F

    ssd1306_command(SSD1306_SETDISPLAYOFFSET);
    ssd1306_command(0x00);

    ssd1306_command(SSD1306_SETSTARTLINE | 0x00);

    ssd1306_command(SSD1306_CHARGEPUMP);
    ssd1306_command(0x14);

    ssd1306_command(SSD1306_MEMORYMODE);
    ssd1306_command(0x00);

    ssd1306_command(SSD1306_SEGREMAP | 0x01);
    ssd1306_command(SSD1306_COMSCANDEC);

    ssd1306_command(SSD1306_SETCOMPINS);
    ssd1306_command(0x12);

    ssd1306_command(SSD1306_SETCONTRAST);
    ssd1306_command(0xCF);

    ssd1306_command(SSD1306_SETPRECHARGE);
    ssd1306_command(0xF1);

    ssd1306_command(SSD1306_SETVCOMDETECT);
    ssd1306_command(0x40);

    ssd1306_command(SSD1306_DISPLAYON);

    ssd1306_clear();
    ssd1306_update();
}

// send one command byte
void ssd1306_command(unsigned char c) {
    uint8_t buf[2];
    buf[0] = 0x00;
    buf[1] = c;

    i2c_write_blocking(i2c0, SSD1306_ADDRESS, buf, 2, false);
}

// push full buffer to display
void ssd1306_update(void) {
    ssd1306_command(SSD1306_PAGEADDR);
    ssd1306_command(0);
    ssd1306_command(7);   // 64 pixels = 8 pages

    ssd1306_command(SSD1306_COLUMNADDR);
    ssd1306_command(0);
    ssd1306_command(127);

    i2c_write_blocking(i2c0, SSD1306_ADDRESS, ssd1306_buffer, 1025, false);
}

// draw one pixel
void ssd1306_drawPixel(unsigned char x, unsigned char y, unsigned char color) {
    if (x >= 128 || y >= 64) {
        return;
    }

    if (color) {
        ssd1306_buffer[1 + x + (y / 8) * 128] |= (1 << (y & 7));
    } else {
        ssd1306_buffer[1 + x + (y / 8) * 128] &= ~(1 << (y & 7));
    }
}

// clear screen buffer
void ssd1306_clear(void) {
    memset(&ssd1306_buffer[1], 0, 1024);
    ssd1306_buffer[0] = 0x40;
}