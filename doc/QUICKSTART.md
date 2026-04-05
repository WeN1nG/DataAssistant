# DataAssistant 快速入门指南

## 🚀 快速开始

### 环境要求

在开始之前，请确保已安装以下软件：

1. **Qt 6.x** (包含MinGW编译器)
   - 下载: https://www.qt.io/download-qt-installer

2. **CMake 3.16+**
   - 下载: https://cmake.org/download/

3. **Inno Setup 6.0+** (可选，用于创建安装程序)
   - 下载: https://jrsoftware.org/isinfo.php

---

## 📦 三种打包方式

### 方式一：完整打包（推荐）✅

生成包含安装向导的完整安装程序：

```powershell
cd scripts
.\package.bat
```

**输出**:
- `installer\DataAssistant_Setup_v0.1.exe` - 安装程序
- `build\deploy\` - 部署目录

**耗时**: 约 8-15 分钟

---

### 方式二：便携版打包

生成无需安装的绿色版本：

```powershell
cd scripts
.\portable.bat
```

**输出**:
- `portable\PersonalDateAssisant.exe` - 可执行文件
- `portable\README.txt` - 使用说明

**特点**:
- ✅ 无需安装，直接运行
- ✅ 可复制到U盘使用
- ✅ 适合多电脑使用
- ✅ 无管理员权限要求

**耗时**: 约 5-10 分钟

---

### 方式三：分步打包

如果需要更多控制，可以分步执行：

```powershell
# 步骤1: 编译项目
cd scripts
.\build.bat

# 步骤2: 部署依赖
.\deploy.bat

# 步骤3: 创建安装程序（需要Inno Setup）
iscc installer.iss
```

---

## 🔒 安全验证

打包完成后，运行Windows Defender安全扫描：

```powershell
cd scripts
.\verify_defender.ps1
```

这将：
1. 扫描所有打包文件
2. 检查潜在威胁
3. 生成安全报告: `security_scan_report.txt`

---

## 📋 交付物清单

### 1. 安装程序版

| 文件 | 路径 | 说明 |
|------|------|------|
| 安装程序 | `installer\DataAssistant_Setup_v0.1.exe` | 标准安装程序 |
| 部署目录 | `build\deploy\` | 完整部署包 |
| 安全报告 | `security_scan_report.txt` | 安全验证报告 |

### 2. 便携版

| 文件 | 路径 | 说明 |
|------|------|------|
| 主程序 | `portable\PersonalDateAssisant.exe` | 直接运行 |
| 使用说明 | `portable\README.txt` | 便携版说明 |
| 部署文件 | `portable\*` | 所有必需文件 |

---

## 💡 使用建议

### 新用户 → 使用安装程序版

1. 双击运行 `DataAssistant_Setup_v0.1.exe`
2. 按照安装向导操作
3. 完成安装
4. 从开始菜单启动程序

### 高级用户 → 使用便携版

1. 复制整个 `portable\` 文件夹
2. 携带到目标电脑
3. 直接运行 `PersonalDateAssisant.exe`
4. 无需安装，立即使用

### 开发者 → 查看详细文档

查看 [PACKAGING_GUIDE.md](PACKAGING_GUIDE.md) 获取：
- 高级配置选项
- 增量打包技巧
- 故障排除指南
- 兼容性测试指南

---

## ⚠️ 常见问题

### Q1: 提示"找不到Qt"？

**解决方案**:

```powershell
# 确保Qt已添加到PATH，或使用Qt Command Prompt
```

### Q2: Inno Setup未安装？

**解决方案**:
1. 下载Inno Setup: https://jrsoftware.org/isinfo.php
2. 安装后重新运行 `package.bat`

或使用便携版打包，无需Inno Setup：
```powershell
.\portable.bat
```

### Q3: Windows Defender警告？

**解决方案**:
1. 运行安全验证脚本
2. 查看详细报告
3. 签名安装程序（可选）

---

## 🔧 高级选项

### 增量打包（加速）

源代码未变更时：

```powershell
# 仅重新部署依赖
.\deploy.bat

# 仅重新创建安装程序
iscc installer.iss
```

### 自定义安装程序

修改 `scripts\installer.iss` 配置文件：
- 修改版本号
- 自定义安装路径
- 添加许可证协议

### 多平台支持

如需Linux/macOS版本，需要：
- Linux: 使用Linux Qt环境和Makefile
- macOS: 使用Xcode和.app打包

---

## 📞 获取帮助

查看完整文档: [PACKAGING_GUIDE.md](PACKAGING_GUIDE.md)

包含：
- 环境配置详解
- 脚本功能说明
- 高级配置选项
- 兼容性测试指南
- 常见问题解答

---

**开始打包吧！** 🎉

运行以下命令，立即体验：

```powershell
cd scripts
.\package.bat
```

或创建便携版：

```powershell
cd scripts
.\portable.bat
```
