# PlatformIO + VSCode 开发指南

本指南介绍如何使用 VSCode + PlatformIO 进行 ESP32 开发。

---

## 📋 目录
- [安装步骤](#安装步骤)
- [项目结构](#项目结构)
- [常用命令](#常用命令)
- [调试方法](#调试方法)
- [常见问题](#常见问题)

---

## 🔧 安装步骤

### 1. 安装 VSCode

下载并安装 VSCode：
- **Windows**: https://code.visualstudio.com/
- **Mac**: https://code.visualstudio.com/
- **Linux**: `sudo apt install code`

### 2. 安装 PlatformIO 扩展

1. 打开 VSCode
2. 点击左侧扩展图标 (或按 `Ctrl+Shift+X`)
3. 搜索 "PlatformIO IDE"
4. 点击安装
5. **重启 VSCode**

### 3. 等待 PlatformIO 初始化

首次启动时，PlatformIO 会自动下载必要的工具：
- Python 环境
- 平台工具链
- 编译器

**注意**: 首次初始化可能需要 10-30 分钟，请耐心等待。

---

## 📁 项目结构

PlatformIO 项目标准结构：

```
ESP32-Dev-Board/
├── .pio/                    # PlatformIO 生成的文件 (自动创建)
├── .vscode/                 # VSCode 配置
├── include/                 # 头文件目录
│   └── config.h            # 配置文件
├── src/                     # 源代码目录
│   └── main.cpp            # 主程序
├── lib/                     # 自定义库
├── test/                    # 测试文件
├── platformio.ini           # 项目配置文件 ⭐
└── README.md
```

### 关键文件说明

#### [platformio.ini](../platformio.ini)
项目主配置文件，包含：
- 开发板和平台配置
- 编译选项
- 库依赖
- 上传配置

#### [src/main.cpp](../src/main.cpp)
主程序文件，必须包含 `setup()` 和 `loop()` 函数。

#### [include/config.h](../include/config.h)
配置文件，定义常量、引脚、网络等参数。

---

## 💻 常用命令

### 方式一：使用 VSCode 底部栏

PlatformIO 安装后，VSCode 底部会出现快捷按钮：

| 按钮 | 功能 |
|------|------|
| 👁️ | 勾选符号 - 构建项目 |
| ➡️ | 箭头符号 - 上传到开发板 |
| 🔌 | 插头符号 - 打开串口监视器 |
| 🗑️ | 垃圾桶 - 清理构建文件 |

### 方式二：使用命令面板

按 `Ctrl+Shift+P` (Mac: `Cmd+Shift+P`)，输入以下命令：

- `PlatformIO: Build` - 编译项目
- `PlatformIO: Upload` - 上传程序
- `PlatformIO: Monitor` - 打开串口监视器
- `PlatformIO: Clean` - 清理构建

### 方式三：使用终端

在 VSCode 终端 (`Ctrl+``) 中：

```bash
# 编译项目
pio run

# 上传到开发板
pio run --target upload

# 打开串口监视器
pio device monitor

# 编译并上传
pio run --target upload && pio device monitor

# 清理构建
pio run --target clean

# 显示所有可用端口
pio device list

# 设置串口 (如果不自动检测)
pio device monitor --port COM3

# 设置波特率
pio device monitor --baud 115200
```

---

## 🚀 快速开始

### 第一个项目：LED 闪烁

#### 1. 打开项目

```bash
# 在 VSCode 中
文件 → 打开文件夹 → 选择 ESP32-Dev-Board
```

#### 2. 等待 PlatformIO 识别项目

打开后，PlatformIO 会自动识别项目结构。

#### 3. 编译项目

- 点击底部栏的 👁️ 按钮
- 或按 `Ctrl+Shift+P` → `PlatformIO: Build`

#### 4. 连接开发板

用 USB 线连接 ESP32 开发板到电脑。

#### 5. 上传程序

- 点击底部栏的 ➡️ 按钮
- 或按 `Ctrl+Shift+P` → `PlatformIO: Upload`

#### 6. 查看串口输出

- 点击底部栏的 🔌 按钮
- 或按 `Ctrl+Shift+P` → `PlatformIO: Monitor`

你将看到 LED 闪烁和串口输出！

---

## 📝 代码示例

### 示例 1：LED 闪烁

[src/main.cpp](../src/main.cpp)

```cpp
#include <Arduino.h>

#define LED_PIN 2

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  Serial.println("ESP32 启动!");
}

void loop() {
  digitalWrite(LED_PIN, HIGH);
  Serial.println("LED 开");
  delay(1000);

  digitalWrite(LED_PIN, LOW);
  Serial.println("LED 关");
  delay(1000);
}
```

### 示例 2：Wi-Fi 连接

创建 `src/wifi_demo.cpp`:

```cpp
#include <Arduino.h>
#include <WiFi.h>

#include "config.h"  // 包含配置文件

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.printf("连接到: %s\n", WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWi-Fi 连接成功!");
  Serial.printf("IP 地址: %s\n", WiFi.localIP().toString().c_str());
}

void loop() {
  // 定期检查连接状态
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("连接断开，重连中...");
    WiFi.reconnect();
  }
  delay(10000);
}
```

### 示例 3：Web 服务器

```cpp
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

#include "config.h"

WebServer server(WEB_SERVER_PORT);

void handleRoot() {
  String html = "<h1>ESP32 Web Server</h1>";
  html += "<p>GPIO 控制</p>";
  html += "<a href='/on'><button>LED 开</button></a> ";
  html += "<a href='/off'><button>LED 关</button></a>";
  server.send(200, "text/html", html);
}

void handleOn() {
  digitalWrite(LED_PIN, HIGH);
  server.send(200, "text/plain", "LED 开启");
}

void handleOff() {
  digitalWrite(LED_PIN, LOW);
  server.send(200, "text/plain", "LED 关闭");
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWi-Fi 连接成功!");
  Serial.printf("访问: http://%s\n", WiFi.localIP().toString().c_str());

  server.on("/", handleRoot);
  server.on("/on", handleOn);
  server.on("/off", handleOff);

  server.begin();
}

void loop() {
  server.handleClient();
}
```

---

## 🔍 调试方法

### 1. 串口调试

```cpp
Serial.begin(115200);

// 打印文本
Serial.println("调试信息");

// 格式化输出
Serial.printf("温度: %.2f°C\n", temperature);

// 打印变量
int value = 42;
Serial.printf("变量值: %d\n", value);
```

### 2. 串口监视器快捷键

在串口监视器中：

- `Ctrl+C` - 退出监视器
- `Ctrl+T` - 打开命令菜单

常用命令：
- `Ctrl+T` + `e` - 切换自动回显
- `Ctrl+T` + `b` - 改变波特率
- `Ctrl+T` + `l` - 切换行结束符

### 3. 异常解码

PlatformIO 自动解码 ESP32 异常信息（Guru Meditation Error）。

在 [platformio.ini](../platformio.ini) 中已配置：
```ini
monitor_filters = esp32_exception_decoder
```

### 4. 高级调试（需要硬件调试器）

使用 JTAG/调试器（如 ESP-PROG）：

```ini
[env:esp32dev]
debug_tool = esp-prog
debug_init_break = tbreak setup
```

然后按 `F5` 启动调试。

---

## 📚 使用库

### 方式 1：通过 platformio.ini 添加（推荐）

编辑 [platformio.ini](../platformio.ini)：

```ini
lib_deps =
    adafruit/Adafruit GFX Library@1.11.9
    adafruit/Adafruit SSD1306@2.5.7
    knolleary/PubSubClient@2.8
```

PlatformIO 会自动下载并链接这些库。

### 方式 2：通过 VSCode 界面

1. 按 `Ctrl+Shift+P`
2. 输入 `PlatformIO: Install Library`
3. 搜索并选择库

### 方式 3：手动安装

将库文件放到 `lib/` 目录。

---

## ⚙️ 配置选项详解

### platformio.ini 配置项

#### 基本配置
```ini
[env:esp32dev]
platform = espressif32      # 开发平台
board = esp32dev            # 开发板型号
framework = arduino         # 框架选择 (arduino, espidf)
```

#### 串口配置
```ini
monitor_speed = 115200      # 监视器波特率
upload_speed = 921600       # 上传速度
```

#### 编译选项
```ini
build_flags =               # 编译标志
    -DDEBUG_MODE=1
    -O2                     # 优化级别
```

#### 分区方案
```ini
board_build.partitions = default.csv   # 默认分区
board_build.partitions = min_spiffs.csv  # 最小 SPIFFS
board_build.partitions = no_ota.csv       # 无 OTA
```

#### 常用分区方案

| 分区方案 | APP 大小 | SPIFFS 大小 | 说明 |
|---------|---------|------------|------|
| default.csv | 1.2MB | 1.5MB | 默认，支持 OTA |
| min_spiffs.csv | 1.9MB | 0.9MB | 更大程序空间 |
| no_ota.csv | 3MB | 0.5MB | 无 OTA，最大程序空间 |

---

## 🎯 最佳实践

### 1. 使用配置文件

在 `include/config.h` 中集中管理配置：

```cpp
// config.h
#define WIFI_SSID "你的WiFi"
#define WIFI_PASSWORD "你的密码"
#define LED_PIN 2
```

在代码中引用：

```cpp
#include "config.h"

WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
digitalWrite(LED_PIN, HIGH);
```

### 2. 使用调试宏

```cpp
#define DEBUG_MODE true

#if DEBUG_MODE
  #define DEBUG_PRINT(x) Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
#endif

// 使用
DEBUG_PRINTLN("调试信息");
```

### 3. 分离功能到不同文件

```
src/
├── main.cpp        # 主程序
├── wifi.cpp        # WiFi 相关功能
├── server.cpp      # Web 服务器
└── sensors.cpp     # 传感器读取
```

在 `main.cpp` 中声明：

```cpp
void setupWiFi();
void setupServer();
void readSensors();
```

---

## 🐛 常见问题

### Q1: PlatformIO 初始化太慢？

**A**: 首次安装需要下载工具链和编译器，请耐心等待。可以尝试：
- 检查网络连接
- 使用国内镜像（设置环境变量）

### Q2: 编译错误 "PlatformIO not found"

**A**:
1. 确认 PlatformIO 扩展已安装
2. 重启 VSCode
3. 如果还不行，重新安装扩展

### Q3: 上传失败 "Failed to connect"

**A**:
1. 检查串口号是否正确
2. 按住 BOOT 按钮，点击上传
3. 降低上传速度到 115200
4. 更换 USB 线

### Q4: 串口监视器乱码

**A**:
1. 检查波特率设置是否为 115200
2. 检查编码是否为 UTF-8
3. 尝试重启串口监视器

### Q5: 找不到串口

**A**:
1. 检查 CH340 驱动是否安装
2. 更换 USB 线
3. 在设备管理器中查看 COM 端口
4. 手动指定端口：
   ```ini
   upload_port = COM3
   monitor_port = COM3
   ```

### Q6: 编译太慢？

**A**:
1. 启用并行编译（默认已启用）
2. 使用 SSD
3. 增加内存
4. 清理构建：`pio run --target clean`

### Q7: 库依赖冲突？

**A**:
1. 检查 `lib_deps` 中是否有重复
2. 删除 `.pio/lib_deps` 文件夹
3. 重新构建

---

## 📖 进阶功能

### 1. 单元测试

在 `test/` 目录创建测试文件：

```cpp
// test/test_main.cpp
#include <unity.h>

void test_led_pin(void) {
    TEST_ASSERT_EQUAL(2, LED_PIN);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_led_pin);
    return UNITY_END();
}
```

运行测试：
```bash
pio test
```

### 2. OTA (Over-The-Air) 升级

在 `platformio.ini` 添加：

```ini
lib_deps =
    esp32Arduino/ESP32OTA@2.0.0
```

### 3. 文件系统 (SPIFFS)

上传文件到 SPIFFS：
```bash
pio run --target uploadfs
```

### 4. 多环境配置

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino

[env:esp32-c3]
platform = espressif32
board = esp32-c3-devkitm-1
framework = arduino
```

---

## 🔗 参考资源

- [PlatformIO 官方文档](https://docs.platformio.org/)
- [PlatformIO ESP32 文档](https://docs.platformio.org/en/latest/boards/espressif32/esp32dev.html)
- [VSCode 快捷键](https://code.visualstudio.com/shortcuts/keyboard-shortcuts-windows.pdf)
- [Arduino ESP32 核心](https://github.com/espressif/arduino-esp32)

---

## 🎓 下一步

1. 熟悉 VSCode 快捷键
2. 阅读 [PlatformIO 文档](https://docs.platformio.org/)
3. 尝试更多示例代码
4. 学习 ESP-IDF 框架

祝你开发顺利！🚀
