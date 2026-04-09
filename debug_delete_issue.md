# 批量删除功能问题诊断报告

## 问题分析

### 代码审查发现

经过严格检查所有相关代码，发现以下潜在问题：

### 1. 删除预览和删除逻辑问题

**问题代码位置**: `DatabaseManager.cpp` - `deleteSchedulesWithDetails` 方法

**潜在问题**:
当批量日程被选中时，删除逻辑使用以下条件匹配：
```cpp
DELETE FROM schedules 
WHERE isBatch = 1 
  AND title = ? 
  AND datetime = ? 
  AND endDatetime = ? 
  AND priority = ?
```

**分析**:
- ✅ 如果 `isBatch = 1` 设置正确，查询应该能匹配
- ⚠️ 如果 `isBatch = 0` 或 NULL，批量日程将无法被识别

### 2. 数据库字段验证

**检查点**:
- [x] `isBatch` 字段存在：`initializeDatabase()` 中已创建
- [x] 默认值设置：`DEFAULT 0`
- [x] 插入逻辑：`schedule.isBatch ? 1 : 0`
- [x] 读取逻辑：`query.value("isBatch").toInt() == 1`

### 3. 读取逻辑分析

在 `getDeletePreview` 和 `deleteSchedulesWithDetails` 中：

```cpp
bool isBatch = query.value(0).toInt() == 1;  // 索引方式
// 或
schedule.isBatch = query.value("isBatch").toInt() == 1;  // 列名方式
```

**问题**: 如果数据库中的 `isBatch` 字段值为 0 或 NULL，`isBatch` 将为 false。

### 4. 可能的根本原因

**最可能的原因**: 批量日程在数据库中的 `isBatch` 字段值不正确

**证据**:
1. `initializeDatabase()` 使用 `ALTER TABLE` 添加字段，依赖向后兼容
2. 旧数据库可能没有正确初始化 `isBatch` 字段
3. 某些情况下字段可能为 NULL 或 0

### 5. 验证方法

需要在数据库中执行以下 SQL 验证：

```sql
-- 检查批量日程的 isBatch 字段
SELECT id, title, datetime, endDatetime, isBatch 
FROM schedules 
WHERE isBatch = 1;

-- 检查所有日程的 isBatch 字段
SELECT id, title, datetime, endDatetime, isBatch 
FROM schedules;
```

### 6. 修复方案

#### 方案 A: 更新数据库中的 isBatch 值

如果批量日程的 `isBatch = 0`，需要更新：

```sql
-- 将 endDatetime > datetime 的日程标记为批量日程
UPDATE schedules 
SET isBatch = 1 
WHERE datetime != endDatetime 
   OR endDatetime > datetime;
```

#### 方案 B: 修改删除逻辑

不仅依赖 `isBatch` 字段，还检查 `endDatetime`：

```cpp
// 如果 endDatetime > datetime，视为批量日程
bool hasEndDate = QDateTime::fromString(endDatetime, Qt::ISODate) > 
                  QDateTime::fromString(datetime, Qt::ISODate);
bool shouldTreatAsBatch = (isBatch == 1) || hasEndDate;
```

## 实施修复

### 修复 1: 增强批量日程识别逻辑

修改 `deleteSchedulesWithDetails` 方法，同时检查 `isBatch` 和 `endDatetime`。

### 修复 2: 添加数据库验证方法

添加方法检查和修复数据完整性。

### 修复 3: 添加诊断输出

添加 qDebug 输出帮助调试。
