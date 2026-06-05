#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>
#include "pico/stdlib.h"

#define HX_SCK 14
#define HX_DT  15

#define MAX_SAMPLES 2000
#define ALPHA 0.10f

static int32_t raw_data[MAX_SAMPLES];
static float filt_data[MAX_SAMPLES];
static uint32_t time_ms[MAX_SAMPLES];

void hx711_init(void) {
    gpio_init(HX_SCK);
    gpio_set_dir(HX_SCK, GPIO_OUT);
    gpio_put(HX_SCK, 0);

    gpio_init(HX_DT);
    gpio_set_dir(HX_DT, GPIO_IN);
}

int32_t hx711_read(void) {
    uint32_t raw = 0;

    // Wait until HX711 data is ready
    while (gpio_get(HX_DT)) {
        tight_loop_contents();
    }

    // Read 24 bits
    for (int i = 0; i < 24; i++) {
        gpio_put(HX_SCK, 1);
        sleep_us(1);

        raw = raw << 1;
        if (gpio_get(HX_DT)) {
            raw |= 1;
        }

        gpio_put(HX_SCK, 0);
        sleep_us(1);
    }

    // 25th clock pulse: gain = 128 for next reading
    gpio_put(HX_SCK, 1);
    sleep_us(1);
    gpio_put(HX_SCK, 0);
    sleep_us(1);

    // Convert 24-bit signed value to 32-bit signed integer
    if (raw & 0x800000) {
        raw |= 0xFF000000;
    }

    return (int32_t)raw;
}

int main() {
    stdio_init_all();
    sleep_ms(2000);

    hx711_init();

    printf("READY\r\n");

    while (true) {
        int n = 0;

        printf("Enter number of samples:\r\n");

        if (scanf("%d", &n) != 1) {
            continue;
        }

        if (n < 1) {
            n = 1;
        }

        if (n > MAX_SAMPLES) {
            n = MAX_SAMPLES;
        }

        float filt = 0.0f;

        for (int i = 0; i < n; i++) {
            int32_t raw = hx711_read();

            if (i == 0) {
                filt = (float)raw;
            } else {
                filt = ALPHA * (float)raw + (1.0f - ALPHA) * filt;
            }

            raw_data[i] = raw;
            filt_data[i] = filt;
            time_ms[i] = to_ms_since_boot(get_absolute_time());
        }

        printf("BEGIN %d\r\n", n);
        printf("time_ms,raw,filtered\r\n");

        for (int i = 0; i < n; i++) {
            printf("%" PRIu32 ",%" PRId32 ",%.2f\r\n",
                   time_ms[i],
                   raw_data[i],
                   filt_data[i]);
        }

        printf("END\r\n");
    }
}