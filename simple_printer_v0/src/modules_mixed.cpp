#include "modules_mixed.h"

// ************************************************** ble module ************************************************** //
bool bleConnected = false;
uint32_t packcount = 0;
BLECharacteristic *pCharacteristic = NULL;

static void clean_blepack_count(){
    packcount = 0;
}

static uint32_t get_blepack_count(){
    return packcount;
}

bool get_ble_connect(){
    return bleConnected;
}

/**
 * @brief  把打印机最近的一次状态传送到手机
 */
void ble_report(){
    if (get_ble_connect()){
        device_state_t *pdevice = get_device_state();
        uint8_t status[4];
        status[0] = pdevice->battery;
        status[1] = pdevice->temperature;
        status[2] = pdevice->paper_state;
        status[3] = pdevice->printer_state;
        pCharacteristic->setValue((uint8_t*)&status,sizeof(status));
        pCharacteristic->notify();
    }
}

// Server回调函数声明
class bleServerCallbacks : public BLEServerCallbacks
{
    void onConnect(BLEServer *pServer)
    {
        bleConnected = true;
        packcount = 0;
        Serial.println("现在有设备接入~");
        run_beep(BEEP_CONNECT);
        run_led(LED_CONNECT);
    }

    void onDisconnect(BLEServer *pServer)
    {
        bleConnected = false;
        Serial.println("现在有设备断开连接~");
        // 在有设备接入后Advertising广播会被停止，所以要在设备断开连接时重新开启广播
        // 不然的话只有重启ESP32后才能重新搜索到
        pServer->startAdvertising(); // 该行效果同 BLEDevice::startAdvertising();
    }
};

class bleCharacteristicCallbacks : public BLECharacteristicCallbacks
{
    void onRead(BLECharacteristic *pCharacteristic)
    { // 客户端读取事件回调函数
        Serial.println("触发读取事件");
    }

    void onWrite(BLECharacteristic *pCharacteristic)
    { // 客户端写入事件回调函数
        size_t length = pCharacteristic->getLength();
        uint8_t *pdata = pCharacteristic->getData();
        if(length == 5){
            if(pdata[0] == 0xA5 && pdata[1] == 0xA5 && pdata[2] == 0xA5 && pdata[3] == 0xA5){
                if(pdata[4] == 1){
                    set_heat_density(30);
                }else if(pdata[4] == 2){
                    set_heat_density(60); 
                }else{
                    set_heat_density(100);
                }
                return;
            }
            if(pdata[0] == 0xA6 && pdata[1] == 0xA6 && pdata[2] == 0xA6 && pdata[3] == 0xA6){
                set_read_ble_finish(true);
                Serial.printf("接收打印数据完成,总行数=%d\n",packcount);
            }
        }
        packcount++;
        write_to_printbuffer(pdata,length);
        Serial.printf("触发写入事件 length=%d count=%d ", length, packcount);
        for (int index = 0; index < length; index++)
        {
            Serial.printf(" %x", pdata[index]);
        }
        Serial.printf("\n");
    }
};

/**
 * @brief 初始化BLE
 */
void init_ble()
{
    BLEDevice::init(BLE_NAME); // 填写自身对外显示的蓝牙设备名称，并初始化蓝牙功能
    BLEDevice::startAdvertising();   // 开启Advertising广播

    BLEServer *pServer = BLEDevice::createServer();  // 创建服务器
    pServer->setCallbacks(new bleServerCallbacks()); // 绑定回调函数

    BLEService *pService = pServer->createService(SERVICE_UUID); // 创建服务
    pCharacteristic = pService->createCharacteristic(            // 创建特征
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_NOTIFY |
            BLECharacteristic::PROPERTY_WRITE |
            BLECharacteristic::PROPERTY_WRITE_NR);
    
    // 如果客户端连上设备后没有任何写入的情况下第一次读取到的数据应该是这里设置的值
    pCharacteristic->setCallbacks(new bleCharacteristicCallbacks());
    pCharacteristic->addDescriptor(new BLE2902()); // 添加描述 
    pService->start(); // 启动服务
    BLEDevice::startAdvertising();
}

// ************************************************** printer module ************************************************** //
/**
 * @brief 打印错误检查
 * @param need_report 是否需BLE上报
 * @return true 打印出错
 * @return false 打印正常
 */
static bool printing_error_check(bool need_report)
{
    if (get_printer_timeout_status())
    {
        Serial.print("打印超时\n");
        return true;
    }
    if (get_device_state()->paper_state == PAPER_STATUS_LACK)
    {
        if(need_report){
            // 停止打印
            clean_printbuffer();
            ble_report();
        }
        // 停止打印
        Serial.print("缺纸\n");
        run_beep(BEEP_WARN);
        run_led(LED_WARN);
        return true;
    }
    if (get_device_state()->temperature > 65)
    {
        if(need_report){
            // 停止打印
            clean_printbuffer();
            ble_report();
        }
        // 停止打印
        Serial.print("温度异常\n");
        run_beep(BEEP_WARN);
        run_led(LED_WARN);
        return true;
    }
    return false;
}

/**
 * @brief 所有通道都打印(真实运行的情况)
 *
 * @param data
 * @param length 数据长度必须是整行 48*n
 */
void start_printing(uint8_t *data, uint32_t len)
{
    uint32_t offset = 0;
    uint8_t *ptr = data;
    bool need_stop = false;
    init_printing();
    while (1)
    {
        if (len > offset)
        {
            // 发送一行数据 48byte*8=384bit
            send_one_line_data(ptr);
            offset += TPH_DI_LEN;
            ptr += TPH_DI_LEN;
        }
        else
            need_stop = true;
        if (move_and_start_std(need_stop, ALL_STB_NUM))
            break;
        if(printing_error_check(false))
            break;
    }
    motor_run_step(40);
    motor_stop();
    Serial.printf("打印完成\n");
}

/**
 * @brief 可变队列打印
 *
 */
void start_printing_by_queuebuf()
{
    uint8_t *pdata = NULL;
    bool need_stop = false;
    uint32_t printer_count = 0;
    init_printing();
    while (1)
    {
        if (get_ble_rx_leftline() > 0)
        {
            // Serial.printf("printing...\n");
            pdata = read_to_printer();
            if (pdata != NULL)
            {
                printer_count ++;
                send_one_line_data(pdata);
                if (move_and_start_std(false, ALL_STB_NUM))
                    break;
            }
        }
        else
        {
            if (move_and_start_std(true, ALL_STB_NUM))
                break;
        }
        if (get_printer_timeout_status())
            break;
        if(printing_error_check(true))
            break;
    }
    motor_run_step(230);
    motor_stop();
    Serial.printf("打印完成 接收总行数:%d 打印总行数:%d\n",get_blepack_count(),printer_count);
    clean_blepack_count();
}

/**
 * @brief  单通道打印(主要用于测试是否可以正常工作)
 * @param  stbnum: 通道号
 * @param  data: 打印的数据
 * @param  len: 打印的数据的长度
 */
static void start_printing_by_onestb(uint8_t stbnum, uint8_t *data, uint32_t len)
{
    uint32_t offset = 0;
    uint8_t *ptr = data;
    bool need_stop = false;
    init_printing();
    while (1)
    {
        Serial.printf("printer %d\n", offset);
        if (len > offset)
        {
            // 发送一行数据 48byte*8=384bit
            send_one_line_data(ptr);
            offset += TPH_DI_LEN;
            ptr += TPH_DI_LEN;
        }
        else
            need_stop = true;
            
        if (move_and_start_std(need_stop, stbnum))
            break;
        if (get_printer_timeout_status())
            break;
        if(printing_error_check(false))
            break;
    }
    motor_run_step(40);
    motor_stop();
}

/**
 * @brief  设置用来测试的数据
 * @param  print_data: 
 */
static void setDebugData(uint8_t *print_data)
{
    for (uint32_t cleardata = 0; cleardata < 48 * 5; ++cleardata)
    {
        print_data[cleardata] = 0x55;
    }
}

/**
 * @brief  测试stb是否能正常工作，使用单通道打印函数 start_printing_by_onestb
 */
void testSTB()
{
    uint8_t print_data[48 * 10];
    uint32_t print_len;
    Serial.println("开始打印打印头选通引脚测试\n顺序: 1  2  3  4  5  6");
    print_len = 48 * 5;
    setDebugData(print_data);
    start_printing_by_onestb(0, print_data, print_len);
    setDebugData(print_data);
    start_printing_by_onestb(1, print_data, print_len);
    setDebugData(print_data);
    start_printing_by_onestb(2, print_data, print_len);
    setDebugData(print_data);
    start_printing_by_onestb(3, print_data, print_len);
    setDebugData(print_data);
    start_printing_by_onestb(4, print_data, print_len);
    setDebugData(print_data);
    start_printing_by_onestb(5, print_data, print_len);
    Serial.println("测试完成");
    motor_run_step(230);
}

// ************************************************** key module ************************************************** //
Button_t Button;
bool printer_test = false;

static void Btn_Dowm_CallBack(void *btn)
{
    Serial.print("Button 单击!");
    printer_test = true;
    // read_all_hal();
}

static void Btn_Double_CallBack(void *btn)
{
    Serial.print("Button 双击!");
}

static void Btn_Long_CallBack(void *btn)
{
    Serial.print("Button 长按!");
    device_state_t *pdevice = get_device_state();
    bool need_beep = false;
    // 不缺纸且不在打印中才执行
    if (pdevice->paper_state == PAPER_STATUS_NORMAL)
    {
        if (pdevice->printer_state == PRINTER_STATUS_FINISH ||
            pdevice->printer_state == PRINTER_STATUS_INIT)
        {
            Serial.print("开始走纸\n");
            motor_start();
        }
        else
        {
            need_beep = true;
        }
    }
    else
        need_beep = true;
    if (need_beep)
    {
        run_beep(BEEP_WARN);
        run_led(LED_WARN);
    }
}

static void Btn_Logn_Free_CallBack(void *btn)
{
    Serial.print("停止走纸\n");
    motor_stop();
}

static uint8_t Read_Button_Level(void)
{
    return digitalRead(PIN_KEY);
}

void init_key()
{
    Button_Create("Button",                                           // 按键名字
                  &Button,                                            // 按键句柄
                  Read_Button_Level,                                  // 按键电平检测函数接口
                  LOW);                                               // 触发电平
    Button_Attach(&Button, BUTTON_DOWM, Btn_Dowm_CallBack);           // 按键单击
    Button_Attach(&Button, BUTTON_DOUBLE, Btn_Double_CallBack);       // 双击
    Button_Attach(&Button, BUTTON_LONG, Btn_Long_CallBack);           // 长按
    Button_Attach(&Button, BUTTON_LONG_FREE, Btn_Logn_Free_CallBack); // 长按释放
}
