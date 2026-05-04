#include <stdio.h>
#include "pico/stdlib.h"

#define BUTTON_PIN 15
#define LED_PIN 16

int main() {
    stdio_init_all();

    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    while (true) {
        int button_value = gpio_get(BUTTON_PIN);

        // Send button state to computer
        printf("B,%d\n", button_value);

        // Read command from computer if available
        int ch = getchar_timeout_us(0);

        if (ch == '1') {
            gpio_put(LED_PIN, 1);
        } else if (ch == '0') {
            gpio_put(LED_PIN, 0);
        }

        sleep_ms(50);
    }
}