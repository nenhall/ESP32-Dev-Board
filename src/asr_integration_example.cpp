/*
 * ASR 集成使用示例实现
 * 创建日期: 2026-01-12
 */

#include "asr_integration_example.h"

// 全局集成模块实例
static ASRChatIntegration g_integration;

// 用户结果回调
static ASRResultCallback g_resultCallback = nullptr;

void initASRIntegrationExample(const String &chatHost, int chatPort, ASRResultCallback resultCallback) {

  Serial.println("\n╔══════════════════════════════════╗");
  Serial.println("║  初始化 ASR 集成示例模块         ║");
  Serial.println("╚══════════════════════════════════╝");

  // 初始化集成模块
  g_integration.init(chatHost, chatPort);

  // 保存结果回调
  g_resultCallback = resultCallback;

  // 从 NVS 读取保存的认证信息
  // 注：使用默认的 Token 和 User ID，可通过 BLE 更新
  Serial.println("[ASR Example] 使用默认的 Token 和 User ID");
  Serial.println("[ASR Example] 提示：可通过 BLE 命令更新认证信息");

  // 启用网络搜索
  g_integration.setWebSearchEnabled(true);

  Serial.println("[ASR Example] 初始化完成");
  Serial.println("[ASR Example] 可用命令: listen, stop, status, test <text>");
}

void handleASRIntegrationLoop() {
  // 处理集成模块的状态机
  g_integration.handle();

  // 检查是否有完成的处理结果
  if (g_integration.hasCompletedResult()) {
    IntegrationResult result = g_integration.getLastResult();

    // 显示结果
    Serial.println("\n╔══════════════════════════════════╗");
    Serial.println("║       ASR-Chat 处理完成          ║");
    Serial.println("╚══════════════════════════════════╝");

    if (result.success) {
      Serial.println("✓ 处理成功");
      Serial.printf("  用户输入: \"%s\"\n", result.recognizedText.c_str());
      Serial.printf("  AI 回复:  \"%s\"\n", result.chatResponse.c_str());
      Serial.printf("  耗时:    %d ms\n", result.totalTime);
    } else {
      Serial.println("✗ 处理失败");
      Serial.printf("  错误: %s\n", result.error.c_str());
    }

    Serial.println("────────────────────────────────────");

    // 调用用户回调（如果已设置）
    if (g_resultCallback != nullptr) {
      g_resultCallback(result);
    }

    // 清除结果标志，准备下一次处理
    g_integration.clearResultFlag();
  }
}

void handleASRBLECommand(const String &command) {
  String cmd = command;
  cmd.toLowerCase();

  if (cmd == "listen" || cmd == "start") {
    // 启动 ASR 监听
    Serial.println("[ASR Example] BLE 命令: 启动监听");
    g_integration.startListening();
    Serial.println("[ASR Example] -> 发送通知: ASR listening started");

  } else if (cmd == "stop") {
    // 停止 ASR 监听
    Serial.println("[ASR Example] BLE 命令: 停止监听");
    g_integration.stopListening();
    Serial.println("[ASR Example] -> 发送通知: ASR listening stopped");

  } else if (cmd == "status") {
    // 显示状态
    showASRStatus();

  } else if (cmd.startsWith("test ")) {
    // 测试 Chat API（跳过 ASR）
    String testText = cmd.substring(5);
    Serial.printf("[ASR Example] BLE 命令: 测试 Chat API，内容: %s\n", testText.c_str());
    testChatAPIDirectly(testText);

  } else {
    Serial.printf("[ASR Example] 未知 BLE 命令: %s\n", command.c_str());
    Serial.println("[ASR Example] -> 发送通知: Unknown command");
  }
}

void testChatAPIDirectly(const String &message) {
  Serial.println("\n╔══════════════════════════════════╗");
  Serial.println("║       直接测试 Chat API           ║");
  Serial.println("╚══════════════════════════════════╝");

  ChatApiClient chatClient;

  // 从 NVS 读取认证信息（使用默认值）
  // 注：可通过 BLE 命令更新 Token

  Serial.printf("发送消息: \"%s\"\n", message.c_str());
  unsigned long startTime = millis();

  // 调用 Chat API
  ChatApiResponse response = chatClient.sendMessage(message);
  unsigned long elapsed = millis() - startTime;

  Serial.printf("耗时: %d ms\n", elapsed);

  if (response.success) {
    Serial.println("响应:");
    Serial.println(response.content);

    String notification = "Reply: " + response.content.substring(0, 20);
    Serial.printf("[ASR Example] -> 发送通知: %s\n", notification.c_str());
  } else {
    Serial.printf("错误: %s\n", response.error.c_str());
    Serial.printf("[ASR Example] -> 发送通知: API Error: %s\n", response.error.c_str());
  }

  Serial.println("────────────────────────────────────\n");
}

void showASRStatus() {
  Serial.println("\n╔══════════════════════════════════╗");
  Serial.println("║       ASR-Chat 状态信息          ║");
  Serial.println("╚══════════════════════════════════╝");

  // 显示当前状态
  ASRChatState state = g_integration.getCurrentState();
  const char *stateStr = "Unknown";

  switch (state) {
  case STATE_IDLE:
    stateStr = "IDLE (空闲)";
    break;
  case STATE_WAITING_FOR_ASR:
    stateStr = "WAITING_FOR_ASR (等待识别)";
    break;
  case STATE_SENDING_TO_CHAT:
    stateStr = "SENDING_TO_CHAT (发送到 API)";
    break;
  case STATE_WAITING_FOR_CHAT:
    stateStr = "WAITING_FOR_CHAT (等待响应)";
    break;
  case STATE_SENDING_TTS:
    stateStr = "SENDING_TTS (发送播报)";
    break;
  case STATE_ERROR:
    stateStr = "ERROR (错误)";
    break;
  }

  Serial.printf("状态: %s\n", stateStr);

  // WiFi 状态
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("WiFi: 已连接 (%s, %d dBm)\n", WiFi.SSID().c_str(), WiFi.RSSI());
  } else {
    Serial.println("WiFi: 未连接");
  }

  // 内存信息
  Serial.printf("自由内存: %d KB\n", ESP.getFreeHeap() / 1024);

  // 认证信息
  String token = loadToken();
  if (token.length() > 0) {
    Serial.print("Token: ");
    Serial.print(token.substring(0, 20));
    Serial.println("...");
  } else {
    Serial.println("Token: 未设置");
  }

  Serial.println("────────────────────────────────────\n");

  // 通过 BLE 发送状态
  Serial.printf("[ASR Example] -> 发送通知: State: %s\n", stateStr);
}
