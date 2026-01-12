/*
 * WiFi 管理模块
 *
 * 功能: WiFi 连接、状态监控、自动重连
 * 修改日期: 2026-01-12
 */

#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>

/**
 * @brief 初始化 WiFi
 * @return 连接成功返回 true，失败返回 false
 */
bool initWiFi();

/**
 * @brief 检查 WiFi 连接状态
 * @return 已连接返回 true，否则返回 false
 */
bool isWiFiConnected();

/**
 * @brief 获取 WiFi SSID
 * @return SSID 字符串
 */
String getWiFiSSID();

/**
 * @brief 获取 WiFi IP 地址
 * @return IP 地址字符串
 */
String getWiFiIP();

/**
 * @brief 获取 WiFi 信号强度
 * @return RSSI 值（dBm）
 */
int32_t getWiFiRSSI();

/**
 * @brief 获取 WiFi MAC 地址
 * @return MAC 地址字符串
 */
String getWiFiMAC();

/**
 * @brief 断开 WiFi 连接
 */
void disconnectWiFi();

/**
 * @brief WiFi 循环任务（处理自动重连）
 * @param checkInterval 检查间隔（毫秒），默认 30000ms
 */
void handleWiFi(unsigned long checkInterval = 30000);

/**
 * @brief 通过蓝牙打印 WiFi 信息
 * @param bt 蓝牙串口对象引用
 */
void printWiFiInfo(Print& output);

#endif // WIFI_MANAGER_H
