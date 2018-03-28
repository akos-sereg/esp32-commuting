#include "include/display.h"

/* ESP32 PIN numbers of 2x7 segment display */
int digits[2][7] = {
    { DIGIT_1_A, DIGIT_1_B, DIGIT_1_C, DIGIT_1_D, DIGIT_1_E, DIGIT_1_F, DIGIT_1_G },
    { DIGIT_2_A, DIGIT_2_B, DIGIT_2_C, DIGIT_2_D, DIGIT_2_E, DIGIT_2_F, DIGIT_2_G }
};

/* used to make 7 segment display blank */
int blank_segments[7] = { 0, 0, 0, 0, 0, 0, 0 };

/* represents an ASCII character displayed on 7 segment display */
struct char_map_item {
    char value;
    int segments[7];
};

/* ASCII character to 7 segment display */
#define CHAR_MAP_SIZE	17
struct char_map_item char_map[CHAR_MAP_SIZE] = {
    { .value = 'L', .segments = { 0, 0, 0, 1, 1, 1, 0 } },
    { .value = 'o', .segments = { 0, 0, 1, 1, 1, 0, 1 } },
    { .value = 'A', .segments = { 1, 1, 1, 0, 1, 1, 1 } },
    { .value = 'd', .segments = { 0, 1, 1, 1, 1, 0, 1 } },
    { .value = 'i', .segments = { 0, 0, 1, 0, 0, 0, 0 } },
    { .value = 'n', .segments = { 0, 0, 1, 0, 1, 0, 1 } },
    { .value = 'g', .segments = { 1, 1, 1, 1, 0, 1, 1 } },
    { .value = ' ', .segments = { 0, 0, 0, 0, 0, 0, 0 } },
    { .value = 'b', .segments = { 0, 0, 1, 1, 1, 1, 1 } },
    { .value = 'l', .segments = { 0, 0, 0, 1, 1, 1, 0 } },
    { .value = 'u', .segments = { 0, 0, 1, 1, 1, 0, 0 } },
    { .value = 'e', .segments = { 1, 0, 0, 1, 1, 1, 1 } },
    { .value = 't', .segments = { 0, 0, 0, 1, 1, 1, 1 } },
    { .value = 'h', .segments = { 0, 0, 1, 0, 1, 1, 1 } },
    { .value = 'c', .segments = { 0, 0, 0, 1, 1, 0, 1 } },
    { .value = 'E', .segments = { 1, 0, 0, 1, 1, 1, 1 } },
    { .value = 'r', .segments = { 0, 0, 0, 0, 1, 0, 1 } },
};

int *_fc_get_segments_of(char c) {

    int i;

    for (i=0; i!=CHAR_MAP_SIZE; i++) {
	if (c == char_map[i].value) {
	    return char_map[i].segments;
	}
    }

    return blank_segments;
}

void fc_setup_display() {

    printf("Initializing 7 segment display driver PINs\n");
    gpio_set_direction(DIGIT_1_A, GPIO_MODE_OUTPUT);
    gpio_set_direction(DIGIT_1_B, GPIO_MODE_OUTPUT);
    gpio_set_direction(DIGIT_1_C, GPIO_MODE_OUTPUT);
    gpio_set_direction(DIGIT_1_D, GPIO_MODE_OUTPUT);
    gpio_set_direction(DIGIT_1_E, GPIO_MODE_OUTPUT);
    gpio_set_direction(DIGIT_1_F, GPIO_MODE_OUTPUT);
    gpio_set_direction(DIGIT_1_G, GPIO_MODE_OUTPUT);

    gpio_pad_select_gpio(DIGIT_2_A);
    gpio_set_direction(DIGIT_2_A, GPIO_MODE_OUTPUT);

    gpio_pad_select_gpio(DIGIT_2_B);
    gpio_set_direction(DIGIT_2_B, GPIO_MODE_OUTPUT);

    gpio_set_direction(DIGIT_2_C, GPIO_MODE_OUTPUT);
    gpio_set_direction(DIGIT_2_D, GPIO_MODE_OUTPUT);
    gpio_set_direction(DIGIT_2_E, GPIO_MODE_OUTPUT);
    gpio_set_direction(DIGIT_2_F, GPIO_MODE_OUTPUT);
    gpio_set_direction(DIGIT_2_G, GPIO_MODE_OUTPUT);
    gpio_set_direction(DIGIT_1_DOT, GPIO_MODE_OUTPUT);

    // unused
    gpio_pad_select_gpio(LED_1);
    gpio_set_direction(LED_1, GPIO_MODE_OUTPUT);

    fc_blank_digits();
}

void fc_set_led(int value) {
    gpio_set_level(LED_1, value);
}

void fc_display_digit(int digit, int *segments) {
    int i=0;

    for (int i=0; i!=7; i++) {
	gpio_set_level(digits[digit][i], !segments[i]);
    }
}

void fc_blank_digits() {

    fc_display_digit(0, blank_segments);
    fc_display_digit(1, blank_segments);

    gpio_set_level(DIGIT_1_DOT, 1);
}

void fc_print_waiting() {
    int i=0;

    for (i=0; i!=8; i++) {
	fc_blank_digits();
	switch(i) {
	    case 0: gpio_set_level(DIGIT_1_E, 0); break;
	    case 1: gpio_set_level(DIGIT_1_F, 0); break;
	    case 2: gpio_set_level(DIGIT_1_A, 0); break;
	    case 3: gpio_set_level(DIGIT_2_A, 0); break;
	    case 4: gpio_set_level(DIGIT_2_B, 0); break;
	    case 5: gpio_set_level(DIGIT_2_C, 0); break;
	    case 6: gpio_set_level(DIGIT_2_D, 0); break;
	    case 7: gpio_set_level(DIGIT_1_D, 0); break;
	}
	vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void fc_print_string(char message[]) {
    int i = 0;
    int len = strlen(message);

    while (i < len - 1) {
	fc_display_chars(message[i], message[i+1]);
	vTaskDelay(500 / portTICK_PERIOD_MS);
	i++;
    }
}

void fc_display_chars(char c1, char c2) {
    int i;
    int *segmentsForDigit1 = _fc_get_segments_of(c1);
    int *segmentsForDigit2 = _fc_get_segments_of(c2);

    fc_display_digit(0, segmentsForDigit1);
    fc_display_digit(1, segmentsForDigit2);

    gpio_set_level(DIGIT_1_DOT, 1);
}

void fc_print_digit(double value) {

    int digit1;
    int digit2;
    int displayDot = 0;

    printf("Print digit %.2f", value);

    if (value > 99 || value < 0.0) {
	// unable to display value in 2x7 segment display, showing "- -" instead
	fc_blank_digits();
	gpio_set_level(DIGIT_1_G, 0);
	gpio_set_level(DIGIT_2_G, 0);
	return;
    }

#if FLOAT_PRECISION
    if (value >= 10) {
	digit1 = (int)((int)value / 10);
	digit2 = (int)((((int)value * 10) - (digit1 * 100)) / 10);
	displayDot = 0;
    }
    else {
	digit1 = (int)value;
	digit2 = floor((int)(value * 10) % 10);
	displayDot = 1;
    }
#else
    if (value >= 10) {
	digit1 = (int)((int)value / 10);
	digit2 = (int)((((int)value * 10) - (digit1 * 100)) / 10);
	displayDot = 0;
    }
    else {
	digit2 = (int)value;
	digit1 = 10;
	displayDot = 0;
    }
#endif

    printf("Printing value %f, %d and %d with dot %d'\n", value, digit1, digit2, displayDot);

    fc_display_digit(0, digit[digit1]);
    fc_display_digit(1, digit[digit2]);

    gpio_set_level(DIGIT_1_DOT, !displayDot);
}

