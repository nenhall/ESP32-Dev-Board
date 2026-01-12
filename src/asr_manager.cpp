/*
 * ASR 模块管理器实现
 *
 * 功能: 与 ASR 模块进行串口通信，处理语音识别结果和 TTS 播报
 * 创建日期: 2026-01-12
 */

#include "asr_manager.h"
#include "config.h"

// 全局 ASR 管理器实例
static ASRManager asrManager;

ASRManager::ASRManager() : newResultAvailable(false), lastCommunicationTime(0) {}

void ASRManager::init() {
  Serial.println("[ASR] 初始化 ASR 串口通信...");

  // 配置 UART1
  Serial.printf("[ASR] 配置 UART%d, 波特率: %d\n", ASR_UART_NUM, ASR_UART_BAUDRATE);

  // 使用 HardwareSerial 初始化 UART1
  Serial1.begin(ASR_UART_BAUDRATE, SERIAL_8N1, ASR_RX_PIN, ASR_TX_PIN);

  Serial.println("[ASR] ASR 串口初始化完成");

  // 初始化完成后，播报欢迎语音
  delay(500); // 等待 UART 完全就绪
  Serial.println("[ASR] 发送欢迎语音...");
  sendTTS("欢迎使用奇境探索");
}

bool ASRManager::handleASR() {
  // 检查是否有数据可读
  if (Serial1.available() > 0) {
    ASRFrame frame;
    if (receiveFrame(frame)) {
      lastCommunicationTime = millis();

      switch (frame.dataType) {
      case ASR_DT_RECOGNITION:
        handleRecognitionFrame(frame);
        return true;

      case ASR_DT_ACK:
        Serial.printf("[ASR] 收到 ACK 确认 (长度: %d)\n", frame.length);
        break;

      case ASR_DT_ERROR: {
        String errorMsg((const char *)frame.data, frame.length);
        Serial.printf("[ASR] 收到错误: %s\n", errorMsg.c_str());
      } break;

      default:
        Serial.printf("[ASR] 未知数据类型: 0x%02X\n", frame.dataType);
        break;
      }
    }
  }

  return false;
}

bool ASRManager::sendTTS(const String &text) {
  // 文本长度检查
  if (text.length() == 0) {
    Serial.println("[ASR] 错误: TTS 文本为空");
    return false;
  }

  if (text.length() > ASR_TTS_MAX_LENGTH) {
    Serial.printf("[ASR] 警告: TTS 文本过长 (%d > %d), 将被截断\n", text.length(), ASR_TTS_MAX_LENGTH);
  }

  // 构造 TTS 帧
  ASRFrame frame;
  frame.dataType = ASR_DT_TTS;
  frame.length = text.length();

  // 复制文本数据（最多 ASR_TTS_MAX_LENGTH 个字符）
  uint16_t copyLen = (text.length() > ASR_TTS_MAX_LENGTH) ? ASR_TTS_MAX_LENGTH : text.length();
  memcpy(frame.data, text.c_str(), copyLen);
  frame.length = copyLen;

  // 计算校验和
  frame.checksum = calculateChecksum(frame.data, frame.length);

  Serial.printf("[ASR] 发送 TTS: \"%s\" (长度: %d)\n", text.c_str(), frame.length);

  return sendFrame(frame);
}

bool ASRManager::sendCommand(ASRCommand command, uint8_t parameter) {
  ASRFrame frame;
  frame.dataType = ASR_DT_COMMAND;
  frame.data[0] = command;
  frame.data[1] = parameter;
  frame.length = 2;
  frame.checksum = calculateChecksum(frame.data, frame.length);

  Serial.printf("[ASR] 发送命令: 0x%02X (参数: 0x%02X)\n", command, parameter);

  return sendFrame(frame);
}

ASRRecognitionResult ASRManager::getLastResult() const { return lastResult; }

bool ASRManager::hasNewResult() const { return newResultAvailable; }

void ASRManager::clearNewResultFlag() { newResultAvailable = false; }

bool ASRManager::isConnected() const {
  unsigned long now = millis();
  // 如果最后通信时间在超时范围内，则认为连接良好
  return (now - lastCommunicationTime) < COMMUNICATION_TIMEOUT;
}

uint8_t ASRManager::calculateChecksum(const uint8_t *data, uint16_t length) {
  uint8_t checksum = 0;
  for (uint16_t i = 0; i < length; i++) {
    checksum ^= data[i]; // 异或校验
  }
  return checksum;
}

bool ASRManager::sendFrame(const ASRFrame &frame) {
  if (Serial1.write(ASR_FRAME_START) == 0) {
    Serial.println("[ASR] 错误: 无法发送帧头");
    return false;
  }

  if (Serial1.write(frame.dataType) == 0) {
    Serial.println("[ASR] 错误: 无法发送数据类型");
    return false;
  }

  // 发送长度 (小端字节序)
  if (Serial1.write(frame.length & 0xFF) == 0 || Serial1.write((frame.length >> 8) & 0xFF) == 0) {
    Serial.println("[ASR] 错误: 无法发送长度");
    return false;
  }

  // 发送数据
  if (Serial1.write(frame.data, frame.length) != frame.length) {
    Serial.println("[ASR] 错误: 数据发送不完整");
    return false;
  }

  // 发送校验和
  if (Serial1.write(frame.checksum) == 0) {
    Serial.println("[ASR] 错误: 无法发送校验和");
    return false;
  }

  // 发送帧尾
  if (Serial1.write(ASR_FRAME_END) == 0) {
    Serial.println("[ASR] 错误: 无法发送帧尾");
    return false;
  }

  Serial1.flush(); // 等待发送完成
  return true;
}

bool ASRManager::receiveFrame(ASRFrame &frame) {
  // 第一步: 查找帧头 (0xAA)
  uint8_t startByte;
  unsigned long startTime = millis();

  while (Serial1.available() > 0) {
    startByte = Serial1.read();
    if (startByte == ASR_FRAME_START) {
      break;
    }
  }

  if (startByte != ASR_FRAME_START) {
    return false; // 未找到帧头
  }

  // 第二步: 读取数据类型
  if (Serial1.available() < 1) {
    return false;
  }
  frame.dataType = Serial1.read();

  // 第三步: 读取长度 (小端字节序)
  if (Serial1.available() < 2) {
    return false;
  }
  uint8_t lenLow = Serial1.read();
  uint8_t lenHigh = Serial1.read();
  frame.length = lenLow | (lenHigh << 8);

  // 长度检查
  if (frame.length > sizeof(frame.data)) {
    Serial.printf("[ASR] 错误: 数据长度过大 (%d)\n", frame.length);
    return false;
  }

  // 第四步: 读取数据
  if (Serial1.available() < frame.length) {
    return false;
  }
  Serial1.readBytes(frame.data, frame.length);

  // 第五步: 读取校验和
  if (Serial1.available() < 1) {
    return false;
  }
  uint8_t receivedChecksum = Serial1.read();

  // 第六步: 读取帧尾
  if (Serial1.available() < 1) {
    return false;
  }
  uint8_t endByte = Serial1.read();

  if (endByte != ASR_FRAME_END) {
    Serial.println("[ASR] 错误: 帧尾检查失败");
    return false;
  }

  // 第七步: 校验和校验
  uint8_t calculatedChecksum = calculateChecksum(frame.data, frame.length);
  if (receivedChecksum != calculatedChecksum) {
    Serial.printf("[ASR] 错误: 校验和不匹配 (期望: 0x%02X, 接收: 0x%02X)\n", calculatedChecksum, receivedChecksum);
    return false;
  }

  frame.checksum = receivedChecksum;
  return true;
}

void ASRManager::handleRecognitionFrame(const ASRFrame &frame) {
  // 解析识别结果
  // 帧数据格式: [CONFIDENCE(1)] [CONTENT_LENGTH(1)] [CONTENT...]

  if (frame.length < 2) {
    Serial.println("[ASR] 错误: 识别结果帧数据过短");
    lastResult.success = false;
    lastResult.error = "Frame too short";
    return;
  }

  uint8_t confidence = frame.data[0];
  uint8_t contentLength = frame.data[1];

  // 检查内容长度
  if (contentLength == 0 || contentLength > frame.length - 2) {
    Serial.println("[ASR] 错误: 识别内容长度无效");
    lastResult.success = false;
    lastResult.error = "Invalid content length";
    return;
  }

  // 提取识别内容
  lastResult.success = true;
  lastResult.content = String((const char *)(frame.data + 2), contentLength);
  lastResult.confidence = confidence;
  lastResult.error = "";
  newResultAvailable = true;

  Serial.printf("[ASR] 识别成功: \"%s\" (置信度: %d%%)\n", lastResult.content.c_str(), lastResult.confidence);
}
