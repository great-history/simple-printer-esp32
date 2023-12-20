/**
 * @File Name: em_task.cpp
 * @brief  how each module connected to perform a single task
 * @Version : 1.0
 */
#include "device_task.h"

// ************************************************** task ************************************************** //
/**
 * @brief 处理上报相关事件
 */
void run_report()
{
    if (get_state_timeout())
    {
        clean_state_timeout();
        read_all_hal();
        if (get_ble_connect())
        {
            Serial.print("report device status ： report time up\n");
            ble_report();
        }
    }
    if (read_paper_irq_need_report_status())
    {
        // 缺纸中断产生，需要上报
        Serial.print("report device status : paper irq\n");
        ble_report();
    }
}

/**
 * @brief 处理打印相关事件
 */
void run_printer()
{
    device_state_t *pdevice = get_device_state();
    #ifdef START_PRINTER_WHEN_FINISH_RAED
        if (pdevice->read_ble_finish == true)
        {
            if (pdevice->printer_state == PRINTER_STATUS_FINISH ||
                pdevice->printer_state == PRINTER_STATUS_INIT)
            {
                pdevice->read_ble_finish = false;
                pdevice->printer_state = PRINTER_STATUS_START;
                ble_report();
                Serial.printf("report device status : printing start %d\n",get_ble_rx_leftline());
                run_beep(BEEP_PRINTER_START);
                run_led(LED_PRINTER_START);
            }
        }
    #else
    // 接收大于100条时，才触发开始打印
    if (get_ble_rx_leftline()> 100)
    {
        if (pdevice->printer_state == PRINTER_STATUS_FINISH ||
            pdevice->printer_state == PRINTER_STATUS_INIT)
        {
            pdevice->printer_state = PRINTER_STATUS_START;
            ble_report();
            Serial.print("report device status : printing start\n");
            run_beep(BEEP_PRINTER_START);
            run_led(LED_PRINTER_START);
        }
    }
    #endif
    // 开始打印
    if (pdevice->printer_state == PRINTER_STATUS_START)
    {
        // 正常打印
        start_printing_by_queuebuf();
        pdevice->printer_state = PRINTER_STATUS_FINISH;
    }
}

void task_report(void *pvParameters)
{
    for (;;) // A Task shall never return or exit.
    {
        run_report();
        vTaskDelay(100);
    }
}

void task_button(void *pvParameters)
{
    for (;;) // A Task shall never return or exit.
    {
        Button_Process(); // 需要周期调用按键处理函数
        vTaskDelay(20);
    }
}

void printer_run()
{
    for (;;) // A Task shall never return or exit.
    {
        run_printer();
        // vTaskDelay(10);
        if(printer_test){
            printer_test = false;
            testSTB();
        }
    }
}

void init_task()
{
    Serial.begin(115200);
    Serial.print("init_task\n");
    Serial.print("--simple printer V0--\n");
    init_device_state();
    init_timer();
    init_hal();
    init_queue();
    adc_init();
    init_ble();
    init_key();
    init_printer();
    xTaskCreate(
        task_report,  // 任务函数
        "TaskReport", // 任务名
        1024*10,         // 任务栈
        NULL,         // 任务参数
        1,            // 任务优先级, with 3 (configMAX_PRIORITIES - 1) 是最高的，0是最低的.
        NULL          // 任务句柄
    );

    xTaskCreate(
        task_button,  // 任务函数
        "TaskButton", // 任务名
        1024*5,         // 任务栈
        NULL,         // 任务参数
        0,            // 任务优先级, with 3 (configMAX_PRIORITIES - 1) 是最高的，0是最低的.
        NULL          // 任务句柄
    );
}