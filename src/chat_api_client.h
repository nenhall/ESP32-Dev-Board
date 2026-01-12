/*
 * 聊天 API 客户端
 *
 * 功能: 向聊天 API 发送 HTTP 请求并获取响应
 * 创建日期: 2026-01-12
 */

#ifndef CHAT_API_CLIENT_H
#define CHAT_API_CLIENT_H

#include <Arduino.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// API 配置
#define CHAT_API_HOST "192.168.0.103"
#define CHAT_API_PORT 10089
#define CHAT_API_PATH "/chat/completions/"
#define CHAT_API_TIMEOUT 30000  // 30秒超时

// 默认配置
#define CHAT_API_DEFAULT_TOKEN "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJ1c2VyX2lkZW50aWZpZXIiOiIxMzUzZTE2ODY1NjI1MTY4YzJjMWNiMjI0MjQ5YjdhYiIsImV4cCI6MTc2ODMwMTI1MCwiaWF0IjoxNzY4MjE0ODUwLCJpc3MiOiJmYXN0YXBpLWNoYXQtc2VydmljZSIsInR5cGUiOiJhY2Nlc3MifQ.9voLX6mJpkQNatnXNp831kDSkp8mfTmVM8q7CfVKc9I"
#define CHAT_API_DEFAULT_USER "1353e16865625168c2c1cb224249b7ab"

/**
 * @brief 聊天 API 响应结构体
 */
struct ChatApiResponse {
  bool success;
  int statusCode;
  String content;
  String error;
  String rawResponse;

  ChatApiResponse() : success(false), statusCode(0) {}
};

/**
 * @brief 聊天 API 客户端类
 */
class ChatApiClient {
public:
  ChatApiClient();

  // 设置 API 配置
  void setHost(const String& host);
  void setPort(int port);
  void setToken(const String& token);
  void setUser(const String& user);

  // 发送聊天请求（简化版）
  ChatApiResponse sendMessage(const String& message);

  // 发送聊天请求（完整参数）
  ChatApiResponse sendMessage(
    const String& message,
    const String& conversationId,
    bool enableWebSearch = true,
    bool stream = false
  );

  // 检查 WiFi 连接
  bool isConnected();

private:
  String host;
  int port;
  String token;
  String user;

  // 构建请求体
  String buildRequestBody(
    const String& message,
    const String& conversationId,
    bool enableWebSearch,
    bool stream
  );

  // 生成会话 ID
  String generateConversationId();
};

#endif // CHAT_API_CLIENT_H
