#ifndef __display_h_included__
#define __display_h_included__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

/* if switched on, 3.5 will be displayed as "3.5", otherwise "3" */
#define FLOAT_PRECISION	0

/* esp32 pins used to drive 2x7 segment display */
#define DIGIT_1_A	32
#define DIGIT_1_B	33
#define DIGIT_1_C	25
#define DIGIT_1_D	26
#define DIGIT_1_E	23
#define DIGIT_1_F	22
#define DIGIT_1_G	21

#define DIGIT_2_A	1
#define DIGIT_2_B	3
#define DIGIT_2_C	18
#define DIGIT_2_D	5
#define DIGIT_2_E	17
#define DIGIT_2_F	16
#define DIGIT_2_G	4

/* esp32 pin for dot between digits */
#define DIGIT_1_DOT	0

/* not used */
/*
    2: normal
    15: pad select
    19: normal
    13: pad select
    12: pad select
    14: pad select
    27: normal
*/
#define LED_1	27

static int digit[11][7] = {
    { 1, 1, 1, 1, 1, 1, 0 }, /* 0 */
    { 0, 1, 1, 0, 0, 0, 0 }, /* ... */
    { 1, 1, 0, 1, 1, 0, 1 },
    { 1, 1, 1, 1, 0, 0, 1 },
    { 0, 1, 1, 0, 0, 1, 1 },
    { 1, 0, 1, 1, 0, 1, 1 },
    { 1, 0, 1, 1, 1, 1, 1 },
    { 1, 1, 1, 0, 0, 0, 0 },
    { 1, 1, 1, 1, 1, 1, 1 }, /* ... */
    { 1, 1, 1, 1, 0, 1, 1 }, /* 9 */
    { 0, 0, 0, 0, 0, 0, 0 }  /* blank */
};

extern void fc_setup_display();
extern void fc_print_waiting();
extern void fc_print_digit(double value);
extern void fc_print_string(char message[]);
extern void fc_display_chars(char c1, char c2);
extern void fc_blank_digits();
extern void fc_set_led(int value);


#endif