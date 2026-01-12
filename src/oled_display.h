/*
 * OLED 显示屏模块
 *
 * 功能: OLED 屏幕的初始化、内容显示
 * 修改日期: 2026-01-12
 */

#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H

#include <Arduino.h>
#include <U8g2lib.h>

// OLED 显示屏对象声明
extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;

/**
 * @brief 初始化 OLED 显示屏
 */
void initOLED();

/**
 * @brief 显示启动画面
 * @param text 显示的文本
 */
void showStartupScreen(const char* text);

/**
 * @brief 显示 WiFi 连接状态
 * @param connected 是否已连接
 * @param ip IP 地址（可选）
 */
void showWiFiStatus(bool connected, const char* ip = "");

/**
 * @brief 显示蓝牙状态
 * @param connected 是否已连接
 */
void showBluetoothStatus(bool connected);

/**
 * @brief 显示内存信息
 * @param freeRAM 空闲内存（字节）
 */
void showRAMStatus(unsigned int freeRAM);

#endif // OLED_DISPLAY_H
