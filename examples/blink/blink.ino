/*
 * ESP32 LED 闪烁示例
 *
 * 功能: 让板载 LED 每秒闪烁一次
 * 硬件: ESP32-WROOM-32 开发板
 * LED 引脚: GPIO2
 *
 * 修改日期: 2026-01-12
 */

// LED 引脚定义
const int LED_PIN = 2;

// 闪烁间隔（毫秒）
const int BLINK_INTERVAL = 1000;

void setup() {
  // 初始化串口通信
  Serial.begin(115200);
  delay(1000);

  // 设置 LED 引脚为输出模式
  pinMode(LED_PIN, OUTPUT);

  // 输出系统信息
  Serial.println("\n╔══════════════════════════════════╗");
  Serial.println("║   ESP32 LED 闪烁程序              ║");
  Serial.println("╚══════════════════════════════════╝");
  Serial.printf("CPU 频率: %d MHz\n", ESP.getCpuFreqMHz());
  Serial.printf("Flash 大小: %d MB\n", ESP.getFlashChipSize() / (1024 * 1024));
  Serial.printf("自由内存: %d bytes\n", ESP.getFreeHeap());
  Serial.printf("LED 引脚: GPIO %d\n", LED_PIN);
  Serial.println("────────────────────────────────────\n");
}

void loop() {
  // LED 点亮
  digitalWrite(LED_PIN, HIGH);
  Serial.println("LED 开启");
  delay(BLINK_INTERVAL);

  // LED 熄灭
  digitalWrite(LED_PIN, LOW);
  Serial.println("LED 关闭");
  delay(BLINK_INTERVAL);
}
