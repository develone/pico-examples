/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/pio.h"
#include "hardware/uart.h"
#include "uart_rx.pio.h"
#include "hardware/gpio.h"
#include <stdint.h>
#define CRC32_INIT                  ((uint32_t)-1l) 
#define DATA_TO_CHECK_LEN           9
#define CRC32_LEN                   4
#define TOTAL_LEN                   (DATA_TO_CHECK_LEN + CRC32_LEN)
static uint8_t src[TOTAL_LEN] = { 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x00, 0x00, 0x00, 0x00 };
uint16_t crchirec;
uint16_t crclorec;
uint16_t crchirecR;
uint16_t crclorecR;
uint32_t crc_res;
uint32_t crc_res_sav;
uint8_t hihi;
uint8_t hilo;
uint8_t lohi;
uint8_t lolo;
char c;
int i,j;
static uint8_t src1[TOTAL_LEN] = { 0x39, 0x38, 0x37, 0x36, 0x35, 0x34, 0x33, 0x32, 0x31, 0x00, 0x00, 0x00, 0x00 };
static uint8_t srcR[TOTAL_LEN];
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
#include <stdint.h>

uint16_t swap_uint16(uint16_t val) {
    return (val << 8) | (val >> 8);
}
void ck_crc(void)
{
    crc_res = soft_crc32_block(CRC32_INIT, srcR, DATA_TO_CHECK_LEN);
    //crc_res_sav = crc_res;
    printf("crc_res 0x%x \n", crc_res);
    crclorec = crc_res & 0x0000ffff;
    printf("crclorec  0x%x \n", crclorec);
    crc_res_sav = crc_res & 0xffff0000;
    //printf("crc_res_sav  0x%x \n", crc_res_sav);
    unsigned int shift_16 = 16;
    crchirec = crc_res_sav >> shift_16;
    printf("crchirec  0x%x \n", crchirec);
    //for(i=0;i<DATA_TO_CHECK_LEN;i++) printf("%c ",srcR[i]);
    //printf("\n");
    unsigned int shift_8 = 8;
    uint8_t cks = 0;
     
    uint8_t crclo;
   
    // upper 8 bits shifted lower 8 bits
    hihi = crchirec >> shift_8;
    crclo = crchirec & 0x00ff;
    hilo = crclo;
    
 
    // upper 8 bits shifted lower 8 bits
    lohi = crclorec >> shift_8;
    crclo = crclorec & 0x00ff;
    lolo = crclo;
    
    printf("hihi 0x%x hilo 0x%x lohi 0x%x lolo 0x%x \n", hihi, hilo, lohi, lolo);
    if (hihi == srcR[9]) cks = 1;
    if (hilo == srcR[10]) cks = 2;
    if (lohi == srcR[11]) cks = 3;
    if (lolo == srcR[12]) cks = 4;
    if (cks == 4)
    {
        for(i=0;i<DATA_TO_CHECK_LEN;i++) printf("%c ",srcR[i]);
	printf("\n");
    }
    else
    {
	cks = 0;
	crchirecR = srcR[9];
	crchirecR = crchirecR << shift_8;
	crchirecR = crchirecR + srcR[10];
	crchirecR = swap_uint16(crchirecR);
	printf("crchirecR  0x%x \n", crchirecR);
	crclorecR = srcR[11];
	crclorecR = crclorecR << shift_8;
	crclorecR = crclorecR + srcR[12];
	crclorecR = swap_uint16(crclorecR);
	printf("crclorecR  0x%x \n", crclorecR);
	if ( crclorec == crchirecR) cks = 1;
	if ( crchirec == crclorecR) cks = 2;
	if (cks == 2)
	{
	    for(i=0;i<DATA_TO_CHECK_LEN;i++) printf("%c ",srcR[i]);
	    printf("\n");
	}
	
        	
    }
}

// This program
// - Uses UART1 (the spare UART, by default) to transmit some text
// - Uses a PIO state machine to receive that text
// - Prints out the received text to the default console (UART0)
// This might require some reconfiguration on boards where UART1 is the
// default UART.

//#define SERIAL_BAUD PICO_DEFAULT_UART_BAUD_RATE
#define SERIAL_BAUD 2400
#define PIO_SERIAL_BAUD 2400

#define HARD_UART_INST uart1

// You'll need a wire from GPIO4 -> GPIO3
#define HARD_UART_TX_PIN 4
#define PIO_RX_PIN 3

// Check the pin is compatible with the platform
#if PIO_RX_PIN >= NUM_BANK0_GPIOS
#error Attempting to use a pin>=32 on a platform that does not support it
#endif

// Ask core 1 to print a string, to make things easier on core 0
void core1_main() {
    const char *s = (const char *) multicore_fifo_pop_blocking();
    uart_puts(HARD_UART_INST, s);
}

int main() {
    // Console output (also a UART, yes it's confusing)
    setup_default_uart();
    printf("Starting PIO UART RX example\n");

    
    
    // Set up the hard UART we're going to use to print characters
    uart_init(HARD_UART_INST, PIO_SERIAL_BAUD);
    gpio_set_function(HARD_UART_TX_PIN, GPIO_FUNC_UART);

    // Set up the state machine we're going to use to receive them.
    PIO pio;
    uint sm;
    uint offset;
    
    // This will find a free pio and state machine for our program and load it for us
    // We use pio_claim_free_sm_and_add_program_for_gpio_range (for_gpio_range variant)
    // so we will get a PIO instance suitable for addressing gpios >= 32 if needed and supported by the hardware
    bool success = pio_claim_free_sm_and_add_program_for_gpio_range(&uart_rx_program, &pio, &sm, &offset, PIO_RX_PIN, 1, true);
    hard_assert(success);

    uart_rx_program_init(pio, sm, offset, PIO_RX_PIN, SERIAL_BAUD);
    //uart_rx_mini_program_init(pio, sm, offset, PIO_RX_PIN, SERIAL_BAUD);

    // Tell core 1 to print some text to uart1 as fast as it can
    multicore_launch_core1(core1_main);
    const char *text = "Hello, world from PIO! (Plus 2 UARTs and 2 cores, for complex reasons)\n";
    multicore_fifo_push_blocking((uint32_t) text);

    // Echo characters received from PIO to the console
    while (true) {
	
        while ( (c = uart_rx_program_getc(pio, sm)) != 0x2c)
	{
	}
	 
	for(i=0;i < TOTAL_LEN;i++)
	{
	    c = uart_rx_program_getc(pio, sm);
	    putchar(c);
	     
	    
	    
	    srcR[i] = c;
	}
	    
	
	ck_crc();
	    
	    //9-1	0xfea0fdfe	1-9	0x340bc6d9
	    //		fefda0fe		d9c60b34
	    
	
    }

    // This will free resources and unload our program
    //pio_remove_program_and_unclaim_sm(&uart_rx_program, pio, sm, offset);
}
