# HIS — 医疗管理系统

> 吉林大学（JLU）2025级《程序设计基础课程设计》课程作业
>
> 基于 C 语言的终端医疗管理系统（Hospital Information System），采用链表数据结构与管道符分隔的平文件持久化方案，支持患者、医生、管理员三种角色的登录与业务操作。

### 平台支持

| 平台 | 编译器 | 状态 |
|------|--------|------|
| Windows | MinGW (GCC) | ✅ 主要开发环境 |
| macOS | Clang | ✅ 主要开发环境 |
| Linux | GCC | ✅ 已验证 |
| Windows | MSVC | ✅ 已适配 |

> 需要 C11 标准支持和 CMake ≥ 3.10。

---

## 功能概览

### 患者端

| 模块 | 功能                                       |
| ---- | ------------------------------------------ |
| 账户 | 自助注册、登录、查看/修改个人信息          |
| 挂号 | 按科室选择医生挂号、查询挂号记录、取消挂号 |
| 就诊 | 查看就诊记录与诊断结果                     |
| 检查 | 查看检查项目与检查结果                     |
| 住院 | 查看住院/出院记录                          |
| 处方 | 查看处方用药记录（分页浏览）               |

### 医生端

| 模块     | 功能                                            |
| -------- | ----------------------------------------------- |
| 账户     | 登录、查看/修改个人信息                         |
| 患者     | 查看患者信息、查看挂号列表                      |
| 看诊     | 接诊、填写诊断、开检查单、开处方、办理住院/出院、结束看诊（标记已完成） |
| 记录管理 | 检查记录、住院记录、处方记录的增删改查          |
| 发药     | 处方发药（含科室权限校验与库存双重检查）        |

### 管理员端

| 模块      | 功能                                                                             |
| --------- | -------------------------------------------------------------------------------- |
| 患者管理  | 增删改查（支持姓名模糊搜索）、分页浏览                                           |
| 医生管理  | 增删改查（支持姓名/科室模糊搜索）                                                |
| 科室管理  | 增删改查                                                                         |
| 病房/床位 | 病房管理（含 ICU/普通/VIP 三种类型与科室关联）、床位批量添加与释放                |
| 药品管理  | 药品信息（支持名称模糊搜索）、药房管理、库存入库与发药                           |
| 医疗记录  | 挂号、看诊、检查、住院（支持姓名模糊搜索）、处方（支持药品名称模糊搜索）记录管理 |
| 数据分析  | 病房利用率、科室门诊趋势、病房优化建议、药品使用排行与库存预警、住院时长分布与预测 |

---

## 数据模型

系统包含 12 个核心实体，全部以单向链表组织：

```
Patient ──┐
          ├── Registration ── Visit ──┬── Exam
Doctor  ──┘                           ├── Prescription ── Drug
Department                            └── Hospitalization ── Ward ── Bed
Pharmacy ── PharmacyDrug ── Drug
```

每个实体对应一个 `data/*.txt` 文件，字段以 `|` 分隔，每行一条记录。各实体文件格式如下：

| 文件 | 字段格式 |
|------|----------|
| `patients.txt` | `id\|name\|gender\|age\|pwd_hash\|salt` |
| `doctors.txt` | `id\|name\|gender\|department\|pwd_hash\|salt` |
| `admins.txt` | `name\|pwd_hash\|salt` |
| `departments.txt` | `name`（每行一个科室名） |
| `registrations.txt` | `reg_id\|p_id\|d_id\|when\|status` |
| `visits.txt` | `visit_id\|reg_id\|when\|status\|diagnosis` |
| `exams.txt` | `exam_id\|visit_id\|item\|result` |
| `hospitalizations.txt` | `hosp_id\|visit_id\|p_id\|ward_id\|bed_id\|admit_date\|discharge_date\|status` |
| `wards.txt` | `ward_id\|name\|type\|department\|capacity\|occupied` |
| `beds.txt` | `bed_id\|ward_id\|bed_no\|status` |
| `drugs.txt` | `id\|generic_name\|trade_name\|alias\|price\|stock\|department` |
| `pharmacies.txt` | `id\|name\|location` |
| `pharmacy_drugs.txt` | `pharmacy_id\|drug_id\|quantity` |
| `prescriptions.txt` | `pr_id\|visit_id\|p_id\|d_id\|drug_id\|dose\|frequency` |

---

## 项目结构

```
HIS/
├── CMakeLists.txt                  CMake 构建配置
├── README.md
├── include/
│   ├── core/                       基础设施头文件
│   │   ├── config.h                    全局配置宏、数据文件路径、状态常量
│   │   ├── structs.h                   所有结构体定义（12 个实体）
│   │   ├── auth.h                      登录验证与患者注册
│   │   ├── session.h                   会话管理（角色、用户ID）
│   │   ├── sha256.h                    SHA-256 哈希算法
│   │   └── utils.h                     工具函数（输入、校验、文件安全写入、表格对齐）
│   ├── model/                      数据模型头文件
│   │   ├── patient.h                   患者
│   │   ├── doctor.h                    医生
│   │   ├── department.h                科室
│   │   ├── registration.h              挂号
│   │   ├── visit.h                     看诊
│   │   ├── exam.h                      检查
│   │   ├── hospitalization.h           住院
│   │   ├── ward.h                      病房
│   │   ├── bed.h                       床位
│   │   ├── drug.h                      药品与药房
│   │   ├── prescription.h              处方
│   │   └── analytics.h                 数据分析模块常量与入口
│   └── ui/
│       └── menu.h                      菜单系统
├── src/
│   ├── core/                       基础设施实现
│   │   ├── main.c                      程序入口
│   │   ├── auth.c                      认证逻辑
│   │   ├── session.c                   会话状态管理
│   │   ├── sha256.c                    SHA-256 实现
│   │   ├── config.c                    状态文本数组定义
│   │   └── utils.c                     工具函数实现
│   ├── model/                      数据模型实现（加载/保存/CRUD/业务逻辑）
│   │   ├── patient.c, doctor.c, department.c
│   │   ├── registration.c, visit.c, exam.c
│   │   ├── hospitalization.c, ward.c, bed.c
│   │   ├── drug.c                      药品 + 药房 + 药房库存 + 发药业务
│   │   ├── prescription.c              处方管理
│   │   └── analytics.c                 数据分析报表与统计逻辑
│   └── ui/
│       └── menu.c                      三角色菜单驱动
├── data/                           数据文件（管道符分隔 .txt）
│   ├── patients.txt, doctors.txt, admins.txt
│   ├── departments.txt, registrations.txt, visits.txt
│   ├── exams.txt, hospitalizations.txt
│   ├── wards.txt, beds.txt
│   ├── drugs.txt, pharmacies.txt, pharmacy_drugs.txt
│   └── prescriptions.txt
├── tools/
│   └── gen_data.py                 测试数据生成脚本（Python）
└── build/                          构建产物（不入版本控制）
```

---

## 构建与运行

### 构建步骤

```bash
cd HIS
mkdir build
cd build
cmake ..
cmake --build .
```

### 运行

```bash
# 在 build 目录下运行
# Linux / macOS
./his

# Windows (GCC/MinGW)
./his.exe

# Windows (MSVC)
Debug\his.exe
```

> ⚠️ 可执行文件需在 `build/` 目录下运行，程序通过 `../data/` 相对路径访问数据文件。

### 默认账号

使用 `gen_data.py` 生成的测试数据中，管理员用户名为 `root`，密码为 `root`。患者密码格式为 `p` + 编号（如 `p0001`），医生密码格式为 `d` + 编号（如 `d0001`）。

### 测试数据生成

```bash
python tools/gen_data.py
```

脚本自动定位项目根目录下的 `data/` 目录，无论从哪个目录运行都能正确生成。生成内容包括：130 名患者、30 名医生、5 个科室、8 个病房（ICU/普通/VIP 三种类型）、35 种药品、3 个药房，以及完整的挂号→看诊→检查→处方→住院全流程关联数据。

---

## 技术实现

### 数据存储与原子写入

- 每个实体对应一个 `.txt` 文件，字段以 `|` 分隔，每行一条记录
- 读写通过链表中转：加载时逐行解析构建链表，保存时遍历链表写入文件
- 写入采用临时文件 + `rename` 的原子操作（`safe_fopen_tmp` → `safe_fclose_commit`），确保单个文件的写入不会因中断导致数据损坏
- Windows 平台下 `rename` 前先 `remove` 目标文件（Windows 不支持覆盖式 rename）

### 多表联动与回滚策略

系统中多个业务操作涉及多张表的联动修改，采用统一的回滚策略保证数据一致性：

**涉及多表联动的操作：**

| 操作 | 联动表 |
|------|--------|
| 办理住院/出院 | hospitalizations + beds + wards |
| 开始看诊 | registrations + visits |
| 添加/删除床位 | beds + wards |
| 入库/发药 | pharmacy_drugs + drugs |
| 删除药品/药房 | drugs/pharmacies + pharmacy_drugs |

**回滚流程（以办理住院为例）：**

```
1. 快照: 保存修改前的床位状态、病房占用数等关键字段
2. 修改: 在内存中执行入院操作（新增住院记录、占用床位、增加病房计数）
3. 保存: 顺序写入 hospitalizations → beds → wards 三个文件
4. 校验: 若三个文件全部保存成功，则提交完成
5. 回滚: 若任一文件保存失败，从快照恢复内存状态，重新写入全部文件
6. 告警: 若回滚写入也失败，提示用户检查数据文件
```

> **设计局限**：多文件的保存是顺序执行而非原子操作。若在文件 A 写入成功、文件 B 写入前进程被强杀（如断电），文件间会出现不一致。彻底解决此问题需要预写日志（WAL）或两阶段提交机制，超出本课设范围。

### 数据完整性约束

删除操作前执行级联安全检查，防止产生悬挂引用：

- 删除科室前检查是否有关联医生，有则拒绝删除
- 删除医生前检查是否有未完成的挂号记录
- 删除患者前检查是否有未完成的挂号或进行中的住院记录
- 删除病房前检查是否有关联的床位记录
- 删除药品/药房时级联清除 `pharmacy_drugs` 中的关联记录
- 删除已占用床位时拒绝操作
- 发药时同时检查药房库存和全局药品库存，防止库存变为负数

### 密码安全

- 用户密码经 16 字节随机盐 + SHA-256 哈希后存储
- 登录时重新计算哈希比对，明文密码不落盘
- 患者修改密码需先验证旧密码
- 管理员存储格式：`name|hash|salt`；患者/医生的 hash 和 salt 嵌入各自记录字段中

### 会话管理

- 基于全局 `Session` 结构体，记录当前登录角色（patient / doctor / admin）和用户 ID
- 登录成功后调用 `session_set()` 建立会话，退出时调用 `session_clear()` 清除

### 查询功能

- 患者查询：按 ID 精确查询 / 按姓名模糊搜索
- 医生查询：按 ID 精确查询 / 按姓名模糊搜索 / 按科室模糊搜索
- 药品查询：按 ID / 名称（通用名/商品名/别名）/ 科室模糊搜索
- 处方查询：按处方ID / 看诊ID / 患者ID / 医生ID 精确查询 / 按药品名称模糊搜索
- 住院查询：按住院ID / 看诊ID / 患者ID 精确查询 / 按患者姓名模糊搜索
- 模糊搜索基于 `strstr` 子串匹配，支持多条结果同时展示

### 数据分析

管理员端提供五个分析模块，均基于链表遍历和内存聚合实现：

| 模块 | 分析内容 |
|------|----------|
| 病房利用率 | 利用率总览（含进度条）、周转率统计、空床率排名、平均住院天数 |
| 科室门诊趋势 | 按科室聚合挂号量/完成量/取消率、基于时间窗口的趋势百分比、医生接诊量 Top N |
| 病房优化 | 病房住院统计、科室-病房分布矩阵、扩容/缩减/缓冲区建议 |
| 药品使用 | 使用排行 Top N、科室用药金额估算、库存预警（按可用月数分级） |
| 住院时长 | 天数分布直方图、分病房平均天数、在院超期患者提醒 |

### 输入安全

- 过滤管道符 `|`，防止破坏文件分隔格式
- 完整的输入校验：姓名仅中文（UTF-8）、年龄范围 1-150（含负数拦截）、性别枚举、选项范围等
- 使用 `safe_input()` 统一处理输入缓冲区，自动清除残留数据
- 所有 `strtok` 解析均逐字段校验，格式异常的行静默跳过

### 界面显示

- 菜单驱动的交互模式，支持分页浏览（每页 10 条）
- 自适应列宽的表格输出，正确处理中文字符宽度（UTF-8 双字节显示宽度）
- Unicode 边框绘制的菜单框架
- 数据分析模块支持表格、进度条 `[████░░░░]`、柱状图等文本可视化输出

### 编码与代码风格

- 源码使用 UTF-8 编码，换行符统一为 LF
- 注释风格：独占一行的注释使用 `/* */`，跟在代码后的行尾注释使用 `//`
- 编译标准 C11，`-Wall -Wextra -Werror` 零警告通过
- CMakeLists.txt 同时支持 GCC、Clang 和 MSVC 编译器（自动检测）
- 若在 Windows CMD/PowerShell 中遇到中文乱码，可取消 `CMakeLists.txt` 中 `if(WIN32)` 注释块，启用 `-fexec-charset=GBK` 编译选项后重新构建
