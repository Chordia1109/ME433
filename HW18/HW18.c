#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

#define MOTOR_IN1 14
#define MOTOR_IN2 15
#define PWM_WRAP 999

static void pwm_pin_init(uint gpio) {
    gpio_set_function(gpio, GPIO_FUNC_PWM);

    uint slice = pwm_gpio_to_slice_num(gpio);
    pwm_set_wrap(slice, PWM_WRAP);
    pwm_set_clkdiv(slice, 125.0f);
    pwm_set_enabled(slice, true);
}

static void motor_init(void) {
    pwm_pin_init(MOTOR_IN1);
    pwm_pin_init(MOTOR_IN2);

    pwm_set_gpio_level(MOTOR_IN1, 0);
    pwm_set_gpio_level(MOTOR_IN2, 0);
}

static void motor_set_pwm(float duty) {
    if (duty > 1.0f) duty = 1.0f;
    if (duty < -1.0f) duty = -1.0f;

    uint16_t level;

    if (duty > 0.0f) {
        level = (uint16_t)(duty * PWM_WRAP);
        pwm_set_gpio_level(MOTOR_IN1, level);
        pwm_set_gpio_level(MOTOR_IN2, 0);
    } else if (duty < 0.0f) {
        level = (uint16_t)((-duty) * PWM_WRAP);
        pwm_set_gpio_level(MOTOR_IN1, 0);
        pwm_set_gpio_level(MOTOR_IN2, level);
    } else {
        pwm_set_gpio_level(MOTOR_IN1, 0);
        pwm_set_gpio_level(MOTOR_IN2, 0);
    }
}

int main(void) {
    stdio_init_all();
    sleep_ms(2000);

    printf("\nDRV8833 motor test\n");

    motor_init();

    while (true) {
        printf("forward 0.7\n");
        motor_set_pwm(0.7f);
        sleep_ms(2000);

        printf("stop\n");
        motor_set_pwm(0.0f);
        sleep_ms(1000);

        printf("reverse 0.7\n");
        motor_set_pwm(-0.7f);
        sleep_ms(2000);

        printf("stop\n");
        motor_set_pwm(0.0f);
        sleep_ms(1000);
    }

    return 0;
}