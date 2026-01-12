/*
 * ESP32 Wi-Fi 连接示例
 *
 * 功能: 连接到 Wi-Fi 网络，并显示连接信息
 * 硬件: ESP32-WROOM-32 开发板
 *
 * 修改日期: 2026-01-12
 */

#include <WiFi.h>

// ==================== 配置区域 ====================
// 请修改为你的 WiFi 信息
const char* ssid = "你的WiFi名称";
const char* password = "你的WiFi密码";
// ==================================================

// 连接超时时间（毫秒）
const unsigned long CONNECTION_TIMEOUT = 20000;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n╔══════════════════════════════════╗");
  Serial.println("║   ESP32 Wi-Fi 连接程序           ║");
  Serial.println("╚══════════════════════════════════╝\n");

  // 设置 Wi-Fi 模式为站点模式
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);

  // 开始连接
  Serial.printf("正在连接到: %s\n", ssid);
  Serial.println("────────────────────────────────────");

  unsigned long startTime = millis();
  WiFi.begin(ssid, password);

  // 等待连接
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");

    // 检查超时
    if (millis() - startTime > CONNECTION_TIMEOUT) {
      Serial.println("\n\n✗ 连接超时!");
      printConnectionHelp();
      while(1) {
        delay(1000);
        // 可以在这里添加重连逻辑
      }
    }
  }

  Serial.println("\n\n✓ Wi-Fi 连接成功!");
  Serial.println("────────────────────────────────────");
  printNetworkInfo();
  Serial.println("────────────────────────────────────");
}

void loop() {
  // 定期检查连接状态
  static unsigned long lastCheck = 0;

  if (millis() - lastCheck > 10000) {  // 每 10 秒检查一次
    lastCheck = millis();

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("═══════════════════════════════════");
      Serial.println("连接状态: 正常");
      Serial.printf("IP 地址: %s\n", WiFi.localIP().toString().c_str());
      Serial.printf("信号强度: %d dBm\n", WiFi.RSSI());
      Serial.println("═══════════════════════════════════");
    } else {
      Serial.println("✗ 连接断开!");
      Serial.println("尝试重新连接...");
      WiFi.reconnect();
    }
  }

  delay(100);
}

// 打印网络信息
void printNetworkInfo() {
  Serial.printf("SSID (网络名称): %s\n", WiFi.SSID().c_str());
  Serial.printf("BSSID (AP MAC): %s\n", WiFi.BSSIDstr().c_str());
  Serial.printf("IP 地址: %s\n", WiFi.localIP().toString().c_str());
  Serial.printf("子网掩码: %s\n", WiFi.subnetMask().toString().c_str());
  Serial.printf("网关地址: %s\n", WiFi.gatewayIP().toString().c_str());
  Serial.printf("DNS 服务器: %s\n", WiFi.dnsIP().toString().c_str());
  Serial.printf("信号强度 (RSSI): %d dBm\n", WiFi.RSSI());
  Serial.printf("信道: %d\n", WiFi.channel());

  // 信号质量评估
  int rssi = WiFi.RSSI();
  Serial.printf("信号质量: ", rssi);
  if (rssi > -50) {
    Serial.println("优秀");
  } else if (rssi > -60) {
    Serial.println("良好");
  } else if (rssi > -70) {
    Serial.println("一般");
  } else {
    Serial.println("较差");
  }
}

// 打印连接帮助
void printConnectionHelp() {
  Serial.println("\n────────────────────────────────────");
  Serial.println("连接失败可能的原因:");
  Serial.println("1. WiFi 名称 (SSID) 输入错误");
  Serial.println("2. WiFi 密码输入错误");
  Serial.println("3. WiFi 路由器距离太远");
  Serial.println("4. 使用了 5GHz WiFi（ESP32 仅支持 2.4GHz）");
  Serial.println("5. 路由器启用了 MAC 地址过滤");
  Serial.println("6. WiFi 路由器不支持 DHCP");
  Serial.println("\n建议解决方法:");
  Serial.println("- 请确认 WiFi 名称和密码正确");
  Serial.println("- 使用手机开启 2.4GHz 热点进行测试");
  Serial.println("- 将 ESP32 靠近 WiFi 路由器");
  Serial.println("────────────────────────────────────");
}
