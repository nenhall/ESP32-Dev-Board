/*
 * ESP32 配置文件示例
 *
 * 使用方法:
 * 1. 复制此文件到你的项目目录
 * 2. 重命名为 config.h
 * 3. 修改配置参数
 * 4. 在主程序中包含: #include "config.h"
 */

#ifndef CONFIG_EXAMPLE_H
#define CONFIG_EXAMPLE_H

// ==================== WiFi 配置 ====================
#define WIFI_SSID "xxxxx"
#define WIFI_PASSWORD "xxxxxxxx"
#define WIFI_TIMEOUT 20000 // 连接超时时间 (毫秒)

// WiFi 连接模式
// 使用更独特的宏名以避免与系统/SDK 宏冲突
#define WIFI_USE_STA true // true=站点模式, false=AP模式

// ==================== GPIO 引脚定义 ====================
// LED
#define LED_PIN 2
#define LED_BUILTIN 2

// 按钮 (如果有)
#define BUTTON_PIN 0

// 继电器 (如果有)
#define RELAY_PIN 4

// 传感器 (如果有)
#define SENSOR_PIN 34

// I2C 引脚
#define I2C_SDA 21
#define I2C_SCL 22

// SPI 引脚
#define SPI_MOSI 23
#define SPI_MISO 19
#define SPI_SCK 18
#define SPI_CS 5

// ==================== 串口配置 ====================
#define SERIAL_BAUD 115200
#define SERIAL_TIMEOUT 1000

// ==================== 传感器配置 ====================
// 温湿度传感器读取间隔 (毫秒)
#define SENSOR_READ_INTERVAL 5000

// ADC 配置
#define ADC_RESOLUTION 12  // 12位 = 0-4095
#define ADC_ATTENUATION 11 // 11dB 衰减

// ==================== 调试配置 ====================
#define DEBUG_MODE true
#define DEBUG_SERIAL Serial

#if DEBUG_MODE
#define DEBUG_PRINT(x) DEBUG_SERIAL.print(x)
#define DEBUG_PRINTLN(x) DEBUG_SERIAL.println(x)
#define DEBUG_PRINTF(...) DEBUG_SERIAL.printf(__VA_ARGS__)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#define DEBUG_PRINTF(...)
#endif

// ==================== 低功耗配置 ====================
// Deep Sleep 时间 (微秒)
#define DEEP_SLEEP_INTERVAL 60000000 // 60秒

// Light Sleep 时间 (微秒)
#define LIGHT_SLEEP_INTERVAL 10000000 // 10秒

// ==================== MQTT 配置 (可选) ====================
#define MQTT_ENABLED false
#define MQTT_SERVER "192.168.1.200"
#define MQTT_PORT 1883
#define MQTT_USER "username"
#define MQTT_PASSWORD "password"
#define MQTT_CLIENT_ID "ESP32_Client"

// ==================== NTP 时间同步配置 ====================
#define NTP_ENABLED true
#define NTP_SERVER "pool.ntp.org"
#define TIMEZONE "CST-8" // 中国时区 UTC+8

// ==================== OTA 升级配置 ====================
#define OTA_ENABLED false
#define OTA_PORT 3232
#define OTA_PASSWORD "admin"

// ==================== 看门狗配置 ====================
#define WDT_TIMEOUT 5000 // 看门狗超时时间 (毫秒)

// ==================== 网络配置 ====================
// 静态 IP 配置 (可选)
#define USE_STATIC_IP false
#define STATIC_IP "192.168.1.100"
#define GATEWAY "192.168.1.1"
#define SUBNET "255.255.255.0"
#define DNS "8.8.8.8"

// Web 服务器配置
#define WEB_SERVER_PORT 80
#define WEB_SERVER_TIMEOUT 2000

#endif // CONFIG_EXAMPLE_H
