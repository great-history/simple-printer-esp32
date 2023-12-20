#ifndef __ADC_MODULE_H_
#define __ADC_MODULE_H_

#include <Arduino.h>
#define PIN_ADC_EN 4
#define PIN_BATTERY_ADC 34 //电量相关引脚
#define BATTERY_ADC_BIT 12 //set the resolution to 12 bits (0-4096)
#define PIN_TEMPERATRUE 36 //热敏电阻

void adc_init();
int get_adc_volts();
void em_adc_test();
float get_adc_temperatrue();

#endif