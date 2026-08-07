// Copyright (c) 2021 Michael Stoops. All rights reserved.
// Portions copyright (c) 2021 Raspberry Pi (Trading) Ltd.
// 
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the 
// following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following
//    disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the
//    following disclaimer in the documentation and/or other materials provided with the distribution.
// 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote
//    products derived from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
// INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE 
// USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// SPDX-License-Identifier: BSD-3-Clause
//
// Example of an SPI bus slave using the PL022 SPI interface

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"
#define CRC32_INIT                  ((uint32_t)-1l)
#define DATA_TO_CHECK_LEN           9
#define CRC32_LEN                   4
#define TOTAL_LEN                   (DATA_TO_CHECK_LEN + CRC32_LEN)

// commonly used crc test data and also space for the crc value
static uint8_t src[TOTAL_LEN] = { 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x00, 0x00, 0x00, 0x00 };
static uint8_t dummy_dst[1];
#define BUF_LEN         0xd
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

void printbuf(uint8_t buf[], size_t len) {
    size_t i;
    for (i = 0; i < len; ++i) {
        if (i % 16 == 15)
            printf("%02x\n", buf[i]);
        else
            printf("%02x ", buf[i]);
    }

    // append trailing newline if there isn't one
    if (i % 16) {
        putchar('\n');
    }
}


int main() {
    // Enable UART so we can print
    stdio_init_all();
    uint32_t crc_res;
    uint8_t out_buf[TOTAL_LEN], in_buf[TOTAL_LEN];
    crc_res = soft_crc32_block(CRC32_INIT, src, DATA_TO_CHECK_LEN);
    *((uint32_t *)&src[DATA_TO_CHECK_LEN]) = crc_res;
    // calculate and append the crc
    crc_res = soft_crc32_block(CRC32_INIT, src, DATA_TO_CHECK_LEN);
    *((uint32_t *)&src[DATA_TO_CHECK_LEN]) = crc_res;
    printf("crc_res 0x%x \n",crc_res);
    for (int i = 0; i < TOTAL_LEN; i++) out_buf[i] = src[i];

    for (int i = 0; i < TOTAL_LEN; i++) printf("0x%x ",out_buf[i]);
    printf(" \n");
#if !defined(spi_default) || !defined(PICO_DEFAULT_SPI_SCK_PIN) || !defined(PICO_DEFAULT_SPI_TX_PIN) || !defined(PICO_DEFAULT_SPI_RX_PIN) || !defined(PICO_DEFAULT_SPI_CSN_PIN)
#warning spi/spi_slave example requires a board with SPI pins
    puts("Default SPI pins were not defined");
#else

    printf("SPI slave example\n");

    // Enable SPI 0 at 100 KHz and connect to GPIOs
    spi_init(spi_default, 1000 * 1000);
    spi_set_slave(spi_default, true);
    gpio_set_function(PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_CSN_PIN, GPIO_FUNC_SPI);
    // Make the SPI pins available to picotool
    bi_decl(bi_4pins_with_func(PICO_DEFAULT_SPI_RX_PIN, PICO_DEFAULT_SPI_TX_PIN, PICO_DEFAULT_SPI_SCK_PIN, PICO_DEFAULT_SPI_CSN_PIN, GPIO_FUNC_SPI));

    

    // Initialize output buffer
    //for (size_t i = 0; i < BUF_LEN; ++i) {
        // bit-inverted from i. The values should be: {0xff, 0xfe, 0xfd...}
        //out_buf[i] = ~i;
    //}

    printf("SPI slave says: When reading from MOSI, the following buffer will be written to MISO:\n");
    printbuf(out_buf, BUF_LEN);
    size_t i = 0;
    int flag = 1;
    while(flag)
    {
    //for (size_t i = 0; ; ++i) {
        // Write the output buffer to MISO, and at the same time read from MOSI.
        spi_write_read_blocking(spi_default, out_buf, in_buf, TOTAL_LEN);
        printf("out_buf\n");
        for (int i = 0; i < TOTAL_LEN; i++) printf("0x%x ",out_buf[i]);
        printf(" \n");
        printf("in_buf\n");
        for (int i = 0; i < TOTAL_LEN; i++) printf("0x%x ",in_buf[i]);
        printf(" \n");
        // Write to stdio whatever came in on the MOSI line.
        //printf("SPI slave says: read page %d from the MOSI line:\n", i);
        //printbuf(in_buf, BUF_LEN);
    }
#endif
}
