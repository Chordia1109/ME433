#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"

#include "bsp/board_api.h"
#include "tusb.h"
#include "usb_descriptors.h"

#define I2C_PORT i2c0
#define SDA_PIN 4
#define SCL_PIN 5

#define MPU6050_ADDR 0x68
#define MPU6050_PWR_MGMT_1 0x6B
#define MPU6050_ACCEL_XOUT_H 0x3B

#define BUTTON_PIN 15
#define LED_PIN 16

static bool remote_mode = false;
static bool last_button_state = true;

static void mpu6050_write_reg(uint8_t reg, uint8_t value)
{
    uint8_t buf[2] = {reg, value};
    i2c_write_blocking(I2C_PORT, MPU6050_ADDR, buf, 2, false);
}

static void mpu6050_read_accel(int16_t *ax, int16_t *ay, int16_t *az)
{
    uint8_t reg = MPU6050_ACCEL_XOUT_H;
    uint8_t data[6];

    i2c_write_blocking(I2C_PORT, MPU6050_ADDR, &reg, 1, true);
    i2c_read_blocking(I2C_PORT, MPU6050_ADDR, data, 6, false);

    *ax = (int16_t)((data[0] << 8) | data[1]);
    *ay = (int16_t)((data[2] << 8) | data[3]);
    *az = (int16_t)((data[4] << 8) | data[5]);
}

static void mpu6050_init(void)
{
    i2c_init(I2C_PORT, 400 * 1000);

    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);

    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    sleep_ms(100);

    // Wake up MPU6050
    mpu6050_write_reg(MPU6050_PWR_MGMT_1, 0x00);

    sleep_ms(100);
}

static int8_t accel_to_mouse_delta(int16_t accel)
{
    int abs_a = abs(accel);

    // Dead zone
    if (abs_a < 3000)
    {
        return 0;
    }

    int8_t speed;

    // Discrete speed levels required by the assignment
    if (abs_a < 6000)
    {
        speed = 1;
    }
    else if (abs_a < 10000)
    {
        speed = 3;
    }
    else if (abs_a < 14000)
    {
        speed = 5;
    }
    else
    {
        speed = 8;
    }

    if (accel < 0)
    {
        speed = -speed;
    }

    return speed;
}

static void check_button_toggle(void)
{
    bool button_state = gpio_get(BUTTON_PIN);

    // Because we use pull-up:
    // not pressed = 1
    // pressed = 0
    if (last_button_state == true && button_state == false)
    {
        remote_mode = !remote_mode;
        gpio_put(LED_PIN, remote_mode ? 1 : 0);

        // Simple debounce
        sleep_ms(200);
    }

    last_button_state = button_state;
}

static void send_mouse_report(void)
{
    if (!tud_hid_ready())
    {
        return;
    }

    int8_t dx = 0;
    int8_t dy = 0;

    if (remote_mode)
    {
        // Remote working mode:
        // mouse moves in a slow circle
        static uint32_t start_ms = 0;

        uint32_t now = board_millis();

        if (start_ms == 0)
        {
            start_ms = now;
        }

        float t = (now - start_ms) / 1000.0f;

        dx = (int8_t)(3.0f * cosf(t));
        dy = (int8_t)(3.0f * sinf(t));
    }
    else
    {
        // Regular mode:
        // use MPU6050 acceleration to control mouse
        int16_t ax, ay, az;
        mpu6050_read_accel(&ax, &ay, &az);

        dx = accel_to_mouse_delta(ax);
        dy = accel_to_mouse_delta(ay);

        // If direction is wrong, change signs here.
        // dx = -accel_to_mouse_delta(ax);
        // dy = -accel_to_mouse_delta(ay);
    }

    tud_hid_mouse_report(REPORT_ID_MOUSE, 0x00, dx, dy, 0, 0);
}

int main(void)
{
    board_init();
    tusb_init();

    mpu6050_init();

    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 0);

    while (1)
    {
        tud_task();

        check_button_toggle();

        static uint32_t last_ms = 0;
        uint32_t now = board_millis();

        // Send mouse reports at about 100 Hz
        if (now - last_ms >= 10)
        {
            last_ms = now;
            send_mouse_report();
        }
    }

    return 0;
}

uint16_t tud_hid_get_report_cb(uint8_t instance,
                               uint8_t report_id,
                               hid_report_type_t report_type,
                               uint8_t *buffer,
                               uint16_t reqlen)
{
    (void) instance;
    (void) report_id;
    (void) report_type;
    (void) buffer;
    (void) reqlen;

    return 0;
}

void tud_hid_set_report_cb(uint8_t instance,
                           uint8_t report_id,
                           hid_report_type_t report_type,
                           uint8_t const *buffer,
                           uint16_t bufsize)
{
    (void) instance;
    (void) report_id;
    (void) report_type;
    (void) buffer;
    (void) bufsize;
}