/*
 * 配置文件更新建议
 * 
 * 本文件列出了为了支持 ASR 模块需要对现有配置文件的建议修改
 * 创建日期: 2026-01-12
 */

// ==================== include/config.h 建议更新 ====================
// 添加以下内容到 config.h：

/*

// ==================== ASR 串口配置 ====================
// UART1 用于与 ASR 模块通信
#define ASR_UART_NUM UART_NUM_1        // 使用 UART1
#define ASR_UART_BAUDRATE 115200       // 波特率
#define ASR_RX_PIN 16                  // RX 引脚 (ESP32 GPIO16)
#define ASR_TX_PIN 17                  // TX 引脚 (ESP32 GPIO17)

// ==================== Chat API 配置 ====================
// API 服务器地址和端口
#define CHAT_API_HOST "192.168.0.103"
#define CHAT_API_PORT 10089

// ==================== ASR-Chat 超时配置 ====================
#define ASR_TTS_MAX_LENGTH 30          // TTS 文本最大长度 (字符数)
#define CHAT_API_TIMEOUT 30000         // Chat API 请求超时 (毫秒)
#define ASR_STATE_TIMEOUT 60000        // 状态机超时 (毫秒)
#define ASR_COMMUNICATION_TIMEOUT 5000 // ASR 通信超时 (毫秒)

*/

// ==================== platformio.ini 建议更新 ====================
/*
[env:esp32dev]
...existing config...

; 增加以下库依赖以支持 ASR 和 Chat 功能
lib_deps =
    ...existing libs...
    ArduinoJson @ ^7.0
    ESP32-WiFi
    NimBLE-Arduino

; 增加以下编译选项
build_flags =
    ...existing flags...
    -D ARDUINO_LOOP_STACK_SIZE=8192
    -D CONFIG_ARDUHAL_LOG_COLORS=1

*/

// ==================== src/main.cpp 集成步骤 ====================
/*

1. 在 #include 部分添加：

#include "asr_chat_integration.h"
#include "asr_integration_example.h"

2. 在全局变量区添加：

// ASR-Chat 集成模块实例（可选，如果直接使用示例函数）
ASRChatIntegration g_asrIntegration;

3. 在 setup() 函数末尾添加：

void setup() {
    // ... 现有代码 ...
    
    // 初始化 ASR-Chat 集成示例
    initASRIntegrationExample(
        CHAT_API_HOST,
        CHAT_API_PORT,
        nullptr  // 可选的结果回调函数
    );
    
    Serial.println("\n[Main] 所有模块初始化完成");
}

4. 在 loop() 函数添加：

void loop() {
    // ... 现有代码 ...
    
    // 处理 ASR-Chat 集成
    handleASRIntegrationLoop();
    
    // ... 其他代码 ...
    
    delay(10);  // 必要的延时，避免 WDT 复位
}

5. 修改 handleBluetoothCommands() 函数，添加 ASR 命令处理：

// 在现有的蓝牙命令处理中添加：
if (command == "listen" || command == "start") {
    handleASRBLECommand("listen");
} 
else if (command == "stop") {
    handleASRBLECommand("stop");
}
else if (command == "status") {
    handleASRBLECommand("status");
}
else if (command.startsWith("test ")) {
    handleASRBLECommand(command);
}

*/

// ==================== src/bt_manager.cpp 建议修改 ====================
/*

在 handleBluetoothCommands() 函数中添加以下 case 分支：

// 添加到蓝牙命令处理中：
void handleBluetoothCommands() {
    // ... 现有命令处理 ...
    
    // ASR 相关命令
    if (data == "listen" || data == "start") {
        handleASRBLECommand("listen");
        sendBLENotification("ASR listening started");
    }
    else if (data == "stop") {
        handleASRBLECommand("stop");
        sendBLENotification("ASR listening stopped");
    }
    else if (data == "status") {
        handleASRBLECommand("status");
    }
    else if (data.startsWith("test ")) {
        String testText = data.substring(5);
        handleASRBLECommand("test " + testText);
    }
}

*/

// ==================== 新建文件列表 ====================
/*

需要添加的新文件（已创建）：

1. src/asr_manager.h               - ASR 管理器头文件
2. src/asr_manager.cpp             - ASR 管理器实现
3. src/asr_chat_integration.h       - 集成模块头文件
4. src/asr_chat_integration.cpp     - 集成模块实现
5. src/asr_integration_example.h    - 使用示例头文件
6. src/asr_integration_example.cpp  - 使用示例实现

文档文件（已创建）：

7. docs/ASR_Integration_Guide.md    - 完整集成指南
8. docs/ASR_Quick_Reference.md      - 快速参考卡
9. docs/esp_32_asr_pipeline.md      - 更新的系统设计文档

*/

// ==================== 编译检查清单 ====================
/*

确保以下内容在编译前完成：

✓ 所有新的 .h 和 .cpp 文件已复制到 src/ 目录
✓ main.cpp 已添加 include 和初始化调用
✓ platformio.ini 已添加必要的库依赖（ArduinoJson 7.x）
✓ config.h 已添加 ASR 和 Chat 配置常量（可选）
✓ 没有编译错误（pio run）
✓ 没有链接错误
✓ bin 文件大小合理（< 1.5 MB）

*/

// ==================== 运行时检查清单 ====================
/*

上传固件后，通过串口监控检查：

✓ 启动时能看到 "[ASR] 初始化 ASR 串口通信" 日志
✓ 启动时能看到 "[Integration] 初始化完成" 日志
✓ 能收到 BLE 连接通知
✓ 发送 "listen" 命令后能看到 "[ASR] 发送命令: 0x02" 日志
✓ 说话后能看到 "[ASR] 识别成功" 日志
✓ 能看到 "[ChatAPI] 发送请求" 日志
✓ 收到 Chat API 响应后能看到 "[Integration] 发送 TTS" 日志

*/

// ==================== 故障排除 ====================
/*

如果编译失败：
1. 检查是否安装了 ArduinoJson 库（pio lib install ArduinoJson）
2. 检查文件路径是否正确（所有 .h 和 .cpp 在 src/ 目录）
3. 尝试 pio run --target clean 清理构建

如果上传失败：
1. 检查 COM 端口是否正确
2. 检查是否有其他程序占用串口
3. 尝试按下 ESP32 的 Boot 按钮

如果 ASR 无响应：
1. 检查硬件连接（GPIO16/17）
2. 检查波特率（115200）
3. 确保 ASR 模块已上电和工作
4. 查看 "[ASR]" 日志是否有错误信息

*/
