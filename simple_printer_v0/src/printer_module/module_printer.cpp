#include "module_printer.h"

float addTime[6] = {0};

// 热密度
uint8_t heat_density = 64;

/**
 * @brief 失能所有通道
 *
 */
static void set_stb_idle()
{
    digitalWrite(PIN_STB1, LOW);
    digitalWrite(PIN_STB2, LOW);
    digitalWrite(PIN_STB3, LOW);
    digitalWrite(PIN_STB4, LOW);
    digitalWrite(PIN_STB5, LOW);
    digitalWrite(PIN_STB6, LOW);
}

/**
 * @brief 打印前初始化
 *
 */
void init_printing()
{
    // 开启打印超时监听
    open_printer_timeout_timer();
    set_stb_idle();
    digitalWrite(PIN_LAT, HIGH);
    // POWER ON
    digitalWrite(PIN_VHEN, HIGH);
}

/**
 * @brief 打印后停止
 *
 */
static void stop_printing()
{
    // 关闭打印超时监听
    close_printer_timeout_timer();
    // POWER OFF
    digitalWrite(PIN_VHEN, LOW);
    set_stb_idle();
    digitalWrite(PIN_LAT, HIGH);
}

/**
 * @brief Set the heat density object密度设置
 *
 * @param density
 */
void set_heat_density(uint8_t density)
{
    Serial.printf("打印密度设置：%d\n", density);
    heat_density = density;
}

void clearAddTime()
{
    addTime[0] = addTime[1] = addTime[2] = addTime[3] = addTime[4] = addTime[5] = 0;
}

/**
 * @brief 发送一行数据
 *
 * @param data
 */
void send_one_line_data(uint8_t *data)
{
    float tmpAddTime = 0;
    clearAddTime();
    for (uint8_t i = 0; i < 6; ++i)
    {
        for (uint8_t j = 0; j < 8; ++j)
        {
            addTime[i] += data[i * 8 + j];
        }
        tmpAddTime = addTime[i] * addTime[i];
        addTime[i] = kAddTime * tmpAddTime;
    }
    // Serial.printf("data %d\n", data[0]);
    /* Send one dot line data. */
    spiCommand(data, TPH_DI_LEN);
    /* After send one dot line, send LAT signal low pulse.*/
    digitalWrite(PIN_LAT, LOW);
    us_delay(LAT_TIME);
    digitalWrite(PIN_LAT, HIGH);
}

/**
 * @brief 通道打印运行
 *
 * @param now_stb_num
 */
static void run_stb(uint8_t now_stb_num)
{
    switch (now_stb_num)
    {
    case 0:
        digitalWrite(PIN_STB1, 1);
        us_delay((PRINT_TIME + addTime[0] + STB1_ADDTIME) * ((double)heat_density / 100));
        digitalWrite(PIN_STB1, 0);
        us_delay(PRINT_END_TIME);
        break;
    case 1:
        digitalWrite(PIN_STB2, 1);
        us_delay((PRINT_TIME + addTime[1] + STB2_ADDTIME) * ((double)heat_density / 100));
        digitalWrite(PIN_STB2, 0);
        us_delay(PRINT_END_TIME);
        break;
    case 2:
        digitalWrite(PIN_STB3, 1);
        us_delay((PRINT_TIME + addTime[2] + STB3_ADDTIME) * ((double)heat_density / 100));
        digitalWrite(PIN_STB3, 0);
        us_delay(PRINT_END_TIME);
        break;
    case 3:
        digitalWrite(PIN_STB4, 1);
        us_delay((PRINT_TIME + addTime[3] + STB4_ADDTIME) * ((double)heat_density / 100));
        digitalWrite(PIN_STB4, 0);
        us_delay(PRINT_END_TIME);
        break;
    case 4:
        digitalWrite(PIN_STB5, 1);
        us_delay((PRINT_TIME + addTime[4] + STB5_ADDTIME) * ((double)heat_density / 100));
        digitalWrite(PIN_STB5, 0);
        us_delay(PRINT_END_TIME);
        break;
    case 5:
        digitalWrite(PIN_STB6, 1);
        us_delay((PRINT_TIME + addTime[5] + STB6_ADDTIME) * ((double)heat_density / 100));
        digitalWrite(PIN_STB6, 0);
        us_delay(PRINT_END_TIME);
        break;
    default:
        break;
    }
}

/**
 * @brief  移动电机 & 开始打印
 * @param  need_stop: 是否需要停止打印
 * @param  stbnum: 通道号
 * @return true: 停止打印
 * @return false: 继续打印
 */
bool move_and_start_std(bool need_stop, uint8_t stbnum)
{
    if (need_stop == true)
    {
        Serial.print("打印停止\n");
        motor_stop();
        stop_printing();
        return true;
    }
    // 4step一行
    motor_run();
    if (stbnum == ALL_STB_NUM)
    {
        // 所有通道打印
        for (uint8_t index = 0; index < 6; index++)
        {
            run_stb(index);
            // 把电机运行信号插入通道加热中，减少打印卡顿和耗时
            if (index == 1 || index == 3 || index == 5)
            {
                motor_run();
            }
        }
        // motor_run_step(3);

    }
    else
    {
        // 单通道打印
        run_stb(stbnum);
        motor_run_step(3);
    }
    return false;
}

/**
 * @brief  初始化 printer_module
 */
void init_printer()
{
    init_motor();

    pinMode(PIN_LAT, OUTPUT);
    pinMode(PIN_SCK, OUTPUT);
    pinMode(PIN_SDA, OUTPUT);

    pinMode(PIN_STB1, OUTPUT);
    pinMode(PIN_STB2, OUTPUT);
    pinMode(PIN_STB3, OUTPUT);
    pinMode(PIN_STB4, OUTPUT);
    pinMode(PIN_STB5, OUTPUT);
    pinMode(PIN_STB6, OUTPUT);
    set_stb_idle();
    digitalWrite(PIN_LAT, HIGH);


    pinMode(PIN_VHEN, OUTPUT);
    digitalWrite(PIN_VHEN, LOW);

    init_spi();
}
