#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

// ---------------- Pins ----------------
#define SPI_PORT spi0

#define PIN_MISO   16   // SPI0 RX, from RAM SO
#define PIN_CS_DAC 17   // MCP4912 CS
#define PIN_SCK    18   // SPI0 SCK, shared by DAC and RAM
#define PIN_MOSI   19   // SPI0 TX, to DAC SDI and RAM SI
#define PIN_CS_RAM 20   // 23K256 CS

// ---------------- 23K256 SRAM commands ----------------
#define RAM_WRITE 0x02
#define RAM_READ  0x03
#define RAM_WRSR  0x01

#define RAM_MODE_SEQUENTIAL 0x40

// ---------------- Wave settings ----------------
#define NUM_POINTS 1000

static inline void cs_select(uint cs_pin) {
    asm volatile("nop \n nop \n nop");
    gpio_put(cs_pin, 0);
    asm volatile("nop \n nop \n nop");
}

static inline void cs_deselect(uint cs_pin) {
    asm volatile("nop \n nop \n nop");
    gpio_put(cs_pin, 1);
    asm volatile("nop \n nop \n nop");
}

void spi_pin_init(void) {
    // SPI0 at 1 MHz
    spi_init(SPI_PORT, 1000 * 1000);

    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    gpio_init(PIN_CS_DAC);
    gpio_set_dir(PIN_CS_DAC, GPIO_OUT);
    gpio_put(PIN_CS_DAC, 1);

    gpio_init(PIN_CS_RAM);
    gpio_set_dir(PIN_CS_RAM, GPIO_OUT);
    gpio_put(PIN_CS_RAM, 1);
}

// MCP4912 10-bit DAC command
// channel 0 = A, channel 1 = B
uint16_t dac_command(uint8_t channel, uint16_t value) {
    if (value > 1023) {
        value = 1023;
    }

    uint16_t command = 0;

    // bit 15: 0 = A, 1 = B
    if (channel == 1) {
        command |= 0x8000;
    }

    // bit 14 BUF = 1
    // bit 13 GA  = 1, 1x gain
    // bit 12 SHDN = 1, active
    command |= 0x7000;

    // MCP4912 data bits are D9:D0 in bits 11:2
    command |= (value << 2);

    return command;
}

void dac_write_bytes(uint8_t data[2]) {
    cs_select(PIN_CS_DAC);
    spi_write_blocking(SPI_PORT, data, 2);
    cs_deselect(PIN_CS_DAC);
}

void dac_write_raw(uint16_t command) {
    uint8_t data[2];
    data[0] = (command >> 8) & 0xFF;
    data[1] = command & 0xFF;
    dac_write_bytes(data);
}

void spi_ram_init(void) {
    // Set 23K256 SRAM into sequential mode
    uint8_t data[2];
    data[0] = RAM_WRSR;
    data[1] = RAM_MODE_SEQUENTIAL;

    cs_select(PIN_CS_RAM);
    spi_write_blocking(SPI_PORT, data, 2);
    cs_deselect(PIN_CS_RAM);
}

void ram_write_bytes(uint16_t address, uint8_t *data, uint16_t length) {
    uint8_t header[3];

    header[0] = RAM_WRITE;
    header[1] = (address >> 8) & 0xFF;
    header[2] = address & 0xFF;

    cs_select(PIN_CS_RAM);
    spi_write_blocking(SPI_PORT, header, 3);
    spi_write_blocking(SPI_PORT, data, length);
    cs_deselect(PIN_CS_RAM);
}

void ram_read_bytes(uint16_t address, uint8_t *data, uint16_t length) {
    uint8_t header[3];

    header[0] = RAM_READ;
    header[1] = (address >> 8) & 0xFF;
    header[2] = address & 0xFF;

    cs_select(PIN_CS_RAM);
    spi_write_blocking(SPI_PORT, header, 3);
    spi_read_blocking(SPI_PORT, 0x00, data, length);
    cs_deselect(PIN_CS_RAM);
}

void store_sine_wave_to_ram(void) {
    for (int i = 0; i < NUM_POINTS; i++) {
        // 0V to 3.3V sine wave
        float voltage = 1.65f + 1.65f * sinf(2.0f * M_PI * i / NUM_POINTS);

        // Convert voltage to MCP4912 10-bit value
        uint16_t dac_value = (uint16_t)((voltage / 3.3f) * 1023.0f);

        // Convert to 16-bit MCP4912 command
        uint16_t command = dac_command(0, dac_value);

        // Split into two bytes
        uint8_t bytes[2];
        bytes[0] = (command >> 8) & 0xFF;
        bytes[1] = command & 0xFF;

        // Store each DAC command in SRAM, 2 bytes per point
        ram_write_bytes(i * 2, bytes, 2);
    }
}

int main() {
    stdio_init_all();
    sleep_ms(1000);

    spi_pin_init();
    spi_ram_init();

    // Calculate 1000 sine values and save them into external RAM
    store_sine_wave_to_ram();

    while (true) {
        for (int i = 0; i < NUM_POINTS; i++) {
            uint8_t bytes[2];

            // Read two bytes from RAM
            ram_read_bytes(i * 2, bytes, 2);

            // Apply them directly to the SPI DAC
            dac_write_bytes(bytes);

            // 1000 points * 1 ms = 1 second = 1 Hz
            sleep_ms(1);
        }
    }
}