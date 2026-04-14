#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "ssd1306.h"
#include "font.h"

void drawChar(int x, int y, char c) {
    int index = c - 0x20;

    if (index < 0 || index >= 96) {
        return;
    }

    for (int col = 0; col < 5; col++) {
        unsigned char line = ASCII[index][col];
        for (int row = 0; row < 8; row++) {
            int pixel = (line >> row) & 0x01;
            ssd1306_drawPixel(x + col, y + row, pixel);
        }
    }
}

void drawString(int x, int y, char *m) {
    int i = 0;
    while (m[i] != '\0') {
        drawChar(x + 6 * i, y, m[i]);
        i++;
    }
}

int main() {
    char message[50];
    char fpsmsg[50];

    stdio_init_all();

    i2c_init(i2c_default, 400000);
    gpio_set_function(4, GPIO_FUNC_I2C);
    gpio_set_function(5, GPIO_FUNC_I2C);
    gpio_pull_up(4);
    gpio_pull_up(5);

    adc_init();
    adc_gpio_init(26);
    adc_select_input(0);

    sleep_ms(500);
    ssd1306_setup();

    while (1) {
        unsigned int t1 = to_us_since_boot(get_absolute_time());

        uint16_t adc_raw = adc_read();
        float voltage = adc_raw * 3.3f / 4095.0f;

        sprintf(message, "ADC0 = %.2f V", voltage);

        ssd1306_clear();
        drawString(0, 0, message);

        unsigned int t2 = to_us_since_boot(get_absolute_time());
        float fps = 1000000.0f / (t2 - t1);

        sprintf(fpsmsg, "FPS = %.1f", fps);
        drawString(0, 20, fpsmsg);

        ssd1306_update();

        sleep_ms(100);
    }
}