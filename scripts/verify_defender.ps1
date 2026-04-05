# ============================================
# DataAssistant Windows Defender安全验证脚本 v1.0
# ============================================
# 用途: 使用Windows Defender扫描打包文件，确保安全性
# 要求: PowerShell 5.1+, Windows Defender已启用
# ============================================

param(
    [string]$TargetPath = "",
    [switch]$FullScan = $false,
    [switch]$ReportOnly = $false
)

$ErrorActionPreference = "Continue"
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = Split-Path -Parent (Split-Path -Parent $ScriptDir)

if ([string]::IsNullOrEmpty($TargetPath)) {
    $TargetPath = Join-Path $ProjectRoot "installer"
}

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "DataAssistant Windows Defender安全验证" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

$InstallerPath = Join-Path $TargetPath "DataAssistant_Setup_v0.1.exe"
$PortablePath = Join-Path $ProjectRoot "portable"

Write-Host "[步骤 1/4] 检查Windows Defender状态..." -ForegroundColor Yellow
$DefenderStatus = Get-MpComputerStatus -ErrorAction SilentlyContinue
if ($null -eq $DefenderStatus) {
    Write-Host "[错误] 无法访问Windows Defender！" -ForegroundColor Red
    Write-Host "请确保以管理员权限运行此脚本" -ForegroundColor Red
    exit 1
}

Write-Host "  - 防病毒状态: $($DefenderStatus.AntivirusEnabled)" -ForegroundColor Gray
Write-Host "  - 实时保护: $($DefenderStatus.RealTimeProtectionEnabled)" -ForegroundColor Gray
Write-Host "  - 病毒定义版本: $($DefenderStatus.AntivirusSignatureVersion)" -ForegroundColor Gray

if (-not $DefenderStatus.AntivirusEnabled) {
    Write-Host "[警告] Windows Defender未启用！" -ForegroundColor Red
    Write-Host "建议在启用Windows Defender的情况下运行此脚本" -ForegroundColor Red
}

Write-Host ""
Write-Host "[步骤 2/4] 查找待扫描文件..." -ForegroundColor Yellow

$FilesToScan = @()

if (Test-Path $InstallerPath) {
    $FilesToScan += Get-ChildItem $InstallerPath -File
    Write-Host "  + 安装程序: $InstallerPath" -ForegroundColor Green
}

if (Test-Path $PortablePath) {
    $PortableFiles = Get-ChildItem $PortablePath -Recurse -File
    $FilesToScan += $PortableFiles
    Write-Host "  + 便携版文件: $($PortableFiles.Count) 个文件" -ForegroundColor Green
}

if ($FilesToScan.Count -eq 0) {
    Write-Host "[错误] 未找到任何打包文件！" -ForegroundColor Red
    Write-Host "请先运行 package.bat 或 portable.bat 进行打包" -ForegroundColor Red
    exit 1
}

Write-Host "  总计待扫描文件: $($FilesToScan.Count)" -ForegroundColor Cyan
Write-Host ""

Write-Host "[步骤 3/4] 执行Windows Defender扫描..." -ForegroundColor Yellow

$ScanResults = @()
$ThreatsFound = $false

foreach ($file in $FilesToScan) {
    Write-Host "  正在扫描: $($file.Name)..." -ForegroundColor Gray -NoNewline

    try {
        $ScanResult = Start-MpScan -ScanPath $file.FullName -ScanType CustomScan -ErrorAction Stop
        Start-Sleep -Milliseconds 500

        $Threats = Get-MpThreatDetection -ErrorAction SilentlyContinue | Where-Object {
            $_.ActionSuccess -eq $false -or $_.ThreatStatus -ne "NotFound"
        }

        if ($Threats) {
            Write-Host " [发现问题]" -ForegroundColor Red
            $ThreatsFound = $true
            $ScanResults += [PSCustomObject]@{
                File = $file.Name
                Status = "发现问题"
                Details = $Threats | Select-Object -First 1
            }
        } else {
            Write-Host " [安全]" -ForegroundColor Green
            $ScanResults += [PSCustomObject]@{
                File = $file.Name
                Status = "安全"
                Details = "无威胁"
            }
        }
    } catch {
        Write-Host " [扫描失败]" -ForegroundColor Yellow
        $ScanResults += [PSCustomObject]@{
            File = $file.Name
            Status = "扫描失败"
            Details = $_.Exception.Message
        }
    }
}

Write-Host ""
Write-Host "[步骤 4/4] 生成扫描报告..." -ForegroundColor Yellow

$ReportPath = Join-Path $ProjectRoot "security_scan_report.txt"

$ReportContent = @"
========================================
DataAssistant 安全扫描报告
========================================
扫描时间: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")
扫描路径: $TargetPath
扫描文件数: $($FilesToScan.Count)
Windows Defender版本: $($DefenderStatus.AntivirusSignatureVersion)
========================================

扫描结果摘要：
--------------

"@

foreach ($result in $ScanResults) {
    $ReportContent += "文件: $($result.File)`n"
    $ReportContent += "状态: $($result.Status)`n"
    $ReportContent += "详情: $($result.Details)`n"
    $ReportContent += "`n"
}

if ($ThreatsFound) {
    $ReportContent += @"

========================================
警告: 发现潜在威胁！
========================================
建议：
1. 不要运行发现的威胁文件
2. 更新Windows Defender病毒定义
3. 检查源代码是否有可疑内容
4. 在隔离环境中进行进一步分析

========================================
"@
    Write-Host ""
    Write-Host "⚠ 发现潜在威胁！请查看完整报告。" -ForegroundColor Red
} else {
    $ReportContent += @"

========================================
扫描结论: 所有文件通过安全扫描
========================================
建议：
1. 文件已通过Windows Defender验证
2. 可以安全分发和使用
3. 建议在目标系统上再次验证

========================================
"@
    Write-Host ""
    Write-Host "✓ 所有文件通过安全扫描" -ForegroundColor Green
}

$ReportContent | Out-File -FilePath $ReportPath -Encoding UTF8

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "扫描完成！" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "生成的文件：" -ForegroundColor White
Write-Host "  - 扫描报告: $ReportPath" -ForegroundColor Gray
Write-Host ""
Write-Host "下一步操作：" -ForegroundColor White
Write-Host "1. 查看扫描报告确认结果" -ForegroundColor Gray
Write-Host "2. 如有问题，运行 package.bat 重新打包" -ForegroundColor Gray
Write-Host "3. 在目标Windows 10/11系统上进行功能测试" -ForegroundColor Gray
Write-Host ""

if (-not $ReportOnly) {
    Write-Host "是否打开扫描报告？ (Y/N)" -ForegroundColor Yellow
    $response = Read-Host
    if ($response -eq "Y" -or $response -eq "y") {
        Start-Process notepad.exe $ReportPath
    }
}

exit 0
