#ifndef __MODULE_MIXED_
#define __MODULE_MIXED_

#include "config_setup.h"
// ************************************************** ble module ************************************************** //
#include <BLEDevice.h>
#include <BLE2902.h>
#include "ble_module/module_ble.h"
// ************************************************** key module ************************************************** //
#include "key_module/module_key.h"
// ************************************************** monitor module ************************************************** //
#include "monitor_module/module_monitor.h"
// ************************************************** printer module ************************************************** //
#include "printer_module/module_printer.h"

extern bool printer_test;

bool get_ble_connect();
void ble_report();
void init_ble();
void start_printing(uint8_t *data, uint32_t len);
void start_printing_by_queuebuf();
void testSTB();
void init_key();

#endif