/*
 * ASR 与 Chat API 集成模块
 *
 * 功能: 协调 ASR 识别、Chat API 调用和 TTS 播报的完整流程
 * 流程: ASR识别 -> Chat API 请求 -> TTS 播报
 * 创建日期: 2026-01-12
 */

#ifndef ASR_CHAT_INTEGRATION_H
#define ASR_CHAT_INTEGRATION_H

#include "asr_manager.h"
#include "chat_api_client.h"
#include <Arduino.h>

// ==================== 集成模块配置 ====================
#define CHAT_RESPONSE_MAX_LENGTH 30 // Chat 响应截断长度（用于 TTS）
#define CHAT_TIMEOUT 30000          // Chat API 请求超时 (毫秒)

// 处理状态枚举
enum ASRChatState {
  STATE_IDLE = 0,             // 空闲，等待 ASR 识别
  STATE_WAITING_FOR_ASR = 1,  // 等待 ASR 识别结果
  STATE_SENDING_TO_CHAT = 2,  // 发送到 Chat API
  STATE_WAITING_FOR_CHAT = 3, // 等待 Chat API 响应
  STATE_SENDING_TTS = 4,      // 发送 TTS 播报
  STATE_ERROR = 5             // 错误状态
};

/**
 * @brief ASR-Chat 集成处理结果
 */
struct IntegrationResult {
  bool success;
  String recognizedText; // 识别的用户输入
  String chatResponse;   // Chat API 的响应
  String error;          // 错误信息
  uint32_t totalTime;    // 总耗时（毫秒）

  IntegrationResult() : success(false), totalTime(0) {}
};

/**
 * @brief ASR-Chat 集成管理器
 * 负责协调整个流程：ASR识别 -> Chat API -> TTS播报
 */
class ASRChatIntegration {
public:
  ASRChatIntegration();

  /**
   * @brief 初始化集成模块
   * @param chatHost Chat API 主机地址
   * @param chatPort Chat API 端口
   */
  void init(const String &chatHost = "192.168.0.103", int chatPort = 10089);

  /**
   * @brief 处理主循环（应在 Arduino loop() 中调用）
   * 自动管理状态转换和流程推进
   */
  void handle();

  /**
   * @brief 启动监听 ASR 识别
   * 向 ASR 发送开始监听命令
   */
  void startListening();

  /**
   * @brief 停止监听
   * 向 ASR 发送停止监听命令
   */
  void stopListening();

  /**
   * @brief 获取当前处理状态
   */
  ASRChatState getCurrentState() const;

  /**
   * @brief 检查是否有完成的处理结果
   */
  bool hasCompletedResult() const;

  /**
   * @brief 获取最后一次完成的处理结果
   */
  IntegrationResult getLastResult() const;

  /**
   * @brief 清除已读结果标志
   */
  void clearResultFlag();

  /**
   * @brief 重置模块状态
   */
  void reset();

  /**
   * @brief 设置 Chat API Token
   */
  void setChatToken(const String &token);

  /**
   * @brief 设置 Chat API User ID
   */
  void setChatUser(const String &user);

  /**
   * @brief 是否启用网络搜索
   */
  void setWebSearchEnabled(bool enabled);

private:
  /**
   * @brief 处理 ASR 识别结果
   */
  void handleASRResult();

  /**
   * @brief 调用 Chat API
   */
  void callChatAPI();

  /**
   * @brief 发送 TTS 播报
   */
  void sendTTSResponse();

  /**
   * @brief 处理错误情况
   */
  void handleError(const String &errorMsg);

  // 成员变量
  ASRManager asr;
  ChatApiClient chatClient;

  ASRChatState currentState;
  IntegrationResult lastResult;
  bool resultAvailable;

  String recognizedText;        // 临时存储识别结果
  String chatResponse;          // 临时存储 Chat 响应
  unsigned long stateStartTime; // 当前状态开始时间
  unsigned long totalStartTime; // 总处理开始时间

  bool webSearchEnabled;

  // 状态转换超时控制
  static const unsigned long STATE_TIMEOUT = 60000; // 60秒超时
};

#endif // ASR_CHAT_INTEGRATION_H
