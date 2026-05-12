#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"

#define UART_ID uart0
#define BAUD_RATE 115200

#define UART_TX_PIN 0   // Pico GP0 = UART0 TX
#define UART_RX_PIN 1   // Pico GP1 = UART0 RX

int main() {
    stdio_init_all();

    sleep_ms(2000);

    printf("Pico bidirectional UART started\r\n");

    uart_init(UART_ID, BAUD_RATE);

    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    absolute_time_t last_send_time = get_absolute_time();

    while (true) {
        // Pico sends message to STM32 every 1 second
        if (absolute_time_diff_us(last_send_time, get_absolute_time()) > 1000000) {
            uart_puts(UART_ID, "Hello STM32 from Pico\r\n");
            printf("Pico sent: Hello STM32 from Pico\r\n");
            last_send_time = get_absolute_time();
        }

        // Pico receives message from STM32
        if (uart_is_readable(UART_ID)) {
            char c = uart_getc(UART_ID);
            printf("%c", c);
        }

        sleep_ms(5);
    }

    return 0;
}