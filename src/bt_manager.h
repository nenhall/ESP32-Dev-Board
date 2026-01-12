/*
 * 蓝牙管理模块 (基于 NimBLE-Arduino)
 *
 * 功能: BLE (NUS) 初始化、命令处理、数据传输
 * 修改日期: 2026-01-12
 */

#ifndef BT_MANAGER_H
#define BT_MANAGER_H

#include <Arduino.h>

// Token 相关函数
/**
 * @brief 初始化 NVS 存储
 */
void initStorage();

/**
 * @brief 保存 token 到 NVS
 * @param token 要保存的 token
 * @return true 保存成功, false 保存失败
 */
bool saveToken(const char* token);

/**
 * @brief 从 NVS 读取 token
 * @return 保存的 token，如果不存在返回空字符串
 */
String loadToken();

/**
 * @brief 检查是否已设置 token
 * @return true 已设置, false 未设置
 */
bool isTokenSet();

/**
 * @brief 清除已保存的 token
 * @return true 清除成功, false 清除失败
 */
bool clearToken();

// BLE 相关函数
/**
 * @brief 初始化 BLE (Nordic UART Service)
 * @param deviceName BLE 广播名称
 */
void initBluetoothBLE(const char *deviceName);

/**
 * @brief 检查是否有数据可读
 * @return true 有数据, false 无数据
 */
bool bleAvailable();

/**
 * @brief 读取一条字符串（会清空内部缓冲）
 * @return 接收到的字符串
 */
String bleReadString();

/**
 * @brief 发送字符串（带换行）
 * @param msg 要发送的消息
 */
void blePrintln(const char *msg);

/**
 * @brief printf 风格发送
 * @param format 格式化字符串
 * @param ... 可变参数
 */
void blePrintf(const char *format, ...);

/**
 * @brief 检查 BLE 是否已连接
 * @return true 已连接, false 未连接
 */
bool isBLEConnected();

/**
 * @brief 处理 BLE 维护任务
 * 在主循环中调用，确保广告继续
 */
void handleBLE();

/**
 * @brief 处理蓝牙接收到的命令
 */
void handleBluetoothCommands();

/**
 * @brief 通过蓝牙打印日志
 * @param message 日志消息
 */
void btLog(const char *message);

#endif // BT_MANAGER_H
