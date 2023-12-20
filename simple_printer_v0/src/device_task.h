#ifndef __DEVICE_TASK_H_
#define __DEVICE_TASK_H_

#include "modules_mixed.h"
#define START_PRINTER_WHEN_FINISH_RAED 1 //接收完成所有数据才开始打印

void init_task();
void printer_run();

#endif