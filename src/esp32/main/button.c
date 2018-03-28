#include "include/button.h"

int get_button_state(void)
{
    esp_adc_cal_characteristics_t characteristics;
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_TEST_CHANNEL, ADC_ATTEN_DB_0);
    esp_adc_cal_get_characteristics(V_REF, ADC_ATTEN_DB_0, ADC_WIDTH_BIT_12, &characteristics);
    uint32_t voltage;

    voltage = adc1_to_voltage(ADC1_TEST_CHANNEL, &characteristics);
    printf("Voltage on button is %d mV\n", voltage);

    return (int)voltage > 800 ? 1 : 0;
}
