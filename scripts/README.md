# DataAssistant 打包脚本使用说明

## 脚本目录说明

本目录包含DataAssistant Windows平台打包所需的全部脚本：

### 核心脚本

| 脚本 | 功能 | 使用场景 |
|------|------|----------|
| `build.bat` | 编译项目 | 单独编译源代码 |
| `deploy.bat` | 部署依赖 | 配合build使用 |
| `package.bat` | 完整打包 | **推荐使用** |
| `portable.bat` | 便携版打包 | 创建绿色版本 |
| `verify_defender.ps1` | 安全验证 | 检查打包文件 |
| `installer.iss` | 安装脚本 | Inno Setup配置 |

---

## 快速使用

### 1. 完整打包（推荐）

```powershell
cd scripts
.\package.bat
```

生成：
- `..\installer\DataAssistant_Setup_v0.1.exe` - 安装程序
- `..\build\deploy\` - 部署目录

### 2. 便携版

```powershell
cd scripts
.\portable.bat
```

生成：
- `..\portable\` - 便携版文件夹

### 3. 安全验证

```powershell
cd scripts
.\verify_defender.ps1
```

生成：
- `..\security_scan_report.txt` - 扫描报告

---

## 详细说明

### build.bat - 项目构建

**功能**: 使用CMake和MinGW编译Qt项目

**流程**:
1. 清理构建目录
2. 配置CMake
3. 编译源代码
4. 生成可执行文件

**输出**: `..\build\PersonalDateAssisant.exe`

---

### deploy.bat - 依赖部署

**功能**: 使用windeployqt部署Qt运行时库

**流程**:
1. 检查可执行文件
2. 创建部署目录
3. 复制可执行文件
4. 部署Qt DLL和资源

**输出**: `..\build\deploy\`

**包含内容**:
- Qt6核心库
- Qt6 Widgets/GUI/Sql/Network
- MinGW运行时
- Visual C++运行时
- QtAwesome字体

---

### package.bat - 完整打包（推荐）

**功能**: 一键完成所有打包步骤

**流程**:
1. 调用 build.bat 编译
2. 调用 deploy.bat 部署
3. 编译安装程序（需要Inno Setup）

**输出**:
- `..\installer\DataAssistant_Setup_v0.1.exe`
- `..\build\deploy\`

**耗时**: 约8-15分钟

---

### portable.bat - 便携版打包

**功能**: 创建无需安装的绿色版本

**特点**:
- 无需管理员权限
- 可在U盘运行
- 配置文件在用户目录
- 适合多电脑使用

**输出**:
- `..\portable\PersonalDateAssisant.exe`
- `..\portable\README.txt`

---

### verify_defender.ps1 - Windows Defender验证

**功能**: 使用Windows Defender扫描打包文件

**功能**:
- 扫描安装程序
- 扫描便携版
- 生成详细报告
- 检查威胁

**输出**: `..\security_scan_report.txt`

---

### installer.iss - Inno Setup配置

**功能**: 定义安装程序的各项配置

**可配置项**:
- 应用程序信息
- 安装路径
- 快捷方式
- 注册表项
- 多语言支持
- 卸载配置

---

## 环境要求

1. **Qt 6.x** (含MinGW)
2. **CMake 3.16+**
3. **Inno Setup 6.0+** (仅package.bat需要)

---

## 故障排除

### 常见问题

**Q: windeployqt找不到？**
A: 确保Qt已添加到系统PATH

**Q: Inno Setup编译失败？**
A: 检查installer.iss语法，确保Inno Setup已安装

**Q: 编译失败？**
A: 查看Qt和CMake错误信息，确保环境正确

---

## 下一步

1. 查看 [QUICKSTART.md](..\QUICKSTART.md) - 快速入门
2. 查看 [PACKAGING_GUIDE.md](..\PACKAGING_GUIDE.md) - 完整文档
3. 运行打包脚本体验

---

## 技术支持

如有问题，请查看 PACKAGING_GUIDE.md 中的详细说明和常见问题解答。

---

**版本**: v1.0
**更新日期**: 2026-04-05
