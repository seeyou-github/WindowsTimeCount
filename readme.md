# WindowsTimeCount

Win32 API + C++17 开发的 Windows 10 倒计时桌面应用。

## 目录结构

- `src/`：源代码
- `res/`：资源文件与中文字符串
- `build/`：编译输出目录
- `build.bat`：MinGW-w64 编译脚本

## 当前能力

- 通过预设按钮增加倒计时时长
- 开始、暂停、继续、停止、重置
- 桌面右下角半透明悬浮倒计时
- 到时声音提醒
- 循环模式自动进入下一轮倒计时

## 构建方式

在项目根目录执行：

```bat
build.bat
```

生成文件：

```text
build\WindowsTimeCount.exe
```
