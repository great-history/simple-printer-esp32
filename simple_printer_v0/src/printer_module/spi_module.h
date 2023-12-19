#ifndef __SPI_MODULE_H_
#define __SPI_MODULE_H_

#include "config_setup.h"
#include <SPI.h>

// void spiCommand(byte data);
void spiCommand(uint8_t *data_buffer, uint8_t data_len);
void init_spi();

#endif