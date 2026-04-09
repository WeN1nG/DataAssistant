# 优先级Bug完整修复说明

## 修复时间
2026-04-07

## 问题描述
当用户尝试添加优先级设置为非"一般"选项（如重要、紧急）的新日程时，系统错误地将所有此类日程统一按"一般"优先级进行存储。

---

## 修复方案

### 核心修改：将索引访问改为列名访问

为彻底解决索引问题，我将所有数据库查询方法从使用 `query.value(index)` 改为使用 `query.value("column_name")`。

#### 修改前（使用索引）
```cpp
schedule.priority = query.value(6).toInt();  // ❌ 可能因字段顺序变化而出错
```

#### 修改后（使用列名）
```cpp
schedule.priority = query.value("priority").toInt();  // ✅ 无论字段顺序如何都正确
```

### 修改的方法清单

| 方法名 | 文件位置 | 修改内容 |
|--------|----------|----------|
| `getBatchSchedulesByGroupId()` | DatabaseManager.cpp | 将索引改为列名 |
| `getSchedules()` | DatabaseManager.cpp | 将索引改为列名 |
| `getSchedulesByDate()` | DatabaseManager.cpp | 将索引改为列名 |
| `getScheduleById()` | DatabaseManager.cpp | 将索引改为列名 |

---

## 完整代码修改对照

### 1. getBatchSchedulesByGroupId()

```cpp
// 修改前
schedule.id = query.value(0).toInt();
schedule.priority = query.value(6).toInt();

// 修改后
schedule.id = query.value("id").toInt();
schedule.priority = query.value("priority").toInt();
```

### 2. getSchedules()

```cpp
// 修改前
schedule.id = query.value(0).toInt();
schedule.priority = query.value(6).toInt();

// 修改后
schedule.id = query.value("id").toInt();
schedule.priority = query.value("priority").toInt();
```

### 3. getSchedulesByDate()

```cpp
// 修改前
schedule.id = query.value(0).toInt();
schedule.priority = query.value(6).toInt();

// 修改后
schedule.id = query.value("id").toInt();
schedule.priority = query.value("priority").toInt();
```

### 4. getScheduleById()

```cpp
// 修改前
schedule.id = query.value(0).toInt();
schedule.priority = query.value(6).toInt();

// 修改后
schedule.id = query.value("id").toInt();
schedule.priority = query.value("priority").toInt();
```

---

## 数据清理建议

### ⚠️ 重要说明

由于之前的索引错误，数据库中**已有的日程数据可能包含错误的优先级值**。为确保测试结果准确，建议执行以下操作之一：

#### 方案一：清除旧数据（推荐）

删除数据库文件，重新开始：
```
1. 关闭应用程序
2. 删除数据库文件：
   - Windows: %APPDATA%/PersonalDateAssisant/schedules.db
   - Linux: ~/.local/share/PersonalDateAssisant/schedules.db
   - macOS: ~/Library/Application Support/PersonalDateAssisant/schedules.db
3. 重新启动应用程序
```

#### 方案二：使用SQL命令清理

```sql
-- 备份重要数据
CREATE TABLE schedules_backup AS SELECT * FROM schedules;

-- 将所有日程的优先级重置为"一般"（如果需要保留数据）
UPDATE schedules SET priority = 0;

-- 或者删除所有日程（如果可以接受）
DELETE FROM schedules;
```

---

## 完整测试清单

### 第一阶段：基础功能测试

- [ ] 重新编译程序
- [ ] 启动应用程序
- [ ] 验证日历视图正常显示

### 第二阶段：优先级添加测试

#### 测试2.1：添加"一般"优先级日程
- [ ] 点击"添加日程"按钮
- [ ] 输入标题："测试-一般优先级"
- [ ] 选择优先级："一般"
- [ ] 点击"保存"
- [ ] **验证**：在日历视图中查看，应显示淡蓝色背景

#### 测试2.2：添加"重要"优先级日程
- [ ] 点击"添加日程"按钮
- [ ] 输入标题："测试-重要优先级"
- [ ] 选择优先级："重要"
- [ ] 点击"保存"
- [ ] **验证**：在日历视图中查看，应显示淡黄色背景

#### 测试2.3：添加"紧急"优先级日程
- [ ] 点击"添加日程"按钮
- [ ] 输入标题："测试-紧急优先级"
- [ ] 选择优先级："紧急"
- [ ] 点击"保存"
- [ ] **验证**：在日历视图中查看，应显示淡红色背景

### 第三阶段：批量添加测试

#### 测试3.1：批量添加"重要"优先级日程
- [ ] 点击"添加日程"按钮
- [ ] 勾选"批量添加日程"
- [ ] 输入标题："测试-批量重要优先级"
- [ ] 设置开始日期：今天
- [ ] 设置结束日期：3天后
- [ ] 选择优先级："重要"
- [ ] 点击"保存"
- [ ] **验证**：
  - 提示信息显示添加了4个日程
  - 在日历视图中，这4天都应显示淡黄色背景

#### 测试3.2：批量添加"紧急"优先级日程
- [ ] 重复上述测试，选择"紧急"优先级
- [ ] **验证**：日历视图中应显示淡红色背景

### 第四阶段：编辑功能测试

#### 测试4.1：编辑"一般"为"重要"
- [ ] 找到"测试-一般优先级"日程
- [ ] 点击编辑
- [ ] 修改优先级为"重要"
- [ ] 保存
- [ ] **验证**：日历视图中应变为淡黄色背景

#### 测试4.2：编辑"重要"为"紧急"
- [ ] 找到"测试-重要优先级"日程
- [ ] 点击编辑
- [ ] 修改优先级为"紧急"
- [ ] 保存
- [ ] **验证**：日历视图中应变为淡红色背景

### 第五阶段：查看日程列表

- [ ] 打开"查看所有日程"对话框
- [ ] **验证**：
  - "一般"优先级的日程显示🔵蓝色圆点
  - "重要"优先级的日程显示🟠橙色圆点
  - "紧急"优先级的日程显示🔴红色圆点

### 第六阶段：数据验证（可选）

使用SQL工具直接查询数据库：
```sql
SELECT id, title, priority, color FROM schedules ORDER BY id DESC LIMIT 10;
```

预期结果：
| id | title | priority | color |
|----|----|----------|-------|
| 10 | 测试-紧急优先级 | 2 | #ff0000 |
| 9 | 测试-重要优先级 | 1 | #ffa500 |
| 8 | 测试-一般优先级 | 0 | #0000ff |

---

## 优先级值定义

| 优先级 | 数据库值 | 日历背景色 | 说明 |
|--------|----------|-----------|------|
| 一般 | 0 | 淡蓝色 #B4C8FF | 普通日程 |
| 重要 | 1 | 淡黄色 #FFFFB4 | 需要关注 |
| 紧急 | 2 | 淡红色 #FFB4B4 | 需要立即处理 |

---

## 修复的潜在问题

### 1. 索引偏移问题 ✅ 已修复
- **问题**：新增字段后索引计算错误
- **解决方案**：使用列名访问，完全避免索引问题

### 2. 字段顺序依赖 ✅ 已修复
- **问题**：代码依赖固定字段顺序
- **解决方案**：列名访问与字段顺序无关

### 3. 向后兼容性 ✅ 已确保
- **问题**：旧数据库可能字段不完整
- **解决方案**：
  - 使用 `IFNULL` 处理空值
  - 合理设置默认值

---

## 代码健壮性改进

### 改进1：空值处理

```cpp
// 修改前
schedule.endDatetime = QDateTime::fromString(query.value("endDatetime").toString(), Qt::ISODate);

// 修改后
QString endDatetimeStr = query.value("endDatetime").toString();
schedule.endDatetime = endDatetimeStr.isEmpty() 
    ? schedule.datetime  // 如果为空，使用datetime
    : QDateTime::fromString(endDatetimeStr, Qt::ISODate);
```

### 改进2：颜色默认值

```cpp
// 如果数据库中没有颜色值，使用优先级对应的颜色
if (!colorStr.isEmpty()) {
    schedule.color = QColor(colorStr);
} else {
    schedule.color = getColorByPriority(schedule.priority);
}
```

---

## 相关文档

- [项目逻辑文档](../logic/PROJECT_LOGIC_DOCUMENTATION.md)
- [DatabaseManager 源代码](../DatabaseManager.cpp)
- [优先级Bug修复报告](./优先级存储Bug修复报告.md)

---

## 后续建议

### 1. 添加单元测试
建议为数据库操作添加单元测试，确保：
- 不同优先级的日程能正确保存
- 从数据库读取的优先级值正确
- 颜色映射正确

### 2. 数据迁移脚本
如果需要保留旧数据，可以创建迁移脚本：
```sql
-- 确保所有新字段存在
ALTER TABLE schedules ADD COLUMN endDatetime TEXT DEFAULT '';
ALTER TABLE schedules ADD COLUMN isBatch INTEGER DEFAULT 0;
```

### 3. 日志记录
建议在关键操作点添加日志，便于调试：
```cpp
qDebug() << "Schedule saved with priority:" << schedule.priority;
qDebug() << "Schedule loaded with priority:" << loadedSchedule.priority;
```

---

## 联系与支持

如遇到问题，请：
1. 检查应用程序日志
2. 验证数据库文件权限
3. 尝试清除缓存后重新测试

---

**修复完成！请按照上述测试清单进行全面测试。**
