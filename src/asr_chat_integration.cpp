/*
 * ASR 与 Chat API 集成模块实现
 *
 * 创建日期: 2026-01-12
 */

#include "asr_chat_integration.h"

ASRChatIntegration::ASRChatIntegration()
    : currentState(STATE_IDLE), resultAvailable(false), webSearchEnabled(true), stateStartTime(0), totalStartTime(0) {}

void ASRChatIntegration::init(const String &chatHost, int chatPort) {
  Serial.println("\n╔══════════════════════════════════╗");
  Serial.println("║    初始化 ASR-Chat 集成模块      ║");
  Serial.println("╚══════════════════════════════════╝");

  // 初始化 ASR
  asr.init();

  // 初始化 Chat API 客户端
  chatClient.setHost(chatHost);
  chatClient.setPort(chatPort);

  Serial.printf("[Integration] Chat API: %s:%d\n", chatHost.c_str(), chatPort);
  Serial.println("[Integration] 模块初始化完成");
}

void ASRChatIntegration::handle() {
  // 在任何状态下都处理 ASR 数据
  if (asr.handleASR()) {
    // 如果当前不是等待 ASR 的状态，记录识别但不处理
    if (currentState == STATE_WAITING_FOR_ASR) {
      ASRRecognitionResult result = asr.getLastResult();
      if (result.success) {
        handleASRResult();
      } else {
        handleError("ASR recognition failed: " + result.error);
      }
    }
  }

  // 状态机处理
  switch (currentState) {
  case STATE_IDLE:
    // 空闲状态，等待外部触发
    break;

  case STATE_WAITING_FOR_ASR:
    // 已请求 ASR，等待结果
    // 超时检查
    if (millis() - stateStartTime > STATE_TIMEOUT) {
      handleError("ASR recognition timeout");
    }
    break;

  case STATE_SENDING_TO_CHAT:
    // 立即调用 Chat API
    callChatAPI();
    break;

  case STATE_WAITING_FOR_CHAT:
    // Chat API 调用已完成（同步），立即转到 TTS 发送
    sendTTSResponse();
    break;

  case STATE_SENDING_TTS:
    // TTS 发送已完成，返回空闲
    currentState = STATE_IDLE;
    resultAvailable = true;
    break;

  case STATE_ERROR:
    // 错误状态，返回空闲
    currentState = STATE_IDLE;
    resultAvailable = true;
    break;

  default:
    currentState = STATE_IDLE;
    break;
  }
}

void ASRChatIntegration::startListening() {
  Serial.println("[Integration] 启动 ASR 监听...");
  asr.sendCommand(ASR_CMD_START_LISTEN);

  currentState = STATE_WAITING_FOR_ASR;
  stateStartTime = millis();
  totalStartTime = millis();
}

void ASRChatIntegration::stopListening() {
  Serial.println("[Integration] 停止 ASR 监听");
  asr.sendCommand(ASR_CMD_STOP_LISTEN);
  currentState = STATE_IDLE;
}

ASRChatState ASRChatIntegration::getCurrentState() const { return currentState; }

bool ASRChatIntegration::hasCompletedResult() const { return resultAvailable; }

IntegrationResult ASRChatIntegration::getLastResult() const { return lastResult; }

void ASRChatIntegration::clearResultFlag() { resultAvailable = false; }

void ASRChatIntegration::reset() {
  currentState = STATE_IDLE;
  resultAvailable = false;
  recognizedText = "";
  chatResponse = "";
  asr.clearNewResultFlag();
}

void ASRChatIntegration::setChatToken(const String &token) { chatClient.setToken(token); }

void ASRChatIntegration::setChatUser(const String &user) { chatClient.setUser(user); }

void ASRChatIntegration::setWebSearchEnabled(bool enabled) { webSearchEnabled = enabled; }

void ASRChatIntegration::handleASRResult() {
  ASRRecognitionResult result = asr.getLastResult();

  Serial.printf("[Integration] ASR 识别完成: \"%s\" (置信度: %d%%)\n", result.content.c_str(), result.confidence);

  recognizedText = result.content;
  currentState = STATE_SENDING_TO_CHAT;
  stateStartTime = millis();
}

void ASRChatIntegration::callChatAPI() {
  Serial.printf("[Integration] 调用 Chat API: \"%s\"\n", recognizedText.c_str());

  currentState = STATE_WAITING_FOR_CHAT;
  stateStartTime = millis();

  // 调用 Chat API（同步调用）
  ChatApiResponse response = chatClient.sendMessage(recognizedText);

  if (response.success) {
    Serial.printf("[Integration] Chat API 响应成功\n");

    // 截断响应长度（用于 TTS 播报）
    chatResponse = response.content;
    if (chatResponse.length() > CHAT_RESPONSE_MAX_LENGTH) {
      chatResponse = chatResponse.substring(0, CHAT_RESPONSE_MAX_LENGTH);
      Serial.printf("[Integration] 响应被截断至 %d 字符\n", CHAT_RESPONSE_MAX_LENGTH);
    }

    Serial.printf("[Integration] 响应内容: \"%s\"\n", chatResponse.c_str());
    currentState = STATE_SENDING_TTS;
  } else {
    handleError("Chat API error: " + response.error);
  }
}

void ASRChatIntegration::sendTTSResponse() {
  if (chatResponse.length() == 0) {
    handleError("No response content to speak");
    return;
  }

  Serial.printf("[Integration] 发送 TTS 播报: \"%s\"\n", chatResponse.c_str());

  if (asr.sendTTS(chatResponse)) {
    Serial.println("[Integration] TTS 请求发送成功");
    currentState = STATE_SENDING_TTS;

    // 记录结果
    lastResult.success = true;
    lastResult.recognizedText = recognizedText;
    lastResult.chatResponse = chatResponse;
    lastResult.error = "";
    lastResult.totalTime = millis() - totalStartTime;

    Serial.printf("[Integration] 处理完成，耗时: %d ms\n", lastResult.totalTime);
  } else {
    handleError("Failed to send TTS");
  }
}

void ASRChatIntegration::handleError(const String &errorMsg) {
  Serial.printf("[Integration] 错误: %s\n", errorMsg.c_str());

  currentState = STATE_ERROR;
  stateStartTime = millis();

  // 记录错误结果
  lastResult.success = false;
  lastResult.recognizedText = recognizedText;
  lastResult.chatResponse = chatResponse;
  lastResult.error = errorMsg;
  lastResult.totalTime = millis() - totalStartTime;

  // 尝试播报错误提示
  Serial.println("[Integration] 尝试播报错误提示...");
  asr.sendTTS("出错了");
}
