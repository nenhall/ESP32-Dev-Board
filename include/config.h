/*
 * ESP32 配置文件
 *
 * 使用方法:
 * 在主程序中包含: #include "config.h"
 */

#ifndef CONFIG_H
#define CONFIG_H

// ==================== WiFi 配置 ====================
#define WIFI_SSID "boss"
#define WIFI_PASSWORD "@nenhall666"
#define WIFI_TIMEOUT 20000 // 连接超时时间 (毫秒)

// WiFi 连接模式
// 使用更独特的宏名以避免与系统/SDK 宏冲突
#define WIFI_USE_STA true // true=站点模式, false=AP模式

// ==================== GPIO 引脚定义 ====================
// LED
#define LED_PIN 2
#define LED_BUILTIN 2

// OLED I2C 引脚
#define OLED_SDA 21
#define OLED_SCL 22

// ==================== 蓝牙配置 ====================
#define BLUETOOTH_DEVICE_NAME "Questech_BT"

#endif // CONFIG_H
