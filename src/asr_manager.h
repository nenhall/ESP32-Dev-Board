/*
 * ASR 模块管理器
 *
 * 功能: 与 ASR 模块进行串口通信，处理语音识别结果和 TTS 播报
 * 创建日期: 2026-01-12
 */

#ifndef ASR_MANAGER_H
#define ASR_MANAGER_H

#include <Arduino.h>

// ==================== ASR 串口配置 ====================
#define ASR_UART_NUM 1           // 使用 UART1
#define ASR_UART_BAUDRATE 115200 // 波特率
#define ASR_RX_PIN 16            // RX 引脚 (ESP32 GPIO16)
#define ASR_TX_PIN 17            // TX 引脚 (ESP32 GPIO17)
#define ASR_BUFFER_SIZE 512      // 缓冲区大小
#define ASR_READ_TIMEOUT 100     // 读取超时 (毫秒)
#define ASR_TTS_MAX_LENGTH 30    // TTS 文本最大长度 (字符数)

// ==================== ASR 协议定义 ====================
// 帧头和帧尾
#define ASR_FRAME_START 0xAA
#define ASR_FRAME_END 0xCC

// 数据类型定义
enum ASRDataType {
  ASR_DT_RECOGNITION = 0x01, // 识别结果 (ASR -> ESP32)
  ASR_DT_TTS = 0x02,         // TTS 播报 (ESP32 -> ASR)
  ASR_DT_COMMAND = 0x03,     // 控制命令 (ESP32 -> ASR)
  ASR_DT_ACK = 0x04,         // 确认帧 (双向)
  ASR_DT_ERROR = 0x05        // 错误信息 (双向)
};

// 控制命令类型
enum ASRCommand {
  ASR_CMD_RESET = 0x01,        // 重置 ASR 模块
  ASR_CMD_START_LISTEN = 0x02, // 开始监听
  ASR_CMD_STOP_LISTEN = 0x03,  // 停止监听
  ASR_CMD_GET_VERSION = 0x04   // 获取版本信息
};

/**
 * @brief ASR 消息帧结构
 * 帧格式: [START(0xAA)] [TYPE] [LENGTH:2] [DATA] [CHECKSUM] [END(0xCC)]
 */
struct ASRFrame {
  uint8_t dataType;                  // 数据类型
  uint16_t length;                   // 数据长度
  uint8_t data[ASR_BUFFER_SIZE - 8]; // 数据内容
  uint8_t checksum;                  // 校验和

  ASRFrame() : dataType(0), length(0), checksum(0) {}
};

/**
 * @brief ASR 识别结果
 */
struct ASRRecognitionResult {
  bool success;   // 识别是否成功
  String content; // 识别内容
  int confidence; // 置信度 (0-100)
  String error;   // 错误信息

  ASRRecognitionResult() : success(false), confidence(0) {}
};

/**
 * @brief ASR 模块管理器类
 */
class ASRManager {
public:
  ASRManager();

  /**
   * @brief 初始化 ASR 串口通信
   */
  void init();

  /**
   * @brief 处理 ASR 通信（检查是否有数据，处理接收到的帧）
   * @return true 如果接收到新的识别结果
   */
  bool handleASR();

  /**
   * @brief 发送 TTS 播报请求给 ASR 模块
   * @param text 要播报的文本（不超过 30 个字符）
   * @return true 发送成功
   */
  bool sendTTS(const String &text);

  /**
   * @brief 发送控制命令给 ASR 模块
   * @param command 命令类型
   * @param parameter 命令参数（可选）
   * @return true 命令发送成功
   */
  bool sendCommand(ASRCommand command, uint8_t parameter = 0);

  /**
   * @brief 获取最后一次识别结果
   */
  ASRRecognitionResult getLastResult() const;

  /**
   * @brief 检查是否有新的识别结果
   */
  bool hasNewResult() const;

  /**
   * @brief 清除新结果标志
   */
  void clearNewResultFlag();

  /**
   * @brief 获取 ASR 模块连接状态
   */
  bool isConnected() const;

private:
  /**
   * @brief 计算校验和 (异或校验)
   */
  uint8_t calculateChecksum(const uint8_t *data, uint16_t length);

  /**
   * @brief 发送帧
   * @param frame 要发送的帧
   */
  bool sendFrame(const ASRFrame &frame);

  /**
   * @brief 接收帧
   * @param frame 接收到的帧（输出参数）
   * @return true 成功接收到有效的帧
   */
  bool receiveFrame(ASRFrame &frame);

  /**
   * @brief 处理识别结果帧
   * @param frame 包含识别结果的帧
   */
  void handleRecognitionFrame(const ASRFrame &frame);

  // 成员变量
  ASRRecognitionResult lastResult;                         // 最后一次识别结果
  bool newResultAvailable;                                 // 是否有新的识别结果
  unsigned long lastCommunicationTime;                     // 最后通信时间
  static const unsigned long COMMUNICATION_TIMEOUT = 5000; // 5秒超时
};

#endif // ASR_MANAGER_H
