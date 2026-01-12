# Web 服务器示例

## 功能说明
这个示例创建一个 Web 服务器，允许通过浏览器控制 ESP32 的 LED。

## 功能特性
- 🌐 通过浏览器控制 LED
- 📊 实时显示 LED 状态
- 📈 显示运行时间和内存信息
- 🔌 JSON API 接口
- 📱 响应式设计，支持手机访问

## 硬件要求
- ESP32-WROOM-32 开发板
- 2.4GHz Wi-Fi 网络

## 配置步骤
1. 打开 [webserver.ino](webserver.ino)
2. 修改 WiFi 配置：
   ```cpp
   const char* ssid = "你的WiFi名称";
   const char* password = "你的WiFi密码";
   ```
3. 保存并上传

## 使用步骤

### 1. 上传程序
- 选择开发板: ESP32 Dev Module
- 选择串口
- 上传程序

### 2. 获取 IP 地址
打开串口监视器（波特率 115200），查看输出：
```
✓ WiFi 连接成功!
IP 地址: 192.168.1.100
```

### 3. 访问 Web 界面
在浏览器中输入：`http://192.168.1.100`

### 4. 控制 LED
点击 "LED 开" 和 "LED 关" 按钮控制 LED

## API 接口

### 获取状态
```bash
GET /status
```

响应示例：
```json
{
  "led": 1,
  "count": 5,
  "uptime": 120,
  "heap": 267456,
  "rssi": -45
}
```

### 控制 LED
```bash
GET /on    # LED 开
GET /off   # LED 关
```

## 主页显示信息
- LED 当前状态
- 开关次数统计
- 运行时间
- 可用内存

## 预期结果
1. WiFi 连接成功
2. Web 服务器启动
3. 浏览器访问显示控制面板
4. 点击按钮可以控制 LED

## 扩展功能建议
- 添加更多 GPIO 控制
- 实现用户认证
- 添加定时任务
- 集成传感器数据
- WebSocket 实时更新
- mDNS 域名访问 (esp32.local)

## 故障排查

### 无法访问网页
1. 检查 IP 地址是否正确
2. 确认手机/电脑与 ESP32 在同一 WiFi
3. 检查防火墙设置
4. 尝试使用不同浏览器

### WiFi 连接失败
1. 确认 WiFi 为 2.4GHz
2. 检查密码是否正确
3. 靠近 WiFi 路由器

### LED 不响应
1. 检查串口输出
2. 刷新浏览器页面
3. 按下 ESP32 的 EN/RST 复位按钮
