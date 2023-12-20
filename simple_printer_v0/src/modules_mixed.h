#ifndef __MODULE_MIXED_
#define __MODULE_MIXED_

// #include "em_config.h"
// ************************************************** ble module ************************************************** //
#include <BLEDevice.h>
#include <BLE2902.h>
#include "./ble_module/module_ble.h"
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"         // 自定义打印服务UUID
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"  // 自定义打印特征UUID
#define BLE_NAME "Mini-Printer"

// ************************************************** key module ************************************************** //
#include "key_module/module_key.h"

// ************************************************** monitor module ************************************************** //
#include "./monitor_module/module_monitor.h"

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