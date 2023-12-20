#ifndef __MOTOR_MODULE_H_
#define __MOTOR_MODULE_H_

// #include "em_config.h"
#include <Arduino.h>

//V3 电机引脚
#define PIN_MOTOR_AP 23
#define PIN_MOTOR_AM 22
#define PIN_MOTOR_BP 21
#define PIN_MOTOR_BM 19

//打印头电源升压控制引脚
#define PIN_VHEN 17
#define MOTOR_WATI_TIME 4000    //电机一步时间
#define us_delay(ms) delayMicroseconds(ms)

void motor_run_step(uint32_t steps);
void motor_start();
void motor_stop();
void init_motor();
void motor_run();

#endif
