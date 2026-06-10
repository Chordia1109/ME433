#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "pico/stdlib.h"
#include "hardware/i2c.h"

// =======================
// Pin setup
// =======================

// AS5600 on I2C0
#define I2C_PORT i2c0
#define AS5600_ADDR 0x36

#define I2C_SDA_PIN 4
#define I2C_SCL_PIN 5

// HX711
#define HX711_DOUT_PIN 14
#define HX711_SCK_PIN 15

#define HX711_TIMEOUT_CODE 123456789

// B channel = 2 extra pulses
#define HX711_EXTRA_PULSES 2

// Force filtering
#define TARE_SAMPLES 30
#define AVG_SAMPLES 8


// =======================
// AS5600 functions
// =======================

int read_as5600_angle_raw(void) {
    uint8_t reg = 0x0E;
    uint8_t data[2];

    int w = i2c_write_blocking(I2C_PORT, AS5600_ADDR, &reg, 1, true);
    if (w < 0) {
        return -1;
    }

    int r = i2c_read_blocking(I2C_PORT, AS5600_ADDR, data, 2, false);
    if (r < 0) {
        return -1;
    }

    int angle = ((data[0] & 0x0F) << 8) | data[1];
    return angle;
}


void scan_i2c(void) {
    printf("I2C scan: ");

    bool found = false;

    for (int addr = 0x08; addr < 0x78; addr++) {
        uint8_t dummy;
        int ret = i2c_read_blocking(I2C_PORT, addr, &dummy, 1, false);

        if (ret >= 0) {
            printf("0x%02X ", addr);
            found = true;
        }
    }

    if (!found) {
        printf("none");
    }

    printf("\n");
}


// =======================
// HX711 functions
// =======================

void hx711_init(void) {
    gpio_init(HX711_DOUT_PIN);
    gpio_set_dir(HX711_DOUT_PIN, GPIO_IN);

    gpio_init(HX711_SCK_PIN);
    gpio_set_dir(HX711_SCK_PIN, GPIO_OUT);

    gpio_put(HX711_SCK_PIN, 0);

    sleep_ms(100);
}


bool hx711_wait_ready(uint32_t timeout_ms) {
    uint64_t start = time_us_64();

    while (gpio_get(HX711_DOUT_PIN) == 1) {
        if ((time_us_64() - start) > timeout_ms * 1000) {
            return false;
        }
    }

    return true;
}


int32_t hx711_read_raw_B_channel(void) {
    if (!hx711_wait_ready(100)) {
        return HX711_TIMEOUT_CODE;
    }

    int32_t value = 0;

    // Read 24 data bits
    for (int i = 0; i < 24; i++) {
        gpio_put(HX711_SCK_PIN, 1);
        sleep_us(2);

        value = value << 1;

        if (gpio_get(HX711_DOUT_PIN)) {
            value++;
        }

        gpio_put(HX711_SCK_PIN, 0);
        sleep_us(2);
    }

    // 2 extra pulses = select B channel, gain 32 for next reading
    for (int i = 0; i < HX711_EXTRA_PULSES; i++) {
        gpio_put(HX711_SCK_PIN, 1);
        sleep_us(2);
        gpio_put(HX711_SCK_PIN, 0);
        sleep_us(2);
    }

    // Convert 24-bit signed to 32-bit signed
    if (value & 0x800000) {
        value |= ~0xFFFFFF;
    }

    return value;
}


int32_t hx711_read_average_B(int samples) {
    int64_t sum = 0;
    int count = 0;

    for (int i = 0; i < samples; i++) {
        int32_t reading = hx711_read_raw_B_channel();

        if (reading != HX711_TIMEOUT_CODE) {
            sum += reading;
            count++;
        }

        sleep_ms(10);
    }

    if (count == 0) {
        return HX711_TIMEOUT_CODE;
    }

    return (int32_t)(sum / count);
}


int32_t hx711_tare_B(int samples) {
    int64_t sum = 0;
    int count = 0;

    for (int i = 0; i < samples; i++) {
        int32_t reading = hx711_read_raw_B_channel();

        if (reading != HX711_TIMEOUT_CODE) {
            sum += reading;
            count++;
        }

        sleep_ms(20);
    }

    if (count == 0) {
        return 0;
    }

    return (int32_t)(sum / count);
}


// =======================
// Main
// =======================

int main() {
    stdio_init_all();

    sleep_ms(2000);

    printf("\n\n");
    printf("=====================================\n");
    printf("HW17 FINAL: AS5600 + HX711 B CHANNEL\n");
    printf("=====================================\n");

    // -----------------------
    // I2C setup for AS5600
    // -----------------------
    i2c_init(I2C_PORT, 400000);

    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);

    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);

    printf("I2C SDA = GP%d\n", I2C_SDA_PIN);
    printf("I2C SCL = GP%d\n", I2C_SCL_PIN);

    scan_i2c();

    int test_angle = read_as5600_angle_raw();

    if (test_angle < 0) {
        printf("WARNING: AS5600 not readable.\n");
    } else {
        printf("AS5600 readable. Initial angle_raw = %d\n", test_angle);
    }

    // -----------------------
    // HX711 setup
    // -----------------------
    hx711_init();

    printf("HX711 DOUT / DT = GP%d\n", HX711_DOUT_PIN);
    printf("HX711 SCK       = GP%d\n", HX711_SCK_PIN);
    printf("HX711 channel   = B channel, gain 32\n");

    // Important:
    // HX711 channel selection applies to the NEXT reading.
    // First read may still be previous/default channel, so discard it.
    hx711_read_raw_B_channel();
    sleep_ms(150);

    printf("Do not touch the load cell. Taring...\n");
    int32_t force_offset = hx711_tare_B(TARE_SAMPLES);
    printf("force_offset = %ld\n", (long)force_offset);

    printf("\n");
    printf("Start streaming:\n");
    printf("angle_raw,angle_deg,force_raw\n");

    while (true) {
        int angle_raw = read_as5600_angle_raw();

        float angle_deg = -1.0f;
        if (angle_raw >= 0) {
            angle_deg = angle_raw * 360.0f / 4096.0f;
        }

        int32_t force_avg = hx711_read_average_B(AVG_SAMPLES);

        int32_t force_raw = 0;
        if (force_avg != HX711_TIMEOUT_CODE) {
            force_raw = force_avg - force_offset;
        } else {
            force_raw = HX711_TIMEOUT_CODE;
        }

        printf("%d,%.2f,%ld\n",
               angle_raw,
               angle_deg,
               (long)force_raw);

        sleep_ms(50);
    }

    return 0;
}