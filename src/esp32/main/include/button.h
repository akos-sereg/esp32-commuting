#ifndef __button_h_included__
#define __button_h_included__

#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_system.h"
#include "esp_adc_cal.h"

#define V_REF   1100
#define ADC1_TEST_CHANNEL (ADC1_CHANNEL_6)      // for GPIO 34

extern int get_button_state(void);

#endif