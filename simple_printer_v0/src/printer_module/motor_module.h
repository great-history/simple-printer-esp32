#ifndef __MOTOR_MODULE_H_
#define __MOTOR_MODULE_H_

#include "config_setup.h"

void motor_run_step(uint32_t steps);
void motor_start();
void motor_stop();
void init_motor();
void motor_run();

#endif
