/*
 * WiFi 管理模块实现
 *
 * 修改日期: 2026-01-12
 */

#include "wifi_manager.h"
#include "config.h"
#include "oled_display.h"
#include <time.h>

// 上次 WiFi 检查时间
static unsigned long lastWifiCheck = 0;

/**
 * @brief 初始化 WiFi
 */
bool initWiFi() {
  Serial.println("初始化 WiFi...");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("正在连接 WiFi");
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;

    // 更新 OLED 显示连接进度
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(0, 15, "WiFi Connecting...");
    u8g2.setCursor(0, 35);
    u8g2.print("Attempt ");
    u8g2.print(attempts + 1);
    u8g2.print("/20");
    u8g2.sendBuffer();
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi 连接成功!");
    Serial.printf("IP 地址: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("信号强度: %d dBm\n", WiFi.RSSI());
    Serial.printf("MAC 地址: %s\n", WiFi.macAddress().c_str());

    // 配置 NTP 时间同步
    configTime(8 * 3600, 0, "ntp.aliyun.com", "ntp.tencent.com", "cn.ntp.org.cn");
    Serial.println("正在同步时间...");

    // 等待时间同步（最多 10 秒）
    struct tm timeinfo;
    int syncAttempts = 0;
    while (!getLocalTime(&timeinfo) && syncAttempts < 20) {
      delay(500);
      Serial.print(".");
      syncAttempts++;
    }

    if (syncAttempts < 20) {
      Serial.println("\n时间同步成功!");
      char timeStr[64];
      strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);
      Serial.printf("当前时间: %s\n", timeStr);
    } else {
      Serial.println("\n时间同步失败!");
    }

    return true;
  } else {
    Serial.println("\nWiFi 连接失败!");
    return false;
  }
}

/**
 * @brief 检查 WiFi 连接状态
 */
bool isWiFiConnected() { return WiFi.status() == WL_CONNECTED; }

/**
 * @brief 获取 WiFi SSID
 */
String getWiFiSSID() { return WiFi.SSID(); }

/**
 * @brief 获取 WiFi IP 地址
 */
String getWiFiIP() { return WiFi.localIP().toString(); }

/**
 * @brief 获取 WiFi 信号强度
 */
int32_t getWiFiRSSI() { return WiFi.RSSI(); }

/**
 * @brief 获取 WiFi MAC 地址
 */
String getWiFiMAC() { return WiFi.macAddress(); }

/**
 * @brief 断开 WiFi 连接
 */
void disconnectWiFi() {
  WiFi.disconnect();
  Serial.println("WiFi 已断开");
}

/**
 * @brief WiFi 循环任务（处理自动重连）
 */
void handleWiFi(unsigned long checkInterval) {
  // 检查是否到了检查时间
  if (millis() - lastWifiCheck < checkInterval) {
    return;
  }
  lastWifiCheck = millis();

  // 检查连接状态
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi 断开，尝试重新连接...");
    initWiFi();
  }
}

/**
 * @brief 通过蓝牙打印 WiFi 信息
 */
void printWiFiInfo(Print &output) {
  output.println("=== WiFi 信息 ===");
  output.printf("SSID: %s\n", WiFi.SSID().c_str());
  output.printf("信号强度: %d dBm\n", WiFi.RSSI());
  output.printf("IP 地址: %s\n", WiFi.localIP().toString().c_str());
  output.printf("MAC 地址: %s\n", WiFi.macAddress().c_str());
  output.println("================");
}
