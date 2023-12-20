#ifndef __SPI_MODULE_H_
#define __SPI_MODULE_H_

// #include "em_config.h"
#include <SPI.h>
//打印头数据引脚
#define PIN_SCK 15
#define PIN_SDA 13

// void spiCommand(byte data);

void spiCommand(uint8_t *data_buffer, uint8_t data_len);

void init_spi();

#endif