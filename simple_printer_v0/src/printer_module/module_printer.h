#ifndef __MODULE_PRINTER_H_
#define __MODULE_PRINTER_H_

#include "motor_module.h"
#include "spi_module.h"
#include "timer_module.h"

#define PIN_LAT 12 // 数据锁存时间间隔
//STB通道
#define PIN_STB1 26
#define PIN_STB2 27  
#define PIN_STB3 14  
#define PIN_STB4 32  
#define PIN_STB5 33  
#define PIN_STB6 25

#define LAT_TIME 1 //数据锁存时间
#define PRINT_TIME 2600         //打印加热时间  5700
#define PRINT_END_TIME 200      //冷却时间

//每行总点数
#define TPH_DOTS_PER_LINE 384
//每行字节长度 384/8
#define TPH_DI_LEN 48
//所有通道打印
#define ALL_STB_NUM 0xFF

// 点数-增加时间系数
#define kAddTime 0.001
// 根据打印头实际打印效果修改打印时间偏移值
//  #define STB1_ADDTIME 200
//  #define STB2_ADDTIME 200
//  #define STB3_ADDTIME -200
//  #define STB4_ADDTIME 0
//  #define STB5_ADDTIME 100
//  #define STB6_ADDTIME 500
#define STB1_ADDTIME 0
#define STB2_ADDTIME 0
#define STB3_ADDTIME 0
#define STB4_ADDTIME 0
#define STB5_ADDTIME 0
#define STB6_ADDTIME 0

void init_printing();
void set_heat_density(uint8_t density);
void clearAddTime();
void send_one_line_data(uint8_t *data);
bool move_and_start_std(bool need_stop, uint8_t stbnum);
void init_printer();

#endif