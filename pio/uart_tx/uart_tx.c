/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "uart_tx.pio.h"
#include "hardware/gpio.h"
#define CRC32_INIT                  ((uint32_t)-1l)

#define RH_ASK_PREAMBLE_LEN         8
#define DATA_TO_CHECK_LEN           9
#define CRC32_LEN                   4
#define TOTAL_LEN                   (DATA_TO_CHECK_LEN + CRC32_LEN)
#define T_LEN                       21

// Initialise the first 8 nibbles of the tx buffer to be the standard
// preamble. We will append messages after that. 0x38, 0x2c is the start symbol before
// 6-bit conversion to RH_ASK_START_SYMBOL
uint8_t preamble[RH_ASK_PREAMBLE_LEN] = {0x2a, 0x2a, 0x2a, 0x2a, 0x2a, 0x2a, 0x38, 0x2c};

// commonly used crc test data and also space for the crc value
static uint8_t src[TOTAL_LEN] = { 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x00, 0x00, 0x00, 0x00 };
static uint8_t dummy_dst[1];

// This uses a standard polynomial with the alternate 'reversed' shift direction.
// It is possible to use a non-reversed algorithm here but the DMA sniff set-up
// below would need to be modified to remain consistent and allow the check to pass.

static uint32_t soft_crc32_block(uint32_t crc, uint8_t *bytp, uint32_t length) {
    while(length--) {
        uint32_t byte32 = (uint32_t)*bytp++;

        for (uint8_t bit = 8; bit; bit--, byte32 >>= 1) {
            crc = (crc >> 1) ^ (((crc ^ byte32) & 1ul) ? 0xEDB88320ul : 0ul);
        }
    }
    return crc;
}

// We're going to use PIO to print "Hello, world!" on the same GPIO which we
// normally attach UART0 to.
#define PIO_TX_PIN 14

// Check the pin is compatible with the platform
#if PIO_TX_PIN >= NUM_BANK0_GPIOS
#error Attempting to use a pin>=32 on a platform that does not support it
#endif


int main() {
    // This is the same as the default UART baud rate on Pico
    int i, j;
    const uint SERIAL_BAUD = 2400;
    const uint PIO_SERIAL_BAUD = 2400;
    bool invert = true;
    PIO pio;
    uint sm;
    uint offset;
    uint32_t crc_res;
    uint8_t out_buf[T_LEN], in_buf[T_LEN];
    // calculate and append the crc
    crc_res = soft_crc32_block(CRC32_INIT, src, DATA_TO_CHECK_LEN);
    printf("crc_res 0x%x \n",crc_res);
    *((uint32_t *)&src[DATA_TO_CHECK_LEN]) = crc_res;
    
    for (i = 0; i < RH_ASK_PREAMBLE_LEN ; i++)
    {
        out_buf[i] = preamble[i];
    }
    for (i = 8; i < T_LEN ; i++)
    {
        j = i - 8;
        out_buf[i] = src[j];
    }
    
    for (int i = 0; i < T_LEN; i++) printf("0x%x ",out_buf[i]);
    printf(" \n");


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
         
        uart_tx_program_puts(pio, sm,out_buf); 
        uart_tx_program_puts(pio, sm,"\n");
        sleep_ms(10*1000);
    }

    // This will free resources and unload our program
    pio_remove_program_and_unclaim_sm(&uart_tx_program, pio, sm, offset);
}
