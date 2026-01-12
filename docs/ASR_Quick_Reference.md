# ASR + Chat API 快速参考卡

## 🚀 快速开始（5 分钟）

### 1. 复制文件到项目
```bash
# ASR 管理器
src/asr_manager.h
src/asr_manager.cpp

# Chat API 客户端（已存在）
src/chat_api_client.h
src/chat_api_client.cpp

# 集成模块（新）
src/asr_chat_integration.h
src/asr_chat_integration.cpp

# 使用示例（新）
src/asr_integration_example.h
src/asr_integration_example.cpp
```

### 2. 修改 main.cpp

**在 include 部分添加**：
```cpp
#include "asr_chat_integration.h"
#include "asr_integration_example.h"
```

**在 setup() 末尾添加**：
```cpp
initASRIntegrationExample("192.168.0.103", 10089);
```

**在 loop() 末尾添加**：
```cpp
handleASRIntegrationLoop();
```

### 3. 编译上传
```bash
pio run -e esp32dev -t upload
pio device monitor -e esp32dev --port COM3
```

### 4. 通过 BLE 测试
发送命令：`listen` → 开始监听

---

## 📡 硬件连接

```
ASR 模块        ESP32 引脚
─────────       ─────────
RX       ←→     GPIO17 (TX)
TX       ←→     GPIO16 (RX)
GND      ←→     GND
3.3V     ←→     3.3V
```

---

## 🔧 核心 API

### ASRManager（底层）

```cpp
ASRManager asr;
asr.init();                             // 初始化 UART1
asr.sendCommand(ASR_CMD_START_LISTEN); // 启动监听
asr.handleASR();                        // 处理接收数据
asr.sendTTS("hello");                   // 播报文本（max 30字）
```

### ASRChatIntegration（集成）

```cpp
ASRChatIntegration integration;
integration.init();                     // 初始化
integration.startListening();           // 启动监听

void loop() {
    integration.handle();               // 自动状态转换
    if (integration.hasCompletedResult()) {
        IntegrationResult result = integration.getLastResult();
        Serial.println(result.chatResponse);
    }
}
```

### 示例接口（高层）

```cpp
initASRIntegrationExample();            // 初始化
handleASRIntegrationLoop();             // 主循环处理
handleASRBLECommand("listen");          // BLE 命令处理
testChatAPIDirectly("测试文本");        // 直接测试 API
showASRStatus();                        // 显示状态
```

---

## 📊 状态机一览

```
START → IDLE 
    ↓
  startListening()
    ↓
WAITING_FOR_ASR (等待识别)
    ↓ (收到识别)
SENDING_TO_CHAT (发送 API)
    ↓ (立即调用)
WAITING_FOR_CHAT (等待响应)
    ↓ (收到响应)
SENDING_TTS (发送播报)
    ↓
IDLE (结果可用)
```

---

## 📝 ASR 协议速记

### 帧结构
```
[AA] [TYPE] [LEN_L] [LEN_H] [DATA...] [CHECKSUM] [CC]
```

### 常用数据类型
- 0x01：识别结果（ASR→ESP32）
- 0x02：TTS 播报（ESP32→ASR）
- 0x03：控制命令（ESP32→ASR）

### 控制命令
- 0x02：启动监听
- 0x03：停止监听

---

## 🎯 BLE 命令列表

| 命令 | 功能 |
|------|------|
| `listen` | 启动 ASR 监听 |
| `stop` | 停止监听 |
| `status` | 显示状态 |
| `test 你好` | 直接测试 Chat API |

---

## ⚙️ 配置参数

修改 `src/asr_manager.h`：

```cpp
#define ASR_UART_BAUDRATE 115200      // 波特率
#define ASR_RX_PIN 16                 // RX 引脚
#define ASR_TX_PIN 17                 // TX 引脚
#define ASR_TTS_MAX_LENGTH 30         // TTS 最大长度
#define ASR_FRAME_START 0xAA          // 帧起始
#define ASR_FRAME_END 0xCC            // 帧结束
```

修改 `src/asr_chat_integration.h`：

```cpp
#define CHAT_RESPONSE_MAX_LENGTH 30   // 截断长度
#define CHAT_TIMEOUT 30000            // API 超时
#define STATE_TIMEOUT 60000           // 状态超时
```

---

## 🐛 调试技巧

### 查看日志
```
[ASR] ...          // ASR 模块日志
[ChatAPI] ...      // Chat API 日志  
[Integration] ...  // 集成模块日志
```

### 常见错误

**"ASR 无响应"**
- 检查引脚连接（GPIO16/17）
- 检查波特率（115200）
- 检查 ASR 模块是否上电

**"Chat API 超时"**
- 检查 WiFi 连接
- 检查 Token 是否有效
- 检查 API 地址和端口

**"识别乱码"**
- 确保使用 UTF-8 编码
- 检查校验和是否正确
- 检查数据帧完整性

---

## 📊 性能指标

| 指标 | 值 |
|------|-----|
| ASR 识别延迟 | < 2s |
| Chat API 延迟 | 5-10s |
| 总处理时间 | 7-15s |
| 内存占用 | 50-80 KB |
| 串口波特率 | 115200 bps |

---

## 🎓 示例代码片段

### 最小化初始化
```cpp
void setup() {
    ASRChatIntegration integration;
    integration.init();
    integration.startListening();
}

void loop() {
    integration.handle();
    if (integration.hasCompletedResult()) {
        Serial.println(integration.getLastResult().chatResponse);
        integration.clearResultFlag();
    }
}
```

### 带回调的初始化
```cpp
void myResultCallback(const IntegrationResult& result) {
    if (result.success) {
        Serial.printf("用户: %s\n", result.recognizedText.c_str());
        Serial.printf("AI: %s\n", result.chatResponse.c_str());
    }
}

void setup() {
    initASRIntegrationExample("192.168.0.103", 10089, myResultCallback);
}

void loop() {
    handleASRIntegrationLoop();
}
```

---

## 📚 文档链接

- [完整集成指南](ASR_Integration_Guide.md)
- [ESP32 ASR 流程设计](esp_32_asr_pipeline.md)
- [Chat API 文档](chat-api.md)
- [BLE 跨平台指南](BLE_CROSS_PLATFORM_GUIDE.md)

---

## ✅ 检查清单

- [ ] 硬件连接正确（GPIO16/17）
- [ ] 包含所有新文件（asr_*.*, *_integration*.*)
- [ ] 修改 main.cpp 添加初始化和循环
- [ ] 编译通过（pio run）
- [ ] 上传成功
- [ ] 通过 BLE 发送 `listen` 命令
- [ ] 监控中看到 `[ASR]` 和 `[Integration]` 日志
- [ ] 能成功识别语音并获得 API 回复

---

**最后更新**：2026-01-12  
**版本**：1.0
