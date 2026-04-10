#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// =========================
// I2C / MCP23008 settings
// =========================
#define I2C_PORT i2c0
#define SDA_PIN 4
#define SCL_PIN 5
#define MCP23008_ADDR 0x20

// MCP23008 registers
#define IODIR 0x00
#define GPIO  0x09
#define OLAT  0x0A

// MCP pins
#define BUTTON_MASK 0x01   // GP0
#define LED_MASK    0x80   // GP7

// Pico heartbeat LED
#define HEARTBEAT_LED 16

void mcp_write(uint8_t reg, uint8_t value) {
    uint8_t buf[2] = {reg, value};
    i2c_write_blocking(I2C_PORT, MCP23008_ADDR, buf, 2, false);
}

uint8_t mcp_read(uint8_t reg) {
    uint8_t value;
    i2c_write_blocking(I2C_PORT, MCP23008_ADDR, &reg, 1, true);
    i2c_read_blocking(I2C_PORT, MCP23008_ADDR, &value, 1, false);
    return value;
}

int main() {
    stdio_init_all();

    // heartbeat LED on Pico GP16
    gpio_init(HEARTBEAT_LED);
    gpio_set_dir(HEARTBEAT_LED, GPIO_OUT);

    // I2C init
    i2c_init(I2C_PORT, 100000);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    sleep_ms(100);

    // GP7 output, others input
    mcp_write(IODIR, 0x7F);

    // MCP LED off at start
    mcp_write(OLAT, 0x00);

    while (true) {
        // heartbeat ON
        gpio_put(HEARTBEAT_LED, 1);

        uint8_t gpio_val = mcp_read(GPIO);
        if ((gpio_val & BUTTON_MASK) == 0) {
            mcp_write(OLAT, LED_MASK);   // button pressed
        } else {
            mcp_write(OLAT, 0x00);       // button not pressed
        }

        sleep_ms(200);

        // heartbeat OFF
        gpio_put(HEARTBEAT_LED, 0);

        gpio_val = mcp_read(GPIO);
        if ((gpio_val & BUTTON_MASK) == 0) {
            mcp_write(OLAT, LED_MASK);
        } else {
            mcp_write(OLAT, 0x00);
        }

        sleep_ms(200);
    }

    return 0;
}