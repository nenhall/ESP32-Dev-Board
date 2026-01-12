# ESP32 智能音箱方案 A 开发文档（Markdown）

## 1. 项目目标

实现一个基础智能音箱，先跑通完整流程：

- 使用 ASR 模块监听 + 唤醒 + 指令识别
- ESP32 处理联网请求（调用云端 AI / 天气接口）
- ASR 自带喇叭负责提示音 / 简短结果播报
- BLE 用于手机配置 Token / WiFi

> 说明：该方案为阶段 1，先验证整体逻辑链路，不涉及 ESP32 独立播报。

---

## 2. 系统架构

```
用户语音
   ↓
ASR 模块（监听 + 指令识别 + TTS 播报）
   ↓ UART
ESP32（WiFi 连接 + 云 API 调用）
   ↓ UART
ASR 模块（播报短结果 / 提示音）
   ↑
BLE（手机配置 Token / WiFi）
```

---

## 3. 硬件组成

- **ESP32**：WiFi + BLE + 串口处理
- **ASR 模块**：离线语音识别 + 自带喇叭
- **麦克风**：通常在 ASR 模块内置
- **喇叭**：内置在 ASR 模块
- **电源**：根据板子需求，通常 5V / 3.3V

---

## 4. BLE 配置设计

BLE 仅用于配置 ESP32：

- Service UUID: `xxxxxxxxx`
- RX（Write）：手机 → ESP32
- TX（Notify）：ESP32 → 手机

ESP32 收到后保存到 NVS，并返回配置成功状态。

---

## 5. ASR 串口协议设计

### 5.1 物理层配置

- **UART 编号**：UART1
- **波特率**：115200 bps
- **数据位**：8
- **停止位**：1
- **奇偶校验**：无
- **流控**：无
- **ESP32 引脚**：TX=GPIO17, RX=GPIO16

### 5.2 帧格式设计

```
┌─────┬──────┬─────────┬──────────────┬──────────┬─────┐
│起始 │ 类型 │ 长度(2) │   数据       │ 校验和   │结束  │
│0xAA │ TYPE │ LEN_L   │   ...        │ CHECKSUM │0xCC  │
│     │      │ LEN_H   │              │          │      │
└─────┴──────┴─────────┴──────────────┴──────────┴─────┘
```

**字段说明**：
- 起始：固定 0xAA
- 类型：数据类型（见下表）
- 长度：数据长度，小端字节序（低字节在前）
- 数据：实际内容
- 校验和：对数据部分进行 XOR 异或
- 结束：固定 0xCC

### 5.3 数据类型定义

| 类型值 | 名称 | 方向 | 说明 |
|--------|------|------|------|
| 0x01 | RECOGNITION | ASR→ESP32 | 语音识别结果 |
| 0x02 | TTS | ESP32→ASR | TTS 播报请求 |
| 0x03 | COMMAND | ESP32→ASR | 控制命令 |
| 0x04 | ACK | 双向 | 确认帧 |
| 0x05 | ERROR | 双向 | 错误信息 |

### 5.4 识别结果帧格式（0x01）

**ASR → ESP32**（用户说话后，ASR 发送识别结果）

```
数据字段：
[CONFIDENCE(1)] [CONTENT_LEN(1)] [CONTENT...]

字段说明：
- CONFIDENCE：置信度，0-100 的整数
- CONTENT_LEN：识别内容长度（字节数）
- CONTENT：识别内容文本（UTF-8 编码）
```

**示例**：用户说"今天天气怎样"
```
帧起始：0xAA
类型：0x01
长度：2 + 9 = 11 (0x0B)
数据：
  - 0x85              // 置信度 133 (实际除以1.33 ≈ 100%)
  - 0x09              // 内容长度 9 字节
  - "今天天气怎样"   // UTF-8 文本
校验和：0x85 XOR 0x09 XOR ... = ...
帧结束：0xCC
```

### 5.5 TTS 播报请求帧格式（0x02）

**ESP32 → ASR**（收到 Chat API 响应后，请求播报）

```
数据字段：
[TEXT...]

字段说明：
- TEXT：要播报的文本（UTF-8 编码），**最多 30 个字符**

限制：
- 字符数不超过 30（控制播报时长）
- 应为中文或英文
```

**示例**：播报"今天多云转阴，最高气温28度"
```
帧起始：0xAA
类型：0x02
长度：30
数据：
  - "今天多云转阴，最高气温28度" (截断至 30 字)
校验和：...
帧结束：0xCC
```

### 5.6 控制命令帧格式（0x03）

**ESP32 → ASR**（发送控制指令）

```
数据字段：
[COMMAND(1)] [PARAMETER(1)]

命令定义：
- 0x01：RESET（重置模块）
- 0x02：START_LISTEN（开始监听）
- 0x03：STOP_LISTEN（停止监听）
- 0x04：GET_VERSION（获取版本）
```

**示例**：开始监听
```
帧起始：0xAA
类型：0x03
长度：2
数据：
  - 0x02          // START_LISTEN 命令
  - 0x00          // 无参数
校验和：0x02 XOR 0x00 = 0x02
帧结束：0xCC
```

### 5.7 校验和计算

对数据部分的所有字节进行 XOR 异或运算：

```cpp
checksum = data[0] ^ data[1] ^ ... ^ data[n-1]
```

### 5.8 通信流程

```
1. ESP32 → ASR: 发送 COMMAND (START_LISTEN)
   └─ ASR 开始监听用户语音

2. 用户说话...

3. ASR → ESP32: 发送 RECOGNITION (识别结果)
   └─ ESP32 接收并解析识别内容

4. ESP32 → Chat API: 发送识别文本
   └─ 调用云端聊天接口

5. Chat API → ESP32: 返回回复文本
   └─ 获得回复（通常 30 字以内）

6. ESP32 → ASR: 发送 TTS (回复文本)
   └─ ASR 通过喇叭播报

7. ASR 自带喇叭: 播放回复
   └─ 用户听到结果

8. 返回步骤 1（继续监听下一条）
```

### 5.9 错误处理

- **识别失败**：ASR 发送 ERROR 帧，ESP32 播报"没有听清"
- **网络超时**：Chat API 超过 30 秒无响应，播报"网络异常"
- **ASR 无响应**：5 秒内 ASR 无通信，判定为断线
- **TTS 长度超限**：自动截断至 30 字，发出日志警告

### 5.10 异常恢复

- 接收方：校验和错误 → 丢弃此帧，继续接收
- 发送方：发送失败 → 重试 1 次，失败则上报错误
- 长时间无通信（5 秒）→ 触发断线警告，可尝试重新初始化

---

## 7. 播报策略（阶段 1）

- 所有 TTS 播报通过 ASR 模块自带喇叭
- **播报短文本（20~30字）**
- 主要内容：
  - 播报接口返回的文字内容
  - 如果发生错误：统一播报：发生了点小错误 / 网络异常

---

## 8. 软件技术选型

- **BLE**：NimBLE-Arduino
- **串口通信**：HardwareSerial（使用 UART1）
- **WiFi/HTTPS**：WiFiClientSecure
- **JSON**：ArduinoJson 7.x
- **流式 TTS（未来升级）**：ESP32 I2S + MAX98357（可选升级方案）

## 8.1 代码模块结构

### asr_manager（ASR 管理器）
负责与 ASR 模块的底层串口通信

```cpp
ASRManager asr;
asr.init();                              // 初始化 UART1
asr.sendCommand(ASR_CMD_START_LISTEN);  // 发送控制命令
if (asr.handleASR()) {                  // 处理接收数据
    ASRRecognitionResult result = asr.getLastResult();
    String text = result.content;       // 获取识别文本
}
asr.sendTTS("今天天气不错");            // 发送 TTS 播报
```

**主要功能**：
- 底层帧的打包和解包
- 校验和计算和验证
- 识别结果解析
- TTS 请求发送
- 控制命令发送
- 连接状态检测（5秒超时判断）

### chat_api_client（Chat API 客户端）
负责与云端聊天接口的通信

```cpp
ChatApiClient client;
client.setToken(token);                  // 设置授权 token
client.setUser(userId);                  // 设置用户 ID
ChatApiResponse response = client.sendMessage("用户输入");
if (response.success) {
    String reply = response.content;     // 获取 AI 回复
}
```

**主要功能**：
- HTTP 请求构建
- JSON 序列化
- 会话 ID 生成
- 响应解析
- 超时管理（30秒）

### asr_chat_integration（ASR-Chat 集成）
负责整个流程的状态管理和协调

```cpp
ASRChatIntegration integration;
integration.init();                      // 初始化
integration.startListening();            // 启动监听

void loop() {
    integration.handle();                // 自动处理状态转换
    
    if (integration.hasCompletedResult()) {
        IntegrationResult result = integration.getLastResult();
        Serial.printf("识别: %s\n", result.recognizedText);
        Serial.printf("回复: %s\n", result.chatResponse);
        integration.clearResultFlag();
    }
}
```

**主要功能**：
- 状态机管理（6 个状态）
- 流程协调
- 超时控制
- 错误处理
- Web 搜索启用开关

### 状态机设计

```
IDLE（空闲）
  ↓ [startListening()]
WAITING_FOR_ASR（等待ASR识别）
  ↓ [收到识别结果]
SENDING_TO_CHAT（发送到Chat）
  ↓ [立即调用]
WAITING_FOR_CHAT（等待Chat响应）
  ↓ [收到响应]
SENDING_TTS（发送TTS播报）
  ↓ [完成]
IDLE（空闲，设置 resultAvailable=true）

ERROR（错误状态）
  ↓ [播报错误提示]
IDLE（空闲，设置 resultAvailable=true）
```

**状态超时**：60 秒无进展则自动转入 ERROR 状态

---

## 9. 开发注意事项

1. **BLE 配置**：BLE 仅用于配置，不参与语音通信
2. **ASR 识别**：ASR 只负责识别和播报，不做语义理解
3. **内存控制**：
   - Chat 响应截断至 30 字（TTS 播报限制）
   - ASR 接收缓冲区 512 字节
   - JSON 文档使用 ArduinoJson 动态分配
4. **超时管理**：
   - Chat API：30 秒超时
   - ASR 通信：5 秒无数据判定断线
   - 状态转换：60 秒超时自动进入错误状态
5. **错误处理**：
   - 校验和失败 → 丢弃此帧继续接收
   - 识别失败 → 播报"没有听清"
   - 网络错误 → 播报"网络异常"
6. **文本限制**：
   - TTS 播报文本最多 30 个字符
   - ASR 识别内容最多 256 字节
   - Chat 输入保持简短，避免内存溢出

## 10. 集成使用示例

```cpp
#include "asr_chat_integration.h"

ASRChatIntegration integration;

void setup() {
    Serial.begin(115200);
    
    // 初始化集成模块
    integration.init("192.168.0.103", 10089);
    
    // 设置认证信息
    integration.setChatToken("your_token_here");
    integration.setChatUser("your_user_id");
    
    // 启用网络搜索
    integration.setWebSearchEnabled(true);
    
    // 启动监听
    integration.startListening();
}

void loop() {
    // 关键：处理所有状态转换
    integration.handle();
    
    // 检查是否有完成的结果
    if (integration.hasCompletedResult()) {
        IntegrationResult result = integration.getLastResult();
        
        if (result.success) {
            Serial.printf("✓ 用户说: %s\n", result.recognizedText.c_str());
            Serial.printf("→ AI 回复: %s\n", result.chatResponse.c_str());
            Serial.printf("⏱ 耗时: %d ms\n", result.totalTime);
        } else {
            Serial.printf("✗ 处理失败: %s\n", result.error.c_str());
        }
        
        // 清除结果标志，准备下一次
        integration.clearResultFlag();
        
        // 重新启动监听（可选）
        // integration.startListening();
    }
    
    delay(10);
}
```

## 11. 后续升级方向

### 11.1 第二阶段：ESP32 独立 TTS 播放

当前方案（阶段 1）中，所有 TTS 播报由 ASR 模块完成，受限于：
- 文本长度最多 30 字
- 播报延迟较大
- 无法控制音量和语速

升级方案：
- 在 ESP32 添加 **MAX98357 音频放大器** + **扬声器**
- 使用 **I2S 接口** 与 MAX98357 通信
- 从 Chat API 获取 TTS 音频流（MP3/WAV）
- 通过 I2S 直接播放，不再依赖 ASR 的喇叭

### 11.2 多轮对话支持

当前方案：单轮对话（识别 → API 调用 → 播报）

升级方案：
- 在 ESP32 端维护 **会话历史** (最多 10 轮)
- 每次调用 Chat API 时传递完整的消息历史
- 实现 **上下文感知** 的多轮对话
- 使用 NVS 存储会话数据

### 11.3 OTA 远程升级

- 通过 BLE 或 WiFi 推送新固件
- 支持 **渐进式回滚**（失败自动恢复）
- 固件签名验证保证安全

### 11.4 BLE 设备管理增强

- 支持多设备配对（保存 Token 映射）
- 远程开启/关闭监听
- BLE 上报 ASR 状态和错误信息

---

## 12. 调试技巧

### 串口监控
使用 PlatformIO 的 Monitor 功能观察日志：
- `[ASR]` 标签：ASR 模块相关日志
- `[ChatAPI]` 标签：Chat API 相关日志
- `[Integration]` 标签：集成模块状态转换日志

### 常见问题排查

| 问题 | 原因 | 解决方案 |
|------|------|---------|
| ASR 无响应 | UART 未正确初始化或引脚错误 | 检查 GPIO16/17 接线 |
| 识别结果乱码 | 文本编码问题或校验和错误 | 确保使用 UTF-8，检查校验和计算 |
| Chat API 调用失败 | Token 过期或网络无连接 | 通过 BLE 更新 Token，检查 WiFi |
| TTS 播报截断 | 响应文本超过 30 字 | 正常行为，自动截断 |
| 频繁超时 | ASR 处理缓慢或 ESP32 内存不足 | 减少并发操作，增加延时 |
