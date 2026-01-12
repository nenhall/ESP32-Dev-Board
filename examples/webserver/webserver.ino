/*
 * ESP32 Web 服务器示例
 *
 * 功能: 创建一个 Web 服务器，通过浏览器控制 LED
 * 硬件: ESP32-WROOM-32 开发板
 *
 * 修改日期: 2026-01-12
 */

#include <WiFi.h>
#include <WebServer.h>

// ==================== 配置区域 ====================
const char* ssid = "你的WiFi名称";
const char* password = "你的WiFi密码";

// Web 服务器端口
const int SERVER_PORT = 80;

// LED 引脚
const int LED_PIN = 2;
// ==================================================

// 创建 Web 服务器对象
WebServer server(SERVER_PORT);

// 存储开关次数
int toggleCount = 0;

// 处理根路径
void handleRoot() {
  String html = "<!DOCTYPE html>";
  html += "<html>";
  html += "<head>";
  html += "<meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>ESP32 控制面板</title>";
  html += "<style>";
  html += "body { font-family: Arial; max-width: 400px; margin: 50px auto; padding: 20px; text-align: center; background-color: #f5f5f5; }";
  html += "h1 { color: #333; }";
  html += ".status { padding: 15px; margin: 20px 0; border-radius: 5px; background-color: ";
  html += (digitalRead(LED_PIN) ? "#d4edda; color: #155724;" : "#f8d7da; color: #721c24;");
  html += "; font-weight: bold; }";
  html += "button { padding: 15px 30px; margin: 10px; font-size: 16px; border: none; border-radius: 5px; cursor: pointer; }";
  html += ".btn-on { background-color: #28a745; color: white; }";
  html += ".btn-off { background-color: #dc3545; color: white; }";
  html += ".info { background-color: #fff; padding: 15px; margin: 20px 0; border-radius: 5px; border-left: 4px solid #007bff; }";
  html += "</style>";
  html += "</head>";
  html += "<body>";

  html += "<h1>ESP32 控制面板</h1>";

  html += "<div class='status'>";
  html += "LED 状态: ";
  html += (digitalRead(LED_PIN) ? "开启 ●" : "关闭 ○");
  html += "</div>";

  html += "<p>";
  html += "<a href='/on'><button class='btn-on'>LED 开</button></a> ";
  html += "<a href='/off'><button class='btn-off'>LED 关</button></a>";
  html += "</p>";

  html += "<div class='info'>";
  html += "<p>开关次数: " + String(toggleCount) + "</p>";
  html += "<p>运行时间: " + String(millis() / 1000) + " 秒</p>";
  html += "<p>自由内存: " + String(ESP.getFreeHeap() / 1024) + " KB</p>";
  html += "</div>";

  html += "</body>";
  html += "</html>";

  server.send(200, "text/html; charset=utf-8", html);
}

// 处理 LED 开
void handleOn() {
  digitalWrite(LED_PIN, HIGH);
  toggleCount++;

  Serial.println("LED 开启");

  // 发送 JSON 响应
  String json = "{\"status\":\"on\", \"count\":" + String(toggleCount) + "}";
  server.send(200, "application/json", json);

  // 延迟后重定向回主页
  delay(100);
  server.sendHeader("Location", "/");
  server.send(303);
}

// 处理 LED 关
void handleOff() {
  digitalWrite(LED_PIN, LOW);
  toggleCount++;

  Serial.println("LED 关闭");

  // 发送 JSON 响应
  String json = "{\"status\":\"off\", \"count\":" + String(toggleCount) + "}";
  server.send(200, "application/json", json);

  // 延迟后重定向回主页
  delay(100);
  server.sendHeader("Location", "/");
  server.send(303);
}

// 处理 404
void handleNotFound() {
  String message = "404 Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  server.send(404, "text/plain", message);
}

// 处理 API 状态查询
void handleStatus() {
  String json = "{";
  json += "\"led\":" + String(digitalRead(LED_PIN)) + ",";
  json += "\"count\":" + String(toggleCount) + ",";
  json += "\"uptime\":" + String(millis() / 1000) + ",";
  json += "\"heap\":" + String(ESP.getFreeHeap()) + ",";
  json += "\"rssi\":" + String(WiFi.RSSI());
  json += "}";

  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  // 初始化 LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Serial.println("\n╔══════════════════════════════════╗");
  Serial.println("║   ESP32 Web 服务器              ║");
  Serial.println("╚══════════════════════════════════╝\n");

  // 连接 WiFi
  WiFi.mode(WIFI_STA);
  Serial.printf("正在连接到: %s\n", ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n\n✓ WiFi 连接成功!");
  Serial.println("────────────────────────────────────");
  Serial.printf("IP 地址: %s\n", WiFi.localIP().toString().c_str());
  Serial.printf("信号强度: %d dBm\n", WiFi.RSSI());
  Serial.printf("服务器端口: %d\n", SERVER_PORT);
  Serial.println("────────────────────────────────────\n");

  // 设置路由
  server.on("/", handleRoot);
  server.on("/on", handleOn);
  server.on("/off", handleOff);
  server.on("/status", handleStatus);
  server.onNotFound(handleNotFound);

  // 启动服务器
  server.begin();
  Serial.println("✓ Web 服务器启动成功!");
  Serial.printf("访问地址: http://%s\n", WiFi.localIP().toString().c_str());
  Serial.println("────────────────────────────────────\n");
}

void loop() {
  // 处理客户端请求
  server.handleClient();

  // 定期输出状态
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 60000) {  // 每分钟
    lastPrint = millis();
    Serial.println("服务器运行中...");
    Serial.printf("连接数: %d, 自由内存: %d bytes\n",
                  WiFi.softAPgetStationNum(),
                  ESP.getFreeHeap());
  }
}
