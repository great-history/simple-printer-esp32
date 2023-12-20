#ifndef __MODULE_MONITOR_H_
#define __MODULE_MONITOR_H_

// #include "em_config.h"
#include "state_module.h"
#include "adc_module.h"

//缺纸检测引脚
#define PIN_PAPER 35
#define PIN_BEEP 18 //蜂鸣器
#define PIN_LED 18 //LED灯

typedef enum{
    BEEP_CONNECT = 0,
    BEEP_WARN,
    BEEP_PRINTER_START,
}beep_type_e;

typedef enum{
    LED_CONNECT = 0,
    LED_WARN,
    LED_PRINTER_START,
}led_type_e;


void run_beep(beep_type_e type);

void run_led(led_type_e type);

void read_all_hal();

void init_hal();

bool read_paper_irq_need_report_status();


#endif