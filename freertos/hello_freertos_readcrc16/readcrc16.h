#ifndef	READCRC16_H
#define	READCRC16_H
#include <FreeRTOS.h>
#include <task.h>
#include <stream_buffer.h>
#include <stdio.h>
#include <queue.h>
#include <string.h>
#include "pico/stdlib.h"
#include "semphr.h"
#include "event_groups.h"
#include "lifting.h"
#include "crc16.h"
#include "crc.h"
#include "head-tail.h"
#include "klt.h"
#include "comprogs.h"
/*adding pshell */
//#include "pshell.h"
#include "xreceive.h"
#include "xtransmit.h"
#include "hardware/watchdog.h"

#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pico_hal.h"
 
//#include "tusb.h"
//#include "vi.h"
#include "pnmio.h"
#include "error.h"
/* vi: set sw=4 ts=4: */
/* SPDX-License-Identifier: GPL-3.0-or-later */

/* Copyright (C) 1883 Thomas Edison - All Rights Reserved
 * You may use, distribute and modify this code under the
 * terms of the BSD 3 clause license, which unfortunately
 * won't be written for another century.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * A little flash file system manager for the Raspberry Pico
 */

#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "hardware/watchdog.h"

#include "pico/stdio.h"
#include "pico/stdlib.h"

//#include "fs.h"
//#include "tusb.h"
#include "vi.h"
#include "xreceive.h"
#include "xtransmit.h"

#define MAX_ARGS 4

#define VT_ESC "\033"
#define VT_CLEAR VT_ESC "[H" VT_ESC "[J"

typedef char buf_t[128];

typedef void (*cmd_func_t) (void);
typedef struct
{
  const char *name;
  cmd_func_t func;
  const char *descr;
} cmd_t;

//int l = lfs_file_size (&in), charcnt = 0, charsent = 0;

// clang-format off



// clang-format on
/*
static const char *
search_cmds (int len)
{
  if (len == 0)
    return NULL;
  int i, last_i, count = 0;
  for (i = 0; i < sizeof cmd_table / sizeof cmd_table[0]; i++)
    if (strncmp (cmd_buffer, cmd_table[i].name, len) == 0)
      {
	last_i = i;
	count++;
      }
  if (count != 1)
    return NULL;
  return cmd_table[last_i].name + len;
}
*/

static uint32_t screen_x = 80, screen_y = 24;
static lfs_file_t file;
static buf_t cmd_buffer, curdir, path, result;
static int argc;
static char *argv[MAX_ARGS + 1];
static bool mounted = false, run = true;
static void
parse_cmd (void);

char *
full_path (const char *name);

static void
xmodem_cb (uint8_t * buf, uint32_t len);

static bool
check_mount (bool need);

static bool
check_name (void);

static void
put_cmd (void);

int
check_cp_parms (char **from, char **to, int copy);

static void
mv_cmd (void);

static void
cp_cmd (void);

static void
get_cmd (void);

static void
mkdir_cmd (void);

static void
rm_cmd (void);

static void
mount_cmd (void);

static void
unmount_cmd (void);

static void
format_cmd (void);

static void
status_cmd (void);

static void
ls_cmd (void);

static void
cd_cmd (void);

static void
j2k_cmd (void);

static void
test_pnmio_cmd (void);

static void
lsklt_cmd (void);

static void
quit_cmd (void);

static bool
stdio_init (int uart_rx_pin);

static bool
screen_size (void);

/***********************needs to be in a header***********************/
#define STORAGE_SIZE_BYTES 100

#define TASK1_BIT  (1UL << 0UL)	//zero
#define TASK2_BIT  (1UL << 1UL)	//1
#define TASK3_BIT  (1UL << 2UL)	//2
#define TASK4_BIT  (1UL << 3UL)	//3
#define TASK5_BIT  (1UL << 4UL)	//4
#define TASK6_BIT  (1UL << 5UL)	//5

/*Used to dimension the array used to hold the streams.
The availble space is 1 less than this */
static uint8_t ucBufferStorage[STORAGE_SIZE_BYTES];

/*The varaible used to hold the stream buffer structure*/
StaticStreamBuffer_t xStreamBufferStruct;

//StreamBufferHandle_t xStreamBuffer;
StreamBufferHandle_t DynxStreamBuffer;

//const size_t xStreamBufferSizeBytes = 100,xTriggerLevel = 10;
//xStreamBuffer = xStreamBufferCreate(xStreamBufferSizeBytes,xTriggerLevel);
static QueueHandle_t xQueue = NULL;

int streamFlag, ii, received, processed, j, m;
size_t numbytes1;
size_t numbytes2;
uint8_t *pucRXData;
size_t rdnumbytes1;
size_t Event = 0;

static SemaphoreHandle_t mutex;
//static SemaphoreHandle_t mutex1;
//static SemaphoreHandle_t mutex2;

EventGroupHandle_t xCreatedEventGroup;

// define a variable which holds the state of events 
const EventBits_t xBitsToWaitFor =
  (TASK1_BIT | TASK2_BIT | TASK3_BIT | TASK4_BIT);
EventBits_t xEventGroupValue;





int
read_tt (char *head, char *endofbuf, char *topofbuf);

unsigned char userInput;

unsigned char recCRC;
unsigned char message[3] = { 0xd3, 0x01, 0x00 };

int flag = 0, numofchars, error = 0, syncflag = 1, rdyflag = 1, testsx =
  10, testsx1 = 10, syncdone = 0;

unsigned char inpbuf[imgsize * 2];
unsigned char *img1, *img2;
/***********************needs to be in a header***********************/

void
led_task (void *pvParameters);
#endif
