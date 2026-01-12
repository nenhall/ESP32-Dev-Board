/*
 * ESP32 WiFi + 蓝牙 + OLED 主程序
 * PlatformIO 项目
 *
 * 功能:
 * - WiFi 连接和状态显示
 * - 蓝牙串口通信
 * - OLED 显示屏信息展示
 *
 * 修改日期: 2026-01-12
 */

#include "config.h"
#include <Arduino.h>
#include <time.h>

// 引入各个模块
#include "bt_manager.h"
#include "chat_api_client.h"
#include "oled_display.h"
#include "wifi_manager.h"

// ==================== 全局变量 ====================
unsigned long lastBlinkTime = 0; // 上次 LED 闪烁时间
bool ledState = false;           // LED 状态

// 当前显示的索引（用于循环显示不同信息）
int displayIndex = 0;

// API 测试控制
bool apiTestTriggered = false; // 是否已触发 API 测试

/**
 * @brief 显示设备综合状态（WiFi + 蓝牙 + 授权 + 内存）
 * 这是 main 层的辅助函数，组合多个显示功能
 */
void showDeviceStatus() {
  u8g2.clearBuffer();

  // 授权状态
  bool hasToken = isTokenSet();
  if (hasToken) {
    u8g2.drawStr(0, 12, "Auth: OK");
  } else {
    u8g2.drawStr(0, 12, "Auth: No");
  }

  // 蓝牙状态
  bool btConnected = isBLEConnected();
  if (btConnected) {
    u8g2.drawStr(0, 24, "BLE: Connected");
  } else {
    u8g2.drawStr(0, 24, "BLE: Ready");
  }

  // WiFi 状态
  u8g2.setFont(u8g2_font_ncenB08_tr);
  bool wifiConnected = (WiFi.status() == WL_CONNECTED);
  if (wifiConnected) {
    u8g2.drawStr(0, 36, "WiFi: Connected");
  } else {
    u8g2.drawStr(0, 36, "WiFi: Off");
  }

  // 内存信息
  char ramStr[32];
  unsigned int freeRAM = ESP.getFreeHeap();
  snprintf(ramStr, sizeof(ramStr), "RAM: %uKB", freeRAM / 1024);
  u8g2.drawStr(0, 48, ramStr);

  // 当前时间
  time_t now = time(nullptr);
  if (now > 1000000) { // 时间已同步
    struct tm *timeinfo = localtime(&now);
    char timeStr[32];
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", timeinfo);
    u8g2.drawStr(0, 60, timeStr);
  } else { // 时间未同步
    u8g2.drawStr(0, 60, "Time: Not synced");
  }

  u8g2.sendBuffer();
}

/**
 * @brief Arduino 初始化函数
 */
void setup() {
  // 初始化串口
  Serial.begin(115200);
  delay(1000);

  // 初始化 LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // 输出启动信息
  Serial.println("\n╔══════════════════════════════════╗");
  Serial.println("║   ESP32 WiFi + BT + OLED         ║");
  Serial.println("║   多功能示例程序                  ║");
  Serial.println("╚══════════════════════════════════╝");
  Serial.printf("CPU 频率: %d MHz\n", ESP.getCpuFreqMHz());
  Serial.printf("Flash 大小: %d MB\n", ESP.getFlashChipSize() / (1024 * 1024));
  Serial.printf("自由内存: %d bytes\n", ESP.getFreeHeap());
  Serial.println("────────────────────────────────────");

  // 初始化各个模块
  initOLED();
  initWiFi();
  initStorage(); // 初始化 NVS 存储
  // 使用 BLE（iOS/Android/macOS/Windows BLE）
  initBluetoothBLE(BLUETOOTH_DEVICE_NAME);

  Serial.println("\n程序启动成功!\n");
  Serial.println("蓝牙命令: help, status, wifi, ip, ram, restart");
}

/**
 * @brief Arduino 主循环函数
 */
void loop() {
  // LED 闪烁 (每秒)
  if (millis() - lastBlinkTime >= 1000) {
    lastBlinkTime = millis();
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
    showDeviceStatus();
  }

  // 处理 BLE 维护（确保广告继续）
  handleBLE();

  // 处理蓝牙命令
  handleBluetoothCommands();

  // 检查 WiFi 连接状态
  handleWiFi();

  // 蓝牙连接成功后自动测试 API（只测试一次）
  if (!apiTestTriggered && isBLEConnected()) {
    // 等待蓝牙连接稳定（延迟 2 秒）
    static unsigned long bleConnectedTime = 0;
    if (bleConnectedTime == 0) {
      bleConnectedTime = millis();
      Serial.println("[ChatAPI] 蓝牙已连接，准备测试 API...");
    } else if (millis() - bleConnectedTime >= 2000) {
      apiTestTriggered = true;
      Serial.println("\n╔══════════════════════════════════╗");
      Serial.println("║       测试聊天 API                ║");
      Serial.println("╚══════════════════════════════════╝");

      // 创建客户端并发送请求
      ChatApiClient client;
      Serial.println("[ChatAPI] 发送测试消息: '重阳节'");
      ChatApiResponse response = client.sendMessage("重阳节");

      // 打印结果
      Serial.printf("\n状态码: %d\n", response.statusCode);
      if (response.success) {
        Serial.println("═════ 响应内容 ═════");
        Serial.println(response.content);
        Serial.println("═══════════════════");
      } else {
        Serial.printf("错误: %s\n", response.error.c_str());
      }
      Serial.println("────────────────────────────────────\n");
    }
  }

  // 稍微延时，避免 CPU 占用过高
  delay(10);
}
