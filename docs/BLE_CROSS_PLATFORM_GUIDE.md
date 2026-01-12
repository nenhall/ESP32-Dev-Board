# BLE 跨平台蓝牙测试指南

## 概述

本项目已升级为支持 **BLE (Bluetooth Low Energy)** 跨平台蓝牙方案，采用 **Nordic UART Service (NUS)** 规范实现，可在 iOS、Android、macOS、Windows 上使用。

## 重要说明

### iOS 原生蓝牙设置

⚠️ **在 iOS 系统设置（Settings > Bluetooth）中看不到该设备是正常的**

原因：iOS 系统蓝牙设置只显示以下设备类型：
- 经典蓝牙（SPP）设备
- 已配对的 BLE 设备  
- 预定义 BLE Profile 设备（如耳机、心率计等）

**通用 BLE GATT 服务器（如 NUS）不会在系统设置中显示。**

✅ **请改用专门的 BLE 扫描应用（如 nRF Connect 或 LightBlue）连接。**

---

## 连接测试步骤

### iOS / macOS

1. **从 App Store 下载 BLE 扫描应用**：
   - **nRF Connect for iOS** (推荐) [官方链接](https://apps.apple.com/us/app/nrf-connect-for-mobile/id1054362403)
   - **LightBlue** [官方链接](https://apps.apple.com/us/app/lightblue/id557673172)

2. **打开应用，点击"Scan"（扫描）**

3. **在列表中找到设备名 `Questech_BT`**（或你配置的名称）

4. **点击连接**

5. **连接后，在服务列表中找到 NUS 服务**：
   - **Service UUID**: `6E400001-B5A3-F393-E0A9-E50E24DCCA9E`
   - **TX Characteristic** (notify): `6E400003-B5A3-F393-E0A9-E50E24DCCA9E`
   - **RX Characteristic** (write): `6E400002-B5A3-F393-E0A9-E50E24DCCA9E`

6. **向 RX 特性写入数据进行测试**（例如 `help` 命令）

---

### Android

1. **从 Google Play 下载 BLE 扫描应用**：
   - **nRF Connect for Android** (推荐) [官方链接](https://play.google.com/store/apps/details?id=no.nordicsemi.android.mcp)
   - **BLE Scanner** [官方链接](https://play.google.com/store/apps/details?id=com.bluepixelvale.blescan)

2. **打开应用，点击"Scan"**

3. **找到 `Questech_BT` 设备并连接**

4. **浏览 NUS 服务并与特性交互**

---

### Windows 10 / 11

**方式 1：使用 nRF Connect for Windows**

1. 下载 [nRF Connect for Desktop](https://www.nordicsemi.com/Products/Development-tools/nRF-Connect-for-Desktop)

2. 打开 BLE Scanner，扫描找到 `Questech_BT`

3. 连接并测试 NUS 服务

**方式 2：使用原生 Windows 蓝牙**

在 Windows 设置中：
- Settings > Devices > Bluetooth & devices > Add device
- 选择 "Bluetooth"
- 找到 `Questech_BT` 并配对（注：可能需要配对后在 BLE 应用中才能完全交互）

---

## 命令测试

连接成功后，可向设备发送以下命令（通过 RX 特性）：

| 命令 | 功能 |
|-----|------|
| `help` | 显示可用命令列表 |
| `status` | 显示系统状态（运行时间、内存等） |
| `wifi` | 显示 WiFi 连接信息 |
| `ip` | 显示 IP 地址 |
| `ram` | 显示内存信息 |
| `restart` | 重启设备 |

### 测试示例

在 LightBlue 或 nRF Connect 中：

1. 连接到 `Questech_BT`
2. 找到 RX 特性（UUID `6E400002-...`）
3. 向其写入 `help\n`（包括换行符）
4. 查看 TX 特性（UUID `6E400003-...`），应该能收到命令列表响应

---

## 硬件连接要求

- 距离：最多 10 米（BLE 范围），建议 ≤ 2 米
- 环境：避免强 WiFi / 2.4GHz 干扰
- 设备：支持 BLE 5.0 的手机或电脑

---

## 故障排除

### 问题：扫描时找不到设备

**解决方案**：
1. 确认 ESP32 已上电并运行（串口监视器应显示 "BLE 已启动"）
2. 重启手机蓝牙
3. 靠近设备（≤ 1 米）
4. 检查串口监视器输出，确认 BLE 初始化成功

### 问题：连接后无法读写数据

**解决方案**：
1. 确认已连接到正确的 NUS 服务
2. 向 TX 特性启用 "Notify" 或 "Indicate"（订阅通知）
3. 尝试向 RX 特性写入简单数据（如 `hello\n`）

### 问题：iOS 系统设置看不到设备

✅ **这是正常的**，请改用 nRF Connect 或 LightBlue。

---

## 技术细节

### 使用的库

- **BLEDevice / BLEServer / BLECharacteristic**（ESP32 Arduino 框架内置）
- 无外部依赖

### BLE 配置

- **模式**：Peripheral（外围设备）
- **服务**：Nordic UART Service (NUS)
- **特性**：
  - **TX (6E400003-...)**: PROPERTY_NOTIFY（设备→手机）
  - **RX (6E400002-...)**: PROPERTY_WRITE（手机→设备）
- **广告参数**：
  - 设备名称：`Questech_BT`（可在 `config.h` 中修改）
  - 广告包含 NUS Service UUID

### 代码架构

- **`src/bt_ble.h`**: BLE 接口声明
- **`src/bt_ble.cpp`**: BLE 实现（NUS 服务、特性、回调）
- **`src/bt_manager.h/cpp`**: 高层蓝牙管理接口（与 BLE 集成）
- **`src/main.cpp`**: 初始化 `initBluetoothBLE()` 并在 `loop()` 中调用 `handleBLE()`

---

## 未来改进

1. 支持 BLE 安全配对（MITM）
2. 增强 MTU 大小（目前 20 字节/包）
3. 支持批量数据传输
4. 对接 iOS HomeKit 或其他苹果生态

---

**祝测试愉快！** 🎉
