"""
汉字点阵提取工具

使用方法：
1. 访问 https://www.23bei.com/tool/216.html
2. 输入汉字: "奇境探索初始化完成请稍候设备状态已未连接开发板系统信息"
3. 点击"取模"按钮
4. 复制生成的 C51 格式数组
5. 将数据填入 src/mini_chinese_font.h 中的 miniChineseBitmaps 数组

配置说明：
- 字体种类: HZK1616宋体
- 输出格式: C51
- 数据排列: 从左到右从上到下
- 取模方式: 横向8点左高位
- 黑白取反: 正常
- 强制全角: ASCII自动转全角

生成的每个汉字数据格式：
const unsigned char hzk_1616[][32] = {
    /*--  文字:  奇  --*/
    /*--  宽x高: 16x16  --*/
    0x01,0x00,0x01,0x00,0x01,0x00,0x3F,0xF8,0x21,0x08,0x21,0x08,0x21,0x08,0x3F,0xF8,
    0x21,0x08,0x21,0x08,0x21,0x08,0x3F,0xF8,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,
};
"""

# 需要提取的汉字列表
CHINESE_CHARS = "奇境探索初始化完成请稍候设备状态已未连接开发板系统信息"

print("请访问 https://www.23bei.com/tool/216.html")
print(f"输入以下汉字: {CHINESE_CHARS}")
print("然后点击取模，将生成的数据复制到 src/mini_chinese_font.h")
