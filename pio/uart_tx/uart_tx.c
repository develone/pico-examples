/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "uart_tx.pio.h"
#include "hardware/gpio.h"

// We're going to use PIO to print "Hello, world!" on the same GPIO which we
// normally attach UART0 to.
#define PIO_TX_PIN 14

// Check the pin is compatible with the platform
#if PIO_TX_PIN >= NUM_BANK0_GPIOS
#error Attempting to use a pin>=32 on a platform that does not support it
#endif


int main() {
    // This is the same as the default UART baud rate on Pico
    const uint SERIAL_BAUD = 2400;
    const uint PIO_SERIAL_BAUD = 2400;
    bool invert = true;
    PIO pio;
    uint sm;
    uint offset;

    // This will find a free pio and state machine for our program and load it for us
    // We use pio_claim_free_sm_and_add_program_for_gpio_range (for_gpio_range variant)
    // so we will get a PIO instance suitable for addressing gpios >= 32 if needed and supported by the hardware
    bool success = pio_claim_free_sm_and_add_program_for_gpio_range(&uart_tx_program, &pio, &sm, &offset, PIO_TX_PIN, 1, true);
    hard_assert(success);

    uart_tx_program_init(pio, sm, offset, PIO_TX_PIN, PIO_SERIAL_BAUD);

    while (true) {
	if ( invert == false) 
        {
	    // Invert the output logic hardware-wide
            gpio_set_outover(PIO_TX_PIN, GPIO_OVERRIDE_INVERT);
    
	    invert = true;
	}
        uart_tx_program_puts(pio, sm, "Hello, world! (from PIO! Pin GPIO-14 )\r\n");
        sleep_ms(1000);
    }

    // This will free resources and unload our program
    pio_remove_program_and_unclaim_sm(&uart_tx_program, pio, sm, offset);
}
