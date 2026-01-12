# ESP32-WROOM 开发板项目

## 开发板信息

**开发板型号**: ESP32-WROOM-32 Dev Module

**标识**: esp-32开发板 wroom wifi+蓝牙模块 CH9102 (实际芯片: CH340)

**PlatformIO 配置**: `board = esp32dev`

**检测到的端口**: COM3

### 硬件规格
- **模块**: ESP32-WROOM-32
- **CPU**: Xtensa 双核 32-bit LX6 微处理器，240MHz
- **内存**: 520KB SRAM, 4MB Flash
- **USB转串口**: CH340 芯片 (系统识别)
- **无线**: Wi-Fi 802.11 b/g/n, 蓝牙 BLE 4.2 + 经典蓝牙
- **GPIO**: 36 个可编程 GPIO
- **ADC**: 12 位 SAR ADC，最多 18 个通道

---

## ESP32-WROOM Module

> ESP32-WROOM 开发板 (CH340 驱动芯片) 开发指南和示例代码

---

## 📋 项目简介

本项目提供 ESP32-WROOM-32 开发板的完整开发指南和示例代码，包含从环境搭建到实际项目的完整教程。

---

## 📁 项目结构

```
ESP32-Dev-Board/
├── docs/                    # 📚 文档目录
│   ├── 开发指南.md          # 完整开发指南
│   └── 快速开始.md          # 快速入门教程
│
├── examples/                # 💡 示例代码
│   ├── blink/              # LED 闪烁示例
│   ├── wifi/               # Wi-Fi 连接示例
│   └── webserver/          # Web 服务器示例
│
├── config/                  # ⚙️ 配置文件
│   ├── platformio.ini      # PlatformIO 配置
│   ├── sdkconfig.defaults  # ESP-IDF 配置
│   └── config_example.h    # 代码配置模板
│
└── README.md               # 本文件
```

---

## 🚀 快速开始

### 1. 安装驱动
下载并安装 CH340 驱动:
- **Windows**: [下载 CH340 驱动](http://www.wch.cn/download/CH341SER_EXE.html)
- **Mac**: [下载 CH340 驱动](https://macdrivers.com/ch340-mac-driver)
- **Linux**: `sudo apt-get install CH340-driver`

### 2. 选择开发环境

#### 方案 A: Arduino IDE (推荐新手) ⭐
1. 安装 [Arduino IDE](https://www.arduino.cc/en/software)
2. 添加 ESP32 开发板支持 (详情见文档)
3. 打开示例代码开始开发

#### 方案 B: PlatformIO (推荐有经验开发者)
1. 安装 [VSCode](https://code.visualstudio.com/)
2. 安装 PlatformIO 扩展
3. 使用 `config/platformio.ini` 配置

#### 方案 C: ESP-IDF (专业开发)
1. 安装 [ESP-IDF](https://docs.espressif.com/projects/esp-idf/)
2. 使用 `config/sdkconfig.defaults` 配置

### 3. 运行第一个示例

打开 [examples/blink/blink.ino](examples/blink/blink.ino):
```cpp
const int LED_PIN = 2;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  digitalWrite(LED_PIN, HIGH);
  delay(1000);
  digitalWrite(LED_PIN, LOW);
  delay(1000);
}
```

上传到开发板，你将看到 LED 每秒闪烁一次！

---

## 📚 文档

### 开发指南
[📖 完整开发指南](docs/开发指南.md) 包含:
- 硬件准备
- 软件环境搭建 (Arduino IDE, ESP-IDF, PlatformIO)
- 项目创建
- 开发流程
- 调试方法
- 常见问题解决

### 快速开始
[🚀 快速开始指南](docs/快速开始.md) 提供:
- 环境搭建详细步骤
- 三个项目实战:
  - LED 闪烁
  - Wi-Fi 连接
  - Web 服务器
- 调试技巧
- 进阶项目示例

---

## 💡 示例代码

### [LED 闪烁](examples/blink/blink.ino)
最基础的示例，控制板载 LED 闪烁

**功能**:
- GPIO 控制
- 串口输出
- 延时控制

### [Wi-Fi 连接](examples/wifi/wifi_connect.ino)
连接到 Wi-Fi 网络

**功能**:
- Wi-Fi 连接
- 网络信息显示
- 信号强度检测
- 自动重连

### [Web 服务器](examples/webserver/webserver.ino)
通过浏览器控制 LED

**功能**:
- Web 服务器
- HTTP 路由
- JSON API
- 响应式界面

---

## ⚙️ 配置文件

### [platformio.ini](config/platformio.ini)
PlatformIO 项目配置模板

### [sdkconfig.defaults](config/sdkconfig.defaults)
ESP-IDF SDK 默认配置

### [config_example.h](config/config_example.h)
C++ 代码配置模板，包含:
- WiFi 配置
- GPIO 定义
- 传感器配置
- 调试选项

---

## 📖 学习路径

### 初学者
1. 安装驱动和 Arduino IDE
2. 运行 [LED 闪烁示例](examples/blink/)
3. 学习 [快速开始指南](docs/快速开始.md)
4. 尝试 Wi-Fi 连接示例

### 进阶
1. 阅读 [完整开发指南](docs/开发指南.md)
2. 学习 Web 服务器示例
3. 使用 PlatformIO 或 ESP-IDF
4. 集成传感器和模块

### 高级
1. 深入学习 ESP-IDF
2. 开发复杂项目
3. 优化性能和功耗
4. 实现 OTA 升级

---

## 🛠️ 开发工具推荐

### IDE
- **Arduino IDE**: 简单易用，适合初学者
- **VSCode + PlatformIO**: 强大灵活，适合中高级开发
- **Eclipse / CLion**: 专业开发环境

### 调试工具
- **串口监视器**: 查看调试输出
- **ESP Flash Download Tool**: 固件烧录
- **Wireshark**: 网络数据包分析

### 硬件工具
- **逻辑分析仪**: 分析时序
- **万用表**: 测量电压电流
- **示波器**: 信号分析

---

## 🤝 常见问题

### Q: 找不到串口?
**A**: 检查 CH340 驱动是否正确安装，更换 USB 线或端口

### Q: 上传失败?
**A**: 按住 BOOT 按钮再点击上传，降低上传速度

### Q: Wi-Fi 连接失败?
**A**: 确认使用 2.4GHz WiFi (不支持 5GHz)，检查密码

更多问题请查看 [开发指南 - 常见问题](docs/开发指南.md#常见问题)

---

## 📚 参考资料

### 官方文档
- [ESP32 数据手册](https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_cn.pdf)
- [ESP-IDF 编程指南](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/)
- [Arduino ESP32 核心](https://github.com/espressif/arduino-esp32)

### 社区资源
- [乐鑫科技中文社区](https://www.esp32.com/)
- [ESP32 中文论坛](https://www.cnblogs.com/espressif/)
- [GitHub ESP32 项目](https://github.com/espressif/esp32)

---

## 📝 更新日志

### 2026-01-12
- ✨ 初始版本发布
- 📖 完整开发指南
- 🚀 快速开始教程
- 💡 三个示例项目
- ⚙️ 配置文件模板

---

## 📄 许可证

本项目采用 MIT 许可证

---

## 👥 贡献

欢迎提交 Issue 和 Pull Request!

---

**⭐ 如果这个项目对你有帮助，请给个 Star!**
