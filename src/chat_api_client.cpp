/*
 * 聊天 API 客户端实现
 *
 * 功能: 向聊天 API 发送 HTTP 请求并获取响应
 * 创建日期: 2026-01-12
 */

#include "chat_api_client.h"
#include "bt_manager.h"
#include <WiFi.h>
#include <time.h>

ChatApiClient::ChatApiClient()
  : host(CHAT_API_HOST),
    port(CHAT_API_PORT),
    token(CHAT_API_DEFAULT_TOKEN),
    user(CHAT_API_DEFAULT_USER) {
}

void ChatApiClient::setHost(const String& host) {
  this->host = host;
}

void ChatApiClient::setPort(int port) {
  this->port = port;
}

void ChatApiClient::setToken(const String& token) {
  this->token = token;
}

void ChatApiClient::setUser(const String& user) {
  this->user = user;
}

bool ChatApiClient::isConnected() {
  return (WiFi.status() == WL_CONNECTED);
}

String ChatApiClient::generateConversationId() {
  // 生成会话 ID: bzsz_<timestamp:s>_<random:6位数字>
  // 使用真正的 Unix 时间戳（从 1970-01-01 开始的秒数）
  time_t now = time(nullptr);

  // 如果时间还没同步，使用 millis() 加上一个基准时间（2024-01-01）
  unsigned long timestampSec;
  if (now < 1000000) {
    // 2024-01-01 00:00:00 UTC 的 Unix 时间戳是 1704067200
    const unsigned long BASE_TIMESTAMP = 1704067200;
    timestampSec = BASE_TIMESTAMP + (millis() / 1000);
  } else {
    timestampSec = now;
  }

  // 生成 6 位随机数字 (000000-999999)
  unsigned int randomNum = random(1000000);

  char convId[64];
  snprintf(convId, sizeof(convId), "bzsz_%lu_%06u", timestampSec, randomNum);
  return String(convId);
}

String ChatApiClient::buildRequestBody(
  const String& message,
  const String& conversationId,
  bool enableWebSearch,
  bool stream
) {
  // 使用 ArduinoJson 7.x 的方式构建 JSON
  JsonDocument doc;

  // 基本参数
  doc["model"] = "glm";
  doc["store"] = true;
  doc["stream"] = stream;
  doc["user"] = user;
  doc["reasoning"] = false;
  doc["thinking"] = false;
  doc["scene"] = "chat";

  // 消息数组
  JsonArray messages = doc["messages"].to<JsonArray>();
  JsonObject msg = messages.add<JsonObject>();
  msg["content"] = message;
  msg["role"] = "user";
  msg["name"] = "string";

  // 响应格式
  JsonObject responseFormat = doc["response_format"].to<JsonObject>();
  responseFormat["type"] = "text";

  // 网络搜索配置
  JsonObject webSearch = doc["web_search"].to<JsonObject>();
  webSearch["enable"] = enableWebSearch;

  // 序列化为字符串
  String jsonString;
  serializeJson(doc, jsonString);
  return jsonString;
}

ChatApiResponse ChatApiClient::sendMessage(const String& message) {
  // 使用默认参数发送消息
  return sendMessage(message, generateConversationId(), true, false);
}

ChatApiResponse ChatApiClient::sendMessage(
  const String& message,
  const String& conversationId,
  bool enableWebSearch,
  bool stream
) {
  ChatApiResponse response;

  // 检查 WiFi 连接
  if (!isConnected()) {
    response.success = false;
    response.error = "WiFi not connected";
    Serial.println("[ChatAPI] Error: WiFi not connected");
    return response;
  }

  // 获取 token（优先使用 NVS 保存的，否则使用默认值）
  String activeToken = loadToken();
  if (activeToken.length() == 0) {
    activeToken = token;  // 使用默认 token
  }

  // 创建 HTTP 客户端
  HTTPClient http;
  WiFiClient client;

  // 构建 URL
  char url[256];
  snprintf(url, sizeof(url), "http://%s:%d%s", host.c_str(), port, CHAT_API_PATH);

  Serial.println("[ChatAPI] Connecting to: " + String(url));

  // 配置 HTTP 请求
  http.begin(client, url);
  http.setTimeout(CHAT_API_TIMEOUT);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "Bearer " + activeToken);

  // 如果提供了会话 ID，添加到请求头
  if (conversationId.length() > 0) {
    http.addHeader("X-Conversation-Id", conversationId);
  }

  // 构建请求体
  String requestBody = buildRequestBody(message, conversationId, enableWebSearch, stream);

  Serial.println("[ChatAPI] Request body:");
  Serial.println(requestBody);

  // 发送 POST 请求
  Serial.println("[ChatAPI] Sending request...");
  int httpCode = http.POST(requestBody);

  // 处理响应
  response.statusCode = httpCode;
  response.success = (httpCode == HTTP_CODE_OK);

  if (httpCode > 0) {
    Serial.printf("[ChatAPI] Response code: %d\n", httpCode);

    // 获取响应体
    String responseBody = http.getString();
    response.rawResponse = responseBody;

    Serial.println("[ChatAPI] Response body:");
    Serial.println(responseBody);

    // 检查是否是 401 Unauthorized（token 过期或无效）
    if (httpCode == 401) {
      Serial.println("[ChatAPI] Token invalid or expired, clearing...");
      clearToken();  // 清除保存的 token
    }

    // 解析 JSON 响应
    if (response.success) {
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, responseBody);

      if (!error) {
        // 尝试提取消息内容
        // 根据实际的 API 响应结构调整
        if (doc.containsKey("choices") && doc["choices"].is<JsonArray>()) {
          JsonArray choices = doc["choices"].as<JsonArray>();
          if (choices.size() > 0) {
            JsonObject choice = choices[0];
            if (choice.containsKey("message")) {
              JsonObject msg = choice["message"];
              if (msg.containsKey("content")) {
                response.content = msg["content"].as<String>();
              }
            }
          }
        } else if (doc.containsKey("content")) {
          // 直接返回内容
          response.content = doc["content"].as<String>();
        } else {
          // 如果无法解析，返回原始响应
          response.content = responseBody;
        }

        Serial.println("[ChatAPI] Parsed content:");
        Serial.println(response.content);
      } else {
        Serial.printf("[ChatAPI] JSON parse error: %s\n", error.c_str());
        response.content = responseBody;
      }
    } else {
      response.error = "HTTP Error: " + String(httpCode);
      Serial.printf("[ChatAPI] Request failed: %s\n", response.error.c_str());
    }
  } else {
    response.error = http.errorToString(httpCode);
    Serial.printf("[ChatAPI] Connection failed: %s\n", response.error.c_str());
  }

  // 清理
  http.end();

  return response;
}
