#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

// SPI0 pins
#define SPI_PORT spi0
#define PIN_MISO 16   // MCP4912 does not use this, but SPI0 default includes it
#define PIN_CS   17
#define PIN_SCK  18
#define PIN_MOSI 19

#define SAMPLE_RATE_HZ 200
#define DAC_MAX 1023
#define PI 3.14159265358979323846

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

// channel = 0 for DAC A, channel = 1 for DAC B
// value = 0 to 1023
void write_dac(uint8_t channel, uint16_t value) {
    value &= 0x03FF;   // MCP4912 is 10-bit

    /*
     MCP4912 16-bit command:
     bit 15: A/B select, 0 = DAC A, 1 = DAC B
     bit 14: BUF, 0 = unbuffered
     bit 13: GA, 1 = 1x gain
     bit 12: SHDN, 1 = active mode
     bits 11-2: 10-bit data
     bits 1-0: don't care
    */

    uint16_t command = 0x3000 | (value << 2);

    if (channel == 1) {
        command |= 0x8000;   // select DAC B
    }

    uint8_t data[2];
    data[0] = (command >> 8) & 0xFF;
    data[1] = command & 0xFF;

    cs_select(PIN_CS);
    spi_write_blocking(SPI_PORT, data, 2);
    cs_deselect(PIN_CS);
}

int main() {
    stdio_init_all();

    // SPI baud rate
    spi_init(SPI_PORT, 1000 * 1000);

    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    // SPI mode 0
    spi_set_format(SPI_PORT, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    // CS pin
    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);

    printf("HW7 MCP4912 DAC running...\n");

    int sample = 0;

    while (true) {
        double t = (double)sample / SAMPLE_RATE_HZ;

        // DAC A: 2 Hz sine wave, 0V to 3.3V
        double sine = 0.5 + 0.5 * sin(2.0 * PI * 2.0 * t);
        uint16_t sine_value = (uint16_t)(sine * DAC_MAX);

        // DAC B: 1 Hz triangle wave, 0V to 3.3V
        double phase = fmod(t, 1.0);
        double triangle;

        if (phase < 0.5) {
            triangle = phase * 2.0;
        } else {
            triangle = 2.0 - phase * 2.0;
        }

        uint16_t triangle_value = (uint16_t)(triangle * DAC_MAX);

        write_dac(0, sine_value);       // VOUTA
        write_dac(1, triangle_value);   // VOUTB

        sample++;

        sleep_us(1000000 / SAMPLE_RATE_HZ);
    }

    return 0;
}