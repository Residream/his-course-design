# HIS — 医院信息管理系统

基于 C 语言的终端医院信息管理系统（Hospital Information System），采用链表数据结构与管道符分隔的平文件持久化方案，支持患者、医生、管理员三种角色的登录与业务操作。

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
| 处方 | 查看处方用药记录                           |

### 医生端

| 模块     | 功能                                            |
| -------- | ----------------------------------------------- |
| 账户     | 登录、查看/修改个人信息                         |
| 患者     | 查看患者信息、查看挂号列表                      |
| 看诊     | 接诊、填写诊断、开检查单、开处方、办理住院/出院 |
| 记录管理 | 检查记录、住院记录、处方记录的增删改查          |
| 发药     | 处方发药（含科室权限校验与库存双重检查）        |

### 管理员端

| 模块      | 功能                                                                             |
| --------- | -------------------------------------------------------------------------------- |
| 患者管理  | 增删改查（支持姓名模糊搜索）、分页浏览                                           |
| 医生管理  | 增删改查（支持姓名/科室模糊搜索）                                                |
| 科室管理  | 增删改查                                                                         |
| 病房/床位 | 病房管理、床位分配与释放                                                         |
| 药品管理  | 药品信息（支持名称模糊搜索）、药房管理、库存管理                                 |
| 医疗记录  | 挂号、看诊、检查、住院（支持姓名模糊搜索）、处方（支持药品名称模糊搜索）记录管理 |

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

每个实体对应一个 `data/*.txt` 文件，字段以 `|` 分隔，每行一条记录。

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
│   │   └── prescription.h              处方
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
│   │   └── prescription.c             处方管理
│   └── ui/
│       └── menu.c                      三角色菜单驱动（~1960 行）
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

### 环境要求

- C 编译器（GCC / MinGW / MSVC，需支持 C11）
- CMake ≥ 3.10

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

> 可执行文件需在 `build/` 目录下运行，程序通过 `../data/` 相对路径访问数据文件。

### 默认管理员账号

使用 `gen_data.py` 生成的测试数据中，管理员用户名为 `root`，密码为 `root`。患者密码格式为 `p` + 编号（如 `p0001`），医生密码格式为 `d` + 编号（如 `d0001`）。

### 测试数据生成

```bash
python tools/gen_data.py
```

脚本自动定位项目根目录下的 `data/` 目录，无论从哪个目录运行都能正确生成。生成内容包括：130 名患者、30 名医生、5 个科室、8 个病房、28 种药品、3 个药房，以及完整的挂号→看诊→检查→处方→住院全流程关联数据。

---

## 技术实现

### 数据存储

- 每个实体对应一个 `.txt` 文件，字段以 `|` 分隔，每行一条记录
- 读写通过链表中转：加载时逐行解析构建链表，保存时遍历链表写入文件
- 写入采用临时文件 + `rename` 的原子操作（`safe_fopen_tmp` → `safe_fclose_commit`），防止写入中断导致数据损坏

### 数据完整性

- 删除科室前检查是否有关联医生，有则拒绝删除
- 删除医生前检查是否有未完成的挂号记录
- 删除患者前检查是否有未完成的挂号或进行中的住院记录
- 删除病房前检查是否有关联的床位记录
- 发药时同时检查药房库存和全局药品库存，防止库存变为负数

### 密码安全

- 用户密码经 16 字节随机盐 + SHA-256 哈希后存储
- 登录时重新计算哈希比对，明文密码不落盘
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

### 输入安全

- 过滤管道符 `|`，防止破坏文件分隔格式
- 完整的输入校验：姓名仅中文（UTF-8）、年龄范围、性别枚举、选项范围等
- 使用 `safe_input()` 统一处理输入缓冲区，自动清除残留数据

### 界面显示

- 菜单驱动的交互模式，支持分页浏览（每页 10 条）
- 自适应列宽的表格输出，正确处理中文字符宽度（UTF-8 双字节显示宽度）
- Unicode 边框绘制的菜单框架

### 编码说明

源码使用 UTF-8 编码。CMakeLists.txt 同时支持 GCC 和 MSVC 编译器（自动检测）。若在 Windows CMD/PowerShell 中遇到中文乱码，可取消 `CMakeLists.txt` 中 `if(WIN32)` 注释块，启用 `-fexec-charset=GBK` 编译选项后重新构建。
