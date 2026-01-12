/*
 * 蓝牙管理模块实现 (基于 NimBLE-Arduino)
 *
 * 功能: BLE (NUS) 初始化、命令处理、数据传输
 * 修改日期: 2026-01-12
 */

#include "bt_manager.h"
#include "config.h"
#include <NimBLEDevice.h>
#include <NimBLEServer.h>
#include <NimBLEUtils.h>
#include <WiFi.h>
#include <Preferences.h>
#include <stdarg.h>

// NVS 存储
static Preferences preferences;
static const char* NVS_NAMESPACE = "storage";
static const char* NVS_TOKEN_KEY = "api_token";

// Nordic UART Service UUIDs
static const NimBLEUUID serviceUUID("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");
static const NimBLEUUID charRxUUID("6E400002-B5A3-F393-E0A9-E50E24DCCA9E"); // write from client
static const NimBLEUUID charTxUUID("6E400003-B5A3-F393-E0A9-E50E24DCCA9E"); // notify to client

// 全局变量
static NimBLEServer *pServer = nullptr;
static NimBLECharacteristic *pTxCharacteristic = nullptr;
static NimBLECharacteristic *pRxCharacteristic = nullptr;
static bool deviceConnected = false;
static bool advertisingStarted = false;
static String rxBuffer = "";
static bool tokenHasBeenSet = false;  // Token 状态标志（避免频繁读取 NVS）

// ==================== NVS 存储函数 ====================

/**
 * @brief 初始化 NVS 存储
 */
void initStorage() {
  preferences.begin(NVS_NAMESPACE, false);
  // 初始化时检查是否已有 token
  tokenHasBeenSet = preferences.getString(NVS_TOKEN_KEY, "").length() > 0;
  Serial.printf("[Storage] Initialized (token %s)\n", tokenHasBeenSet ? "set" : "not set");
}

/**
 * @brief 保存 token 到 NVS
 */
bool saveToken(const char* token) {
  if (token == nullptr || strlen(token) == 0) {
    Serial.println("[Storage] Token is empty, not saving");
    return false;
  }

  size_t written = preferences.putString(NVS_TOKEN_KEY, token);
  if (written > 0) {
    tokenHasBeenSet = true;  // 更新状态标志
    Serial.printf("[Storage] Token saved (%u bytes)\n", written);
    return true;
  } else {
    Serial.println("[Storage] Failed to save token");
    return false;
  }
}

/**
 * @brief 从 NVS 读取 token（静默版本，用于状态检查）
 */
static String loadTokenSilent() {
  return preferences.getString(NVS_TOKEN_KEY, "");
}

/**
 * @brief 从 NVS 读取 token（带日志输出，用于显式加载）
 */
String loadToken() {
  String token = loadTokenSilent();
  if (token.length() > 0) {
    Serial.printf("[Storage] Token loaded (%u bytes)\n", token.length());
  } else {
    Serial.println("[Storage] No token found");
  }
  return token;
}

/**
 * @brief 检查是否已设置 token
 */
bool isTokenSet() {
  return tokenHasBeenSet;
}

/**
 * @brief 清除已保存的 token
 */
bool clearToken() {
  bool success = preferences.remove(NVS_TOKEN_KEY);
  if (success) {
    tokenHasBeenSet = false;  // 更新状态标志
    Serial.println("[Storage] Token cleared");
  } else {
    Serial.println("[Storage] Failed to clear token");
  }
  return success;
}

// ==================== BLE 相关函数 ====================

// 服务器回调类
class ServerCallbacks : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer *pServer) {
    deviceConnected = true;
    advertisingStarted = false;  // 连接成功后，标记广播已停止
    Serial.println("[BLE] Device connected");
    blePrintln("questech connected");
  }

  void onDisconnect(NimBLEServer *pServer) {
    deviceConnected = false;
    Serial.println("[BLE] Device disconnected, restarting advertising");
    // 断开后自动重启广告
    NimBLEDevice::getAdvertising()->start();
    advertisingStarted = true;
  }
};

// 接收回调类
class RxCallbacks : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic *pChar) {
    std::string val = pChar->getValue();
    if (!val.empty()) {
      rxBuffer += String(val.c_str());
      if (rxBuffer.length() > 0 && rxBuffer.endsWith("\n") == false) {
        rxBuffer += '\n';
      }
    }
  }
};

/**
 * @brief 初始化 BLE (Nordic UART Service)
 */
void initBluetoothBLE(const char *deviceName) {
  if (deviceName == nullptr) {
    deviceName = BLUETOOTH_DEVICE_NAME;
  }

  Serial.println("[BLE] Initializing...");

  // 初始化 NimBLE 设备
  NimBLEDevice::init(deviceName);

  // 设置设备名称
  NimBLEDevice::setPower(ESP_PWR_LVL_P9); // 最大功率

  // 创建服务器
  pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  // 创建 Nordic UART Service
  NimBLEService *pService = pServer->createService(serviceUUID);

  // 创建 TX 特征（notify）
  pTxCharacteristic = pService->createCharacteristic(
    charTxUUID,
    NIMBLE_PROPERTY::NOTIFY
  );

  // 创建 RX 特征（write）
  pRxCharacteristic = pService->createCharacteristic(
    charRxUUID,
    NIMBLE_PROPERTY::WRITE |
    NIMBLE_PROPERTY::WRITE_NR
  );
  pRxCharacteristic->setCallbacks(new RxCallbacks());

  // 启动服务
  pService->start();

  // 配置广告
  NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(serviceUUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // 最低间隔
  pAdvertising->setMaxPreferred(0x12);  // 最高间隔

  // 启动广告
  pAdvertising->start();
  advertisingStarted = true;

  Serial.printf("[BLE] Started, device name: %s\n", deviceName);
  Serial.println("[BLE] Advertising...");
  Serial.printf("[BLE] Service UUID: %s (NUS)\n", serviceUUID.toString().c_str());
}

/**
 * @brief 检查是否有数据可读
 */
bool bleAvailable() {
  return rxBuffer.length() > 0;
}

/**
 * @brief 读取一条字符串
 */
String bleReadString() {
  String s = rxBuffer;
  rxBuffer = "";
  s.trim();
  return s;
}

/**
 * @brief 发送字符串（带换行）
 */
void blePrintln(const char *msg) {
  if (pTxCharacteristic == nullptr || !deviceConnected) {
    return;
  }

  String t = String(msg);
  t += "\n";
  pTxCharacteristic->setValue(t.c_str());
  pTxCharacteristic->notify();
}

/**
 * @brief printf 风格发送
 */
void blePrintf(const char *format, ...) {
  char buffer[256];
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);
  blePrintln(buffer);
}

/**
 * @brief 检查 BLE 是否已连接
 */
bool isBLEConnected() {
  return deviceConnected;
}

/**
 * @brief 处理 BLE 维护任务
 */
void handleBLE() {
  // 确保广告保持启动状态
  if (!advertisingStarted && !deviceConnected) {
    NimBLEDevice::getAdvertising()->start();
    advertisingStarted = true;
    Serial.println("[BLE] Advertising restarted");
  }
}

/**
 * @brief 处理蓝牙命令
 */
void handleBluetoothCommands() {
  if (bleAvailable()) {
    String cmd = bleReadString();
    cmd.trim();

    // 检查是否是设置 token 命令（格式：token <your_token>）
    if (cmd.startsWith("token ")) {
      String token = cmd.substring(6); // 跳过 "token "
      token.trim();

      if (token.length() > 0) {
        if (saveToken(token.c_str())) {
          blePrintln("Token saved successfully");
        } else {
          blePrintln("Failed to save token");
        }
      } else {
        // 空token表示清除
        if (clearToken()) {
          blePrintln("Token cleared");
        } else {
          blePrintln("Failed to clear token");
        }
      }
      return;
    }

    // 其他命令转小写
    cmd.toLowerCase();

    Serial.printf("[BLE] Received command: %s\n", cmd.c_str());

    if (cmd == "help") {
      blePrintln("=== Commands ===");
      blePrintln("token <key> - Set API token");
      blePrintln("status - System status");
      blePrintln("wifi - WiFi info");
      blePrintln("ip - IP address");
      blePrintln("ram - Memory info");
      blePrintln("restart - Restart ESP32");
    }
    else if (cmd == "status") {
      blePrintf("WiFi: %s\n", WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected");
      blePrintf("BLE: %s\n", deviceConnected ? "Connected" : "Disconnected");
      blePrintf("Auth: %s\n", isTokenSet() ? "OK" : "No");
      blePrintf("RAM: %u KB free\n", ESP.getFreeHeap() / 1024);
    }
    else if (cmd == "wifi") {
      if (WiFi.status() == WL_CONNECTED) {
        blePrintln("WiFi: Connected");
        blePrintf("SSID: %s\n", WiFi.SSID().c_str());
        blePrintf("RSSI: %d dBm\n", WiFi.RSSI());
      } else {
        blePrintln("WiFi: Disconnected");
      }
    }
    else if (cmd == "ip") {
      if (WiFi.status() == WL_CONNECTED) {
        blePrintf("IP: %s\n", WiFi.localIP().toString().c_str());
      } else {
        blePrintln("WiFi not connected");
      }
    }
    else if (cmd == "ram") {
      blePrintf("Free RAM: %u bytes\n", ESP.getFreeHeap());
      blePrintf("Free RAM: %u KB\n", ESP.getFreeHeap() / 1024);
    }
    else if (cmd == "restart") {
      blePrintln("Restarting...");
      delay(500);
      ESP.restart();
    }
    else {
      blePrintf("Unknown: %s\n", cmd.c_str());
      blePrintln("Type 'help' for commands");
    }
  }
}

/**
 * @brief 通过蓝牙打印日志
 */
void btLog(const char *message) {
  blePrintln(message);
}
