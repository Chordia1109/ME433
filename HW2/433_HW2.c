#include <stdio.h>
#include "pico/stdlib.h"
#include <hardware/pwm.h>

#define SERVO_PIN 15

void servo_init(uint pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM);

    uint slice_num = pwm_gpio_to_slice_num(pin);

    pwm_set_clkdiv(slice_num, 125.0f);
    pwm_set_wrap(slice_num, 20000 - 1);
    pwm_set_enabled(slice_num, true);
}

void set_servo_angle(uint pin, float angle) {
    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;

    float pulse_width_us = 500.0f + (angle / 180.0f) * 2000.0f;

    uint slice_num = pwm_gpio_to_slice_num(pin);
    uint channel = pwm_gpio_to_channel(pin);

    pwm_set_chan_level(slice_num, channel, (uint16_t)pulse_width_us);
}

int main() {
    stdio_init_all();
    servo_init(SERVO_PIN);

    while (1) {
        for (float angle = 20; angle <= 160; angle += 5)
            set_servo_angle(SERVO_PIN, angle);
            sleep_ms(50);
        }

       for (float angle = 160; angle >= 20; angle -= 5)
            set_servo_angle(SERVO_PIN, angle);
            sleep_ms(50);
        }
    }
}