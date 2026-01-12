# ASR 模块与 ESP32 交互实现指南

## 概述

本文档详细说明了如何在 ESP32 项目中集成 ASR（自动语音识别）模块和 Chat API，实现完整的智能语音助手功能。

## 模块组成

### 1. **asr_manager** （ASR 管理器）
负责与 ASR 模块的底层串口通信

**文件**：
- `src/asr_manager.h` - 头文件，定义了 ASR 协议和接口
- `src/asr_manager.cpp` - 实现文件

**主要功能**：
- 初始化 UART1 (波特率 115200)
- 帧的收发和校验
- 识别结果解析
- TTS 播报请求
- 控制命令发送

**API 示例**：
```cpp
ASRManager asr;
asr.init();                              // 初始化
asr.sendCommand(ASR_CMD_START_LISTEN);  // 启动监听
if (asr.handleASR()) {                  // 处理接收数据
    ASRRecognitionResult result = asr.getLastResult();
    Serial.println(result.content);
}
asr.sendTTS("今天天气不错");            // 播报文本
```

---

### 2. **chat_api_client** （Chat API 客户端）
负责与云端聊天接口通信（已实现）

**主要功能**：
- HTTP 请求构建
- JSON 序列化
- 会话 ID 生成
- 响应解析

**API 示例**：
```cpp
ChatApiClient client;
client.setToken(token);
ChatApiResponse response = client.sendMessage("用户问题");
if (response.success) {
    Serial.println(response.content);  // AI 回复
}
```

---

### 3. **asr_chat_integration** （ASR-Chat 集成）
负责整个流程的状态管理和协调

**文件**：
- `src/asr_chat_integration.h` - 头文件
- `src/asr_chat_integration.cpp` - 实现文件

**主要功能**：
- 6 状态状态机管理
- ASR 识别 → Chat API → TTS 播报的完整流程
- 超时控制和错误处理

**API 示例**：
```cpp
ASRChatIntegration integration;
integration.init();
integration.startListening();  // 启动监听

void loop() {
    integration.handle();  // 自动处理状态转换
    
    if (integration.hasCompletedResult()) {
        IntegrationResult result = integration.getLastResult();
        Serial.printf("用户: %s\n", result.recognizedText.c_str());
        Serial.printf("AI:  %s\n", result.chatResponse.c_str());
    }
}
```

---

### 4. **asr_integration_example** （使用示例）
展示如何在实际项目中使用 ASR 集成模块

**文件**：
- `src/asr_integration_example.h` - 头文件
- `src/asr_integration_example.cpp` - 实现文件

**主要功能**：
- 通过 BLE 命令控制 ASR
- 结果处理回调
- 直接 Chat API 测试
- 状态显示

**支持的 BLE 命令**：
- `listen` / `start` - 启动监听
- `stop` - 停止监听
- `status` - 显示状态
- `test <文本>` - 直接测试 Chat API

---

## 硬件连接

### UART1 引脚配置
| 信号 | ESP32 GPIO | ASR 模块 |
|------|-----------|---------|
| TX   | GPIO17    | RX      |
| RX   | GPIO16    | TX      |
| GND  | GND       | GND     |
| VCC  | -         | 3.3V    |

### 可选：未来 ESP32 独立播放功能
| 信号 | ESP32 GPIO | MAX98357 |
|------|-----------|----------|
| I2S CLK | GPIO27 | BCLK     |
| I2S DIN | GPIO25 | DIN      |
| I2S LRCK | GPIO26 | LRCK     |
| 开启 | GPIO33 | SD       |

---

## 串口协议详解

### 帧格式

```
[START: 0xAA] [TYPE: 1B] [LENGTH: 2B] [DATA: N] [CHECKSUM: 1B] [END: 0xCC]
```

### 数据类型

| 值 | 方向 | 说明 |
|----|------|------|
| 0x01 | ASR → ESP32 | 识别结果 |
| 0x02 | ESP32 → ASR | TTS 播报 |
| 0x03 | ESP32 → ASR | 控制命令 |
| 0x04 | 双向 | ACK 确认 |
| 0x05 | 双向 | 错误 |

### 识别结果帧（0x01）

```
[置信度(1B)] [文本长度(1B)] [文本内容(UTF-8)]

示例（说"今天天气怎样"）：
数据: [0x85] [0x09] ["今天天气怎样"]
```

### TTS 播报帧（0x02）

```
[文本内容(UTF-8)]，最多 30 字符

示例（播报"今天晴朗"）：
数据: ["今天晴朗"]
```

### 控制命令帧（0x03）

```
[命令(1B)] [参数(1B)]

命令定义：
0x01 - 重置
0x02 - 启动监听
0x03 - 停止监听
0x04 - 获取版本
```

---

## 完整流程

### 基本流程（一次交互）

```
1. ESP32 → ASR: COMMAND(START_LISTEN) [0x03 0x02]
2. ASR 监听用户语音...
3. ASR → ESP32: RECOGNITION [0x01 0x85 0x09 ...]
4. ESP32 解析识别结果 → "今天天气怎样"
5. ESP32 → Chat API: 发送问题
6. Chat API → ESP32: 返回回复 → "今天晴朗，温度28度"
7. 截断至 30 字：" 今天晴朗，温度28度"
8. ESP32 → ASR: TTS("今天晴朗，温度28度") [0x02 ...]
9. ASR 通过喇叭播放回复
10. 返回步骤 1，继续监听
```

### 状态机流程

```
START
  ↓
[STATE_IDLE]
  ↑ (返回空闲)
  |
  +-- startListening() 触发
  |
  ↓
[STATE_WAITING_FOR_ASR] ← (启动 60 秒超时)
  │
  ├─ ASR 识别成功 → handleASRResult()
  │
  ├─ ASR 识别失败 → handleError()
  │
  └─ 60 秒超时 → handleError()
  
  ↓ (识别成功)
[STATE_SENDING_TO_CHAT]
  ↓ (立即调用 Chat API)
[STATE_WAITING_FOR_CHAT]
  ├─ API 成功 → chatResponse 已填充
  │  ↓
  │  [STATE_SENDING_TTS]
  │  ↓
  │  asr.sendTTS(chatResponse)
  │  ↓
  │  resultAvailable = true
  │  ↓
  │  [STATE_IDLE]
  │
  └─ API 失败 → handleError()
     ↓
     [STATE_ERROR]
     ↓
     asr.sendTTS("出错了")
     ↓
     resultAvailable = true
     ↓
     [STATE_IDLE]
```

---

## 在现有项目中集成

### 第一步：添加 ASR 头文件到 main.cpp

```cpp
#include "asr_chat_integration.h"
#include "asr_integration_example.h"
```

### 第二步：在 setup() 中初始化

```cpp
void setup() {
    // ... 现有初始化代码 ...
    
    // 初始化 ASR-Chat 集成
    initASRIntegrationExample("192.168.0.103", 10089);
}
```

### 第三步：在 loop() 中处理

```cpp
void loop() {
    // ... 现有代码 ...
    
    // 处理 ASR-Chat 集成
    handleASRIntegrationLoop();
    
    // ... 其他代码 ...
}
```

### 第四步：在 BLE 命令处理中添加

修改 `src/bt_manager.cpp` 的 `handleBluetoothCommand()` 函数：

```cpp
if (command == "listen") {
    handleASRBLECommand("listen");
} else if (command == "stop") {
    handleASRBLECommand("stop");
} else if (command == "status") {
    handleASRBLECommand("status");
} else if (command.startsWith("test ")) {
    handleASRBLECommand(command);
}
```

---

## 编译和部署

### 编译

使用 PlatformIO：
```bash
pio run -e esp32dev
```

### 上传

```bash
pio run -e esp32dev -t upload
```

### 监控

```bash
pio device monitor -e esp32dev --port COM3
```

---

## 常见问题

| 问题 | 原因 | 解决方案 |
|------|------|---------|
| ASR 无响应 | UART 未初始化或引脚错误 | 检查 GPIO16/17，确保 ASR 上电 |
| 识别乱码 | UTF-8 编码问题 | 确保 ASR 模块发送的是有效 UTF-8 |
| Chat API 超时 | 网络或 Token 过期 | 检查 WiFi，通过 BLE 更新 Token |
| TTS 截断 | 文本超过 30 字 | 正常行为，系统会自动截断 |
| 频繁崩溃 | 内存溢出 | 增加延时，减少并发操作 |

---

## 性能指标

- **ASR 识别延迟**：通常 < 2 秒（取决于语音长度）
- **Chat API 延迟**：5-10 秒（网络依赖）
- **总处理时间**：7-15 秒（从识别到播报完成）
- **内存占用**：约 50-80 KB（JSON 缓冲、UART 缓冲等）
- **UART 波特率**：115200 bps（可在 config.h 中调整）

---

## 安全性考虑

1. **Token 管理**：Token 通过 BLE 配置，存储在 NVS 加密分区
2. **会话 ID**：每次请求使用唯一的会话 ID 防止重放攻击
3. **超时保护**：设置多层超时防止无限等待
4. **校验和验证**：所有帧都进行 XOR 校验，防止数据损坏

---

## 扩展建议

1. **记录完整的对话历史**到 SPIFFS，用于调试
2. **添加 LED 指示灯**表示当前状态
3. **实现 OTA 更新**能力
4. **支持多语言**（修改 Chat API 的 system prompt）
5. **添加本地命令处理**（不调用 API 的内置命令）

---

## 参考资源

- [Chat API 文档](chat-api.md)
- [ESP32 ASR 流程设计](esp_32_asr_pipeline.md)
- [BLE 跨平台指南](BLE_CROSS_PLATFORM_GUIDE.md)
- [PlatformIO 指南](PlatformIO指南.md)

