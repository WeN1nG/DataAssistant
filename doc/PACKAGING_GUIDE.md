# DataAssistant Windows平台打包说明文档

## 文档信息

- **文档名称**: DataAssistant Windows打包指南
- **版本**: v1.0
- **创建日期**: 2026-04-05
- **适用系统**: Windows 10 / Windows 11
- **目标用户**: 开发者和最终用户

---

## 目录

1. [概述](#概述)
2. [环境准备](#环境准备)
3. [打包脚本说明](#打包脚本说明)
4. [快速开始](#快速开始)
5. [高级配置](#高级配置)
6. [Windows Defender安全验证](#windows-defender安全验证)
7. [常见问题](#常见问题)
8. [兼容性测试](#兼容性测试)

---

## 概述

本文档提供DataAssistant应用程序的Windows平台打包完整指南，包括：

- **自动化打包脚本**: 一键构建、部署和打包
- **安装程序**: 使用Inno Setup生成专业安装向导
- **便携版**: 无需安装的绿色版本
- **安全验证**: Windows Defender扫描和验证

### 打包产物

| 类型 | 路径 | 说明 |
|------|------|------|
| 安装程序 | `installer/DataAssistant_Setup_v0.1.exe` | 标准Windows安装程序 |
| 便携版 | `portable/` | 无需安装，直接运行 |
| 部署目录 | `build/deploy/` | 包含所有依赖的目录 |

---

## 环境准备

### 必需软件

1. **Qt 6.x**
   - 下载地址: https://www.qt.io/download-qt-installer
   - 必需组件:
     - Qt 6.x MinGW
     - Qt Creator
     - Qt 6.x CMake配置

2. **CMake 3.16+**
   - 下载地址: https://cmake.org/download/
   - 确保添加到系统PATH

3. **MinGW-w64**
   - 建议使用Qt自带的MinGW
   - 或下载独立版本: https://www.mingw-w64.org/

4. **Inno Setup 6.0+** (可选，仅用于创建安装程序)
   - 下载地址: https://jrsoftware.org/isinfo.php
   - 用于生成专业安装向导

5. **Windows Defender** (系统自带)
   - 用于安全验证
   - 确保实时保护已启用

### 系统要求

- **操作系统**: Windows 10 (1809+) 或 Windows 11
- **处理器**: x64架构
- **内存**: 4GB RAM (推荐 8GB)
- **磁盘空间**: 500MB可用空间
- **权限**: 标准用户权限即可（安装时需要管理员权限）

### 环境变量配置

确保以下路径已添加到系统PATH：

```powershell
# Qt路径示例
C:\Qt\6.x.x\mingw_64\bin
C:\Qt\6.x.x\mingw_64\lib

# CMake路径示例
C:\Program Files\CMake\bin

# MinGW路径示例
C:\Qt\Tools\mingw_64\bin
```

验证环境配置：

```powershell
# 检查Qt
qmake --version

# 检查CMake
cmake --version

# 检查MinGW
gcc --version
```

---

## 打包脚本说明

### 脚本文件列表

所有脚本位于 `scripts/` 目录：

| 脚本文件 | 功能 | 使用场景 |
|---------|------|----------|
| `build.bat` | 编译项目 | 单独编译 |
| `deploy.bat` | 部署Qt依赖 | 配合build使用 |
| `package.bat` | 完整打包流程 | 推荐使用 |
| `portable.bat` | 创建便携版 | 创建绿色版本 |
| `verify_defender.ps1` | Windows Defender验证 | 安全检查 |
| `installer.iss` | Inno Setup配置 | 安装程序模板 |

### 脚本详解

#### 1. build.bat - 项目构建脚本

**功能**: 使用CMake和MinGW编译项目

**执行流程**:
1. 清理构建目录
2. 配置CMake项目
3. 编译源代码
4. 生成可执行文件

**使用方法**:

```powershell
cd scripts
.\build.bat
```

**输出**:
- 可执行文件: `build/PersonalDateAssisant.exe`
- 构建日志: CMake配置和编译输出

**错误处理**:
- CMake配置失败: 检查Qt和CMake安装
- 编译失败: 检查源代码语法错误

#### 2. deploy.bat - Qt依赖部署脚本

**功能**: 使用windeployqt部署所有Qt运行时库

**执行流程**:
1. 检查可执行文件是否存在
2. 创建部署目录
3. 复制可执行文件
4. 使用windeployqt部署依赖
5. 复制QtAwesome字体资源

**使用方法**:

```powershell
cd scripts
.\deploy.bat
```

**输出**:
- 部署目录: `build/deploy/`
- 包含所有必需的DLL和资源

**包含的依赖**:
- Qt6核心库 (Qt6Core.dll, Qt6Gui.dll, Qt6Widgets.dll)
- Qt6 SQL模块
- Qt6 Network模块
- MinGW运行时库
- Visual C++运行时库
- QtAwesome字体文件

#### 3. package.bat - 完整打包脚本（推荐）

**功能**: 一键完成所有打包步骤

**执行流程**:
1. 调用 build.bat 编译项目
2. 调用 deploy.bat 部署依赖
3. 调用 Inno Setup 编译安装程序
4. 生成最终安装包

**使用方法**:

```powershell
cd scripts
.\package.bat
```

**输出**:
- 安装程序: `installer/DataAssistant_Setup_v0.1.exe`
- 部署目录: `build/deploy/`

**完整打包示例**:

```powershell
# 1. 进入脚本目录
cd scripts

# 2. 运行完整打包
.\package.bat

# 3. 验证安装程序
.\verify_defender.ps1

# 4. 测试安装程序（双击运行）
explorer installer
```

#### 4. portable.bat - 便携版打包脚本

**功能**: 创建无需安装的绿色版本

**特点**:
- 无需管理员权限
- 可在U盘运行
- 配置文件存储在用户目录
- 适合多电脑使用

**使用方法**:

```powershell
cd scripts
.\portable.bat
```

**输出**:
- 便携版目录: `portable/`
- 主程序: `portable/PersonalDateAssisant.exe`
- 说明文件: `portable/README.txt`

**便携版使用说明**:

1. 复制整个 `portable/` 文件夹到目标位置
2. 运行 `PersonalDateAssisant.exe`
3. 首次运行可能需要管理员权限
4. 数据和配置保存在用户目录

**数据存储位置**:
- Windows 10/11: `%LOCALAPPDATA%\DataAssistant`
- 配置文件: `settings.ini`
- 数据库文件: `data.db`

#### 5. verify_defender.ps1 - Windows Defender验证脚本

**功能**: 使用Windows Defender扫描打包文件

**使用方法**:

```powershell
# 基本扫描
cd scripts
.\verify_defender.ps1

# 指定扫描路径
.\verify_defender.ps1 -TargetPath "C:\Path\To\Scan"

# 仅生成报告
.\verify_defender.ps1 -ReportOnly
```

**输出**:
- 扫描报告: `security_scan_report.txt`
- 终端输出实时状态

**扫描内容**:
- 安装程序: `installer/DataAssistant_Setup_v0.1.exe`
- 便携版所有文件: `portable/*`

#### 6. installer.iss - Inno Setup配置脚本

**功能**: 定义安装程序的各项配置

**主要配置项**:

```ini
AppId: 应用程序唯一标识
AppName: 应用程序名称
AppVersion: 版本号
DefaultDirName: 默认安装目录
MinVersion: 最低Windows版本
OutputDir: 输出目录
Compression: 压缩方式
```

**自定义配置**:

1. 修改版本号:

```ini
#define MyAppVersion "0.2"
```

2. 修改安装路径:

```ini
DefaultDirName={autopf}\YourAppName
```

3. 添加许可证协议:

```ini
LicenseFile=..\LICENSE.md
```

4. 自定义安装图标:

```ini
SetupIconFile=..\icon.ico
```

---

## 快速开始

### 方式一：完整打包（推荐）

```powershell
# 1. 打开PowerShell或CMD
cd scripts

# 2. 运行完整打包脚本
.\package.bat

# 3. 等待打包完成（约5-10分钟）

# 4. 运行安全验证
.\verify_defender.ps1

# 5. 查看生成的安装程序
explorer ..\installer
```

### 方式二：仅便携版

```powershell
cd scripts
.\portable.bat
```

### 方式三：分步执行

```powershell
# 1. 编译项目
cd scripts
.\build.bat

# 2. 部署依赖
.\deploy.bat

# 3. 创建安装程序（需要Inno Setup）
iscc installer.iss

# 4. 安全验证
.\verify_defender.ps1
```

---

## 高级配置

### 增量打包

为提高效率，可使用增量打包，仅重新打包变更部分：

```powershell
# 1. 仅重新部署依赖（源代码未变时）
.\deploy.bat

# 2. 手动复制文件到部署目录
xcopy /y build\PersonalDateAssisant.exe build\deploy\

# 3. 仅重新编译安装程序
iscc installer.iss
```

### 自定义安装程序

修改 `installer.iss` 配置文件：

#### 添加自定义文件

```iss
[Files]
; 添加额外的资源文件
Source: "..\resources\*"; DestDir: "{app}\resources"; Flags: ignoreversion recursesubdirs
```

#### 添加注册表项

```iss
[Registry]
; 应用程序设置
Root: HKCU; Subkey: "Software\YourApp"; ValueType: string; ValueName: "InstallPath"; ValueData: "{app}"
```

#### 自定义快捷方式

```iss
[Icons]
Name: "{autoprograms}\Your App"; Filename: "{app}\YourApp.exe"; Comment: "Description"
```

### 多语言支持

Inno Setup支持多语言安装向导：

```iss
[Languages]
Name: "chinesesimplified"; MessagesFile: "compiler:Languages\ChineseSimplified.isl"
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "japanese"; MessagesFile: "compiler:Languages\Japanese.isl"
```

### 签名安装程序（可选）

为安装程序添加代码签名，增强安全性：

1. 获取代码签名证书
2. 使用signtool签名：

```powershell
signtool sign /f certificate.pfx /p password /tr http://timestamp.digicert.com /td SHA256 installer\DataAssistant_Setup_v0.1.exe
```

---

## Windows Defender安全验证

### 为什么需要验证？

Windows 10/11内置Windows Defender，会对未签名或新生成的应用程序发出警告。通过安全验证可以：

1. 确保打包文件无恶意代码
2. 减少误报警告
3. 增强用户信任
4. 符合Microsoft SmartScreen要求

### 验证步骤

```powershell
# 1. 以管理员身份运行PowerShell
cd scripts
.\verify_defender.ps1

# 2. 查看扫描报告
type ..\security_scan_report.txt

# 3. 如发现威胁，根据报告处理
```

### 减少误报的方法

1. **代码签名**: 使用受信任的证书签名
2. **提交Microsoft**: 将哈希提交到Windows Defender
3. **EV代码签名**: 使用EV证书（立即受信任）

---

## 常见问题

### Q1: 打包脚本运行失败

**问题**: 运行 `package.bat` 时出现错误

**解决方案**:
1. 检查Qt和CMake是否正确安装
2. 确保所有环境变量已配置
3. 查看错误信息，定位问题
4. 尝试分步执行 `build.bat` 和 `deploy.bat`

### Q2: windeployqt找不到

**问题**: 提示 "未找到 windeployqt 工具"

**解决方案**:
1. 确认Qt已正确安装
2. 添加Qt bin目录到PATH
3. 或使用Qt Command Prompt运行脚本

```powershell
# 添加Qt到PATH（临时）
$env:PATH += ";C:\Qt\6.x.x\mingw_64\bin"
```

### Q3: Inno Setup编译失败

**问题**: `iscc` 命令失败

**解决方案**:
1. 确认Inno Setup已安装
2. 检查 `installer.iss` 语法
3. 使用Inno Setup编辑器打开并编译
4. 查看Inno Setup编译日志

### Q4: Windows Defender误报

**问题**: 打包文件被标记为威胁

**解决方案**:
1. 运行 `verify_defender.ps1` 扫描
2. 提交文件哈希到Microsoft
3. 使用代码签名证书签名
4. 联系Microsoft Security Intelligence

### Q5: 安装程序无法在Windows 10运行

**问题**: 安装程序提示不兼容

**解决方案**:
1. 检查Inno Setup配置中的 `MinVersion`
2. 确保设置为 `10.0`
3. 重新编译安装程序
4. 测试不同Windows 10版本

### Q6: 便携版数据迁移

**问题**: 如何将便携版数据迁移到新电脑

**解决方案**:
1. 找到数据目录: `%LOCALAPPDATA%\DataAssistant`
2. 复制整个DataAssistant文件夹
3. 在新电脑的相同位置粘贴
4. 运行程序，数据自动加载

### Q7: 如何更新已安装的程序

**问题**: 如何更新到新版本

**解决方案**:
1. 重新运行打包脚本生成新版本
2. 用户运行新安装程序
3. 选择覆盖安装
4. 用户数据自动保留

---

## 兼容性测试

### 测试矩阵

| 测试环境 | 版本 | 测试结果 | 测试日期 |
|---------|------|---------|---------|
| Windows 10 Home | 21H2 | 待测试 | - |
| Windows 10 Pro | 22H2 | 待测试 | - |
| Windows 11 Home | 22H2 | 待测试 | - |
| Windows 11 Pro | 23H2 | 待测试 | - |

### 测试用例

#### 功能测试

- [ ] 程序启动正常
- [ ] 日历显示正确
- [ ] 日程添加/编辑/删除
- [ ] 提醒功能正常
- [ ] 设置保存成功
- [ ] 数据持久化正常

#### 安装测试

- [ ] 安装程序启动
- [ ] 安装向导显示正确
- [ ] 安装完成无错误
- [ ] 开始菜单快捷方式创建
- [ ] 桌面快捷方式创建（可选）
- [ ] 卸载程序功能正常
- [ ] 卸载后无残留文件

#### 便携版测试

- [ ] 复制到U盘运行正常
- [ ] 复制到新电脑运行正常
- [ ] 数据目录创建正确
- [ ] 配置文件加载正常

#### 安全测试

- [ ] Windows Defender扫描通过
- [ ] 无安全警告
- [ ] 数字签名验证通过（如已签名）
- [ ] SmartScreen检查

### 测试报告模板

```markdown
## 兼容性测试报告

### 测试环境
- 操作系统: Windows 10/11
- 版本: [具体版本号]
- 架构: x64
- 日期: [测试日期]

### 测试结果

#### 安装程序测试
| 测试项 | 结果 | 备注 |
|-------|------|------|
| 安装程序启动 | 通过/失败 | |
| 安装过程 | 通过/失败 | |
| 启动程序 | 通过/失败 | |
| 功能测试 | 通过/失败 | |

#### 便携版测试
| 测试项 | 结果 | 备注 |
|-------|------|------|
| 直接运行 | 通过/失败 | |
| 数据保存 | 通过/失败 | |
| 配置加载 | 通过/失败 | |

### 发现的问题
[列出发现的问题]

### 解决方案
[提供解决方案]

### 测试结论
[通过/不通过]

测试人: [姓名]
```

---

## 技术支持

### 获取帮助

1. 查看打包日志文件
2. 检查Qt和CMake错误信息
3. 参考Inno Setup文档: https://jrsoftware.org/ishelp/
4. Qt官方文档: https://doc.qt.io/

### 反馈问题

如遇到打包问题，请提供以下信息：

1. 操作系统版本
2. Qt版本
3. CMake版本
4. 完整的错误信息
5. 打包日志

---

## 附录

### A. 文件结构

```
DataAssistant/
├── scripts/
│   ├── build.bat              # 构建脚本
│   ├── deploy.bat             # 依赖部署脚本
│   ├── package.bat            # 完整打包脚本
│   ├── portable.bat           # 便携版打包脚本
│   ├── verify_defender.ps1    # 安全验证脚本
│   └── installer.iss          # Inno Setup配置
├── build/                     # 构建输出
│   ├── PersonalDateAssisant.exe
│   └── deploy/               # 部署目录
├── installer/                 # 安装程序输出
│   └── DataAssistant_Setup_v0.1.exe
├── portable/                  # 便携版
│   ├── PersonalDateAssisant.exe
│   └── README.txt
├── doc/                       # 文档目录
│   └── PACKAGING_GUIDE.md    # 本文档
└── security_scan_report.txt   # 安全扫描报告
```

### B. 打包时间估算

| 步骤 | 首次打包 | 增量打包 |
|------|---------|---------|
| CMake配置 | 1-2分钟 | <1分钟 |
| 编译项目 | 3-5分钟 | 1-2分钟 |
| 依赖部署 | 2-3分钟 | <1分钟 |
| 安装程序编译 | 2-5分钟 | 2-5分钟 |
| **总计** | **8-15分钟** | **4-9分钟** |

### C. 快速命令参考

```powershell
# 完整打包
.\scripts\package.bat

# 仅便携版
.\scripts\portable.bat

# 安全验证
.\scripts\verify_defender.ps1

# 清理构建
Remove-Item -Recurse .\build\*

# 查看安装程序
explorer .\installer\

# 查看便携版
explorer .\portable\
```

---

## 更新日志

### v1.0 (2026-04-05)
- 初始版本发布
- 支持Windows 10/11
- 包含完整打包脚本
- 提供Inno Setup配置
- 包含便携版打包
- Windows Defender验证支持

---

**文档结束**

如有问题或建议，请联系开发团队。
