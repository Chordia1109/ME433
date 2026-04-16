#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "ssd1306_HW5.h"

#define I2C_PORT i2c0
#define SDA_PIN 4
#define SCL_PIN 5

#define MPU6050_ADDR 0x68
#define PWR_MGMT_1   0x6B
#define GYRO_CONFIG  0x1B
#define ACCEL_CONFIG 0x1C
#define ACCEL_XOUT_H 0x3B
#define WHO_AM_I     0x75

void mpu6050_write_reg(uint8_t reg, uint8_t value) {
    uint8_t buf[2];
    buf[0] = reg;
    buf[1] = value;
    i2c_write_blocking(I2C_PORT, MPU6050_ADDR, buf, 2, false);
}

uint8_t mpu6050_read_reg(uint8_t reg) {
    uint8_t value;
    i2c_write_blocking(I2C_PORT, MPU6050_ADDR, &reg, 1, true);
    i2c_read_blocking(I2C_PORT, MPU6050_ADDR, &value, 1, false);
    return value;
}

void mpu6050_init(void) {
    mpu6050_write_reg(PWR_MGMT_1, 0x00);
    sleep_ms(100);
    mpu6050_write_reg(ACCEL_CONFIG, 0x00);
    mpu6050_write_reg(GYRO_CONFIG, 0x18);
}

void mpu6050_read_raw(int16_t *ax, int16_t *ay, int16_t *az,
                      int16_t *temp, int16_t *gx, int16_t *gy, int16_t *gz) {
    uint8_t reg = ACCEL_XOUT_H;
    uint8_t data[14];

    i2c_write_blocking(I2C_PORT, MPU6050_ADDR, &reg, 1, true);
    i2c_read_blocking(I2C_PORT, MPU6050_ADDR, data, 14, false);

    *ax   = (int16_t)((data[0] << 8) | data[1]);
    *ay   = (int16_t)((data[2] << 8) | data[3]);
    *az   = (int16_t)((data[4] << 8) | data[5]);
    *temp = (int16_t)((data[6] << 8) | data[7]);
    *gx   = (int16_t)((data[8] << 8) | data[9]);
    *gy   = (int16_t)((data[10] << 8) | data[11]);
    *gz   = (int16_t)((data[12] << 8) | data[13]);
}

void draw_line_from_accel(float ax_g, float ay_g) {
    int cx = 64;
    int cy = 32;
    int scale = 25;

    int x1 = cx + (int)(ax_g * scale);
    int y1 = cy - (int)(ay_g * scale);

    if (x1 < 0) x1 = 0;
    if (x1 > 127) x1 = 127;
    if (y1 < 0) y1 = 0;
    if (y1 > 63) y1 = 63;

    ssd1306_clear();

    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            int px = cx + dx;
            int py = cy + dy;
            if (px >= 0 && px < 128 && py >= 0 && py < 64) {
                ssd1306_drawPixel(px, py, 1);
            }
        }
    }

    int steps = 40;
    for (int i = 0; i <= steps; i++) {
        int x = cx + (x1 - cx) * i / steps;
        int y = cy + (y1 - cy) * i / steps;
        ssd1306_drawPixel(x, y, 1);
    }

    ssd1306_update();
}

int main() {
    stdio_init_all();
    sleep_ms(2000);

    i2c_init(I2C_PORT, 100000);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    sleep_ms(500);

    ssd1306_setup();
    ssd1306_clear();
    ssd1306_update();

    mpu6050_init();

    uint8_t who = mpu6050_read_reg(WHO_AM_I);
    printf("WHO_AM_I = 0x%02X\n", who);

    while (true) {
        int16_t ax, ay, az, temp, gx, gy, gz;
        mpu6050_read_raw(&ax, &ay, &az, &temp, &gx, &gy, &gz);

        float ax_g = ax / 16384.0f;
        float ay_g = ay / 16384.0f;
        float az_g = az / 16384.0f;

        printf("ax=%.2f ay=%.2f az=%.2f\n", ax_g, ay_g, az_g);

        draw_line_from_accel(ax_g, ay_g);

        sleep_ms(20);
    }
}