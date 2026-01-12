/*
 * ASR + Chat API 集成使用示例
 *
 * 这是一个完整的示例，展示如何在 ESP32 项目中使用 ASR 和 Chat API 集成模块
 *
 * 功能演示：
 * - 通过 BLE 配置 Token 和用户 ID
 * - 启动 ASR 监听
 * - 自动调用 Chat API 处理用户输入
 * - 通过 ASR 播报结果
 *
 * 创建日期: 2026-01-12
 */

#ifndef ASR_INTEGRATION_EXAMPLE_H
#define ASR_INTEGRATION_EXAMPLE_H

#include "asr_chat_integration.h"
#include "bt_manager.h"
#include "wifi_manager.h"
#include <Arduino.h>

/**
 * @brief 处理 BLE 命令回调（由 BT 管理器调用）
 *
 * 支持的命令：
 * - "listen" 或 "start" : 启动 ASR 监听
 * - "stop" : 停止 ASR 监听
 * - "status" : 显示当前状态
 * - "test <文本>" : 直接测试 Chat API（跳过 ASR）
 */
extern void handleASRBLECommand(const String &command);

/**
 * @brief 处理 ASR 识别结果的回调函数原型
 * 用户可实现此函数以自定义结果处理
 */
typedef void (*ASRResultCallback)(const IntegrationResult &result);

/**
 * @brief 初始化 ASR 集成示例
 * @param chatHost Chat API 主机地址
 * @param chatPort Chat API 端口
 * @param resultCallback 结果回调函数（可选）
 */
void initASRIntegrationExample(const String &chatHost = "192.168.0.103", int chatPort = 10089,
                               ASRResultCallback resultCallback = nullptr);

/**
 * @brief 处理 ASR 集成的主循环（应在 loop() 中调用）
 */
void handleASRIntegrationLoop();

/**
 * @brief 直接发送测试消息（跳过 ASR 识别）
 * 用于调试 Chat API
 */
void testChatAPIDirectly(const String &message);

/**
 * @brief 显示 ASR 和 Chat API 的当前状态
 */
void showASRStatus();

#endif // ASR_INTEGRATION_EXAMPLE_H
