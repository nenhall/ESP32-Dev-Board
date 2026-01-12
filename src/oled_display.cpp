/*
 * OLED 显示屏模块实现
 *
 * 修改日期: 2026-01-12
 */

#include "oled_display.h"
#include "config.h"

// OLED 显示屏对象定义
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE, /* clock=*/OLED_SCL, /* data=*/OLED_SDA);

/**
 * @brief 初始化 OLED 显示屏
 */
void initOLED() {
  Serial.println("Initializing OLED...");

  u8g2.begin();
  u8g2.clearBuffer();

  // 使用英文字体
  u8g2.setFont(u8g2_font_ncenB10_tr);
  u8g2.drawStr(0, 15, "Questech");
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(0, 35, "OLED Ready");
  u8g2.drawStr(0, 55, "Please wait...");
  u8g2.sendBuffer();

  Serial.println("OLED initialized!");
}

/**
 * @brief 显示启动画面
 */
void showStartupScreen(const char *text) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  u8g2.drawStr(0, 15, "Questech");
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(0, 35, text);
  u8g2.drawStr(0, 55, "Initializing...");
  u8g2.sendBuffer();
}

/**
 * @brief 显示 WiFi 连接状态
 */
void showWiFiStatus(bool connected, const char *ip) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  u8g2.drawStr(0, 15, "WiFi Status");

  u8g2.setFont(u8g2_font_ncenB08_tr);
  if (connected) {
    u8g2.drawStr(0, 35, "Connected!");
    if (strlen(ip) > 0) {
      u8g2.drawStr(0, 50, ip);
    }
  } else {
    u8g2.drawStr(0, 35, "Disconnected");
    u8g2.drawStr(0, 50, "Connecting...");
  }

  u8g2.sendBuffer();
}

/**
 * @brief 显示蓝牙状态
 */
void showBluetoothStatus(bool connected) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  u8g2.drawStr(0, 15, "Bluetooth");

  u8g2.setFont(u8g2_font_ncenB08_tr);
  if (connected) {
    u8g2.drawStr(0, 35, "Connected");
  } else {
    u8g2.drawStr(0, 35, "Ready");
  }

  u8g2.sendBuffer();
}

/**
 * @brief 显示内存信息
 */
void showRAMStatus(unsigned int freeRAM) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  u8g2.drawStr(0, 15, "Memory");

  u8g2.setFont(u8g2_font_ncenB08_tr);
  char ramStr[32];
  snprintf(ramStr, sizeof(ramStr), "RAM: %dKB", freeRAM / 1024);
  u8g2.drawStr(0, 35, ramStr);

  u8g2.sendBuffer();
}
