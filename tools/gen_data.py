#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
HIS 课程设计 —— 演示专用数据生成器（满足课设要求）

规模目标（来自题签特殊说明 §3）：
- 门诊患者 ≥ 100   → 生成 110 名
- 住院患者 ≥ 30    → 生成 30+ 名唯一住院患者 / 45 条住院记录
- 医生 ≥ 20       → 每科 4 人，共 20 名
- 科室 ≥ 5        → 5 个
- 病房类型 ≥ 3    → ICU / 普通 / VIP 均有
- 药品 ≥ 20       → 35 种（含通用名/商品名/别名/科室归属）
- 重名测试        → 3 组同名数据

分析功能覆盖：
- 病房利用率：VIP 4/6 较满 → 普通 13% 较低，跨度合理
- 住院时长分布：1-35 天各档齐全
- 药品使用排行：Top10 有明显峰值，部分药品零使用
- 科室门诊趋势：内科最多，急诊科最少，4 档分布
- 库存预警：DR0029 硫酸吗啡 stock=5

主线：
- P0001 张雅琪（内科，主角）：覆盖全流程
- D0001 王海峰（内科，主角医生）：名下有待接诊/看诊中/已完成
"""
import hashlib
import random
from datetime import datetime, timedelta
from pathlib import Path

DATA_DIR = Path(__file__).resolve().parent.parent / "data"
DATA_DIR.mkdir(exist_ok=True)
random.seed(20250617)


def hash_password(password, salt_raw):
    m = hashlib.sha256()
    m.update(salt_raw)
    m.update(password.encode("utf-8"))
    return m.hexdigest(), salt_raw.hex()


def fixed_salt(seed):
    return hashlib.sha256(f"his-demo-salt-{seed}".encode()).digest()[:16]


def write_file(filename, lines):
    path = DATA_DIR / filename
    with open(path, "w", encoding="utf-8", newline="\n") as f:
        for line in lines:
            f.write(line + "\n")
    print(f"  写入 {filename}: {len(lines)} 条")


NOW = datetime.now()
def days_ago(d, h=10, m=0):
    return (NOW - timedelta(days=d)).replace(hour=h, minute=m, second=0, microsecond=0)


def today_time(h=10, m=0):
    """返回今天的指定时间点（用于"未就诊"挂号和"看诊中"看诊，避免出现昨日未就诊或前日看诊中的不合理状态）"""
    return NOW.replace(hour=h, minute=m, second=0, microsecond=0)


def ts(dt): return int(dt.timestamp())

# 1. 管理员
print("生成管理员数据...")
admin_hash, admin_salt = hash_password("root", fixed_salt(0))
write_file("admins.txt", [f"root|{admin_hash}|{admin_salt}"])

# 2. 科室
print("生成科室数据...")
departments = ["内科", "外科", "妇产科", "儿科", "急诊科"]
write_file("departments.txt", departments)

# 3. 医生 (20)
print("生成医生数据...")
doctors_data = [
    ("D0001", "王海峰",   "男", "内科"), ("D0002", "王海洋", "男", "内科"),
    ("D0003", "刘敏",     "女", "内科"), ("D0016", "陈建国", "男", "内科"),
    ("D0004", "李建国",   "男", "外科"), ("D0005", "李建军", "男", "外科"),
    ("D0006", "周磊",     "男", "外科"), ("D0017", "郑淑华", "女", "外科"),
    ("D0007", "陈婷",     "女", "妇产科"), ("D0008", "吴芳", "女", "妇产科"),
    ("D0009", "黄丽",     "女", "妇产科"), ("D0018", "钱彩霞", "女", "妇产科"),
    ("D0010", "孙浩然",   "男", "儿科"), ("D0011", "郑明轩", "男", "儿科"),
    ("D0012", "冯晓雯",   "女", "儿科"), ("D0019", "江婉茹", "女", "儿科"),
    ("D0013", "朱强",     "男", "急诊科"), ("D0014", "徐峰",  "男", "急诊科"),
    ("D0015", "林勇",     "男", "急诊科"), ("D0020", "王海峰", "男", "急诊科"),
]
doctor_lines = []
for idx, (did, name, gender, dept) in enumerate(doctors_data, start=1):
    pwd_hash, salt_hex = hash_password(f"d{int(did[1:]):04d}", fixed_salt(100 + idx))
    doctor_lines.append(f"{did}|{name}|{gender}|{dept}|{pwd_hash}|{salt_hex}")
write_file("doctors.txt", doctor_lines)

# 4. 患者 (110)
print("生成患者数据...")
hero_patients = [
    ("P0001", "张雅琪", "女", 42), ("P0002", "张雅琴", "女", 38), ("P0003", "张雅文", "女", 45),
    ("P0004", "李建明", "男", 55), ("P0005", "李建华", "男", 60), ("P0006", "王芳",   "女", 35),
    ("P0007", "赵磊",   "男", 50), ("P0008", "孙丽",   "女", 29), ("P0009", "周敏",   "女", 33),
    ("P0010", "吴涛",   "男", 47), ("P0011", "郑小宇", "男", 8),  ("P0012", "冯小雅", "女", 6),
    ("P0013", "陈刚",   "男", 52), ("P0014", "楚文静", "女", 28), ("P0015", "卫国强", "男", 65),
]
support_patients = [
    ("P0016", "蒋丽丽", "女", 31), ("P0017", "沈佳怡", "女", 25), ("P0018", "韩梦洁", "女", 30),
    ("P0019", "杨浩宇", "男", 5),  ("P0020", "朱建邦", "男", 58), ("P0021", "秦思远", "男", 40),
    ("P0022", "尤嘉慧", "女", 27), ("P0023", "许文博", "男", 48), ("P0024", "何静怡", "女", 32),
    ("P0025", "吕俊杰", "男", 39), ("P0026", "施雅婷", "女", 26), ("P0027", "张伟业", "男", 45),
    ("P0028", "孔梦瑶", "女", 23), ("P0029", "曹子轩", "男", 34), ("P0030", "严思琪", "女", 29),
]
deletable_patients = [
    ("P0031", "华佳乐", "男", 41), ("P0032", "金雅楠", "女", 36), ("P0065", "魏思琪", "女", 24),
]
duplicate_patients = [
    ("P0066", "张雅琪", "女", 55), ("P0088", "李建明", "男", 32), ("P0099", "王海峰", "男", 50),
]

_surnames = ["陈","林","黄","周","吴","徐","孙","胡","朱","高","郭","何","罗","宋","谢","韩","唐","冯","邓","曹","彭","曾","肖","田","董","袁","潘","蔡","蒋","余"]
_female_given = ["婷婷","晓雯","佳怡","梦洁","雨桐","思雨","欣怡","子萱","嘉慧","静怡","雅婷","思琪","梦瑶","佳乐","雅楠"]
_male_given   = ["浩然","子轩","俊杰","博文","天佑","志强","建邦","伟业","文博","嘉伟","明轩","浩宇","家豪","思远","俊豪"]

filler_patients = []
fid = 33
while len(filler_patients) < 74:
    if fid in (65, 66, 88, 99):
        fid += 1
        continue
    gender = random.choice(["男", "女"])
    name = random.choice(_surnames) + random.choice(_female_given if gender == "女" else _male_given)
    filler_patients.append((f"P{fid:04d}", name, gender, random.randint(3, 85)))
    fid += 1

all_patients = hero_patients + support_patients + deletable_patients + duplicate_patients + filler_patients
all_patients.sort(key=lambda p: p[0])
patient_lines = []
for idx, (pid, name, gender, age) in enumerate(all_patients, start=1):
    pwd_hash, salt_hex = hash_password(f"p{int(pid[1:]):04d}", fixed_salt(1000 + idx))
    patient_lines.append(f"{pid}|{name}|{gender}|{age}|{pwd_hash}|{salt_hex}")
write_file("patients.txt", patient_lines)

# 5. 病房设置（扩展床位数）
wards_data = [
    ("W0001", "内科ICU病区",  "ICU",  "内科",   8),
    ("W0002", "内科普通病区", "普通", "内科",   15),
    ("W0003", "内科VIP病区",  "VIP",  "内科",   4),
    ("W0004", "外科普通病区", "普通", "外科",   12),
    ("W0005", "外科ICU病区",  "ICU",  "外科",   6),
    ("W0006", "妇产科病区",   "普通", "妇产科", 10),
    ("W0007", "儿科病区",     "普通", "儿科",   10),
    ("W0008", "综合VIP病区",  "VIP",  "综合",   6),
]

# 6. 药品
print("生成药品数据...")
drugs_data = [
    ("DR0001","缬沙坦胶洗","代文","缬沙坦",45.00,1000,"内科"),
    ("DR0002","氨氯地平片","络活喜","氨氯地平",33.50,800,"内科"),
    ("DR0003","二甲双胍片","格华止","降糖药",25.00,1500,"内科"),
    ("DR0004","兰索拉唑肠溶片","达克普隆","兰索拉唑",40.00,600,"内科"),
    ("DR0005","阿托伐他汀钙片","立普妥","降脂药",50.00,900,"内科"),
    ("DR0006","美托洛尔缓释片","倍他乐克","美托洛尔",38.00,700,"内科"),
    ("DR0007","氯吡格雷片","波立维","抗血小板药",62.00,500,"内科"),
    ("DR0008","头孢克肟分散片","世福素","头孢",32.00,1200,"外科"),
    ("DR0009","双氯芬酸钠缓释片","扶他林","止痛片",28.00,800,"外科"),
    ("DR0010","阿莫西林胶囊","阿莫仙","阿莫西林",15.50,2000,"外科"),
    ("DR0011","塞来昔布胶囊","西乐葆","塞来昔布",45.50,500,"外科"),
    ("DR0012","莫匹罗星软膏","百多邦","消炎药膏",18.00,400,"外科"),
    ("DR0013","左氧氟沙星片","可乐必妥","左氧",35.00,900,"外科"),
    ("DR0014","氨甲环酸片","妥塞敏","止血药",29.00,600,"外科"),
    ("DR0015","叶酸片","斯利安","叶酸",12.00,1000,"妇产科"),
    ("DR0016","黄体酮胶囊","益玛欣","黄体酮",35.00,600,"妇产科"),
    ("DR0017","硫酸亚铁片","速力菲","铁剂",22.00,800,"妇产科"),
    ("DR0018","甲硝唑栓","达克宁","甲硝唑",18.50,500,"妇产科"),
    ("DR0019","维生素E软胶囊","来益","维E",25.00,600,"妇产科"),
    ("DR0020","地屈孕酮片","达芙通","孕酮",48.00,400,"妇产科"),
    ("DR0021","对乙酰氨基酚滴剂","泰诺林","退烧药",20.00,800,"儿科"),
    ("DR0022","布洛芬混悬液","美林","布洛芬",25.00,1000,"儿科"),
    ("DR0023","盐酸氨溴索溶液","沐舒坦","化痰药",28.00,600,"儿科"),
    ("DR0024","氯雷他定糖浆","开瑞坦","抗过敏药",30.00,500,"儿科"),
    ("DR0025","枯草杆菌颗粒","妈咪爱","益生菌",35.00,700,"儿科"),
    ("DR0026","蒙脱石散","思密达","止泻药",16.00,900,"儿科"),
    ("DR0027","盐酸肾上腺素注射液","肾上腺素","副肾碱",15.00,200,"急诊科"),
    ("DR0028","硝酸甘油片","硝酸甘油","硝甘",12.50,300,"急诊科"),
    ("DR0029","硫酸吗啡注射液","吗啡","吗啡",55.00,5,"急诊科"),
    ("DR0030","地西泮注射液","安定","安定",8.00,250,"急诊科"),
    ("DR0031","0.9%氯化钠注射液","生理盐水","盐水",5.50,5000,"通用"),
    ("DR0032","5%葡萄糖注射液","葡萄糖","糖水",6.00,5000,"通用"),
    ("DR0033","维生素C片","维C","维C",10.00,2000,"通用"),
    ("DR0034","布洛芬缓释胶囊","芬必得","布洛芬",22.00,1500,"通用"),
    ("DR0035","对乙酰氨基酚片","泰诺林","退烧药",18.00,1000,"通用"),
]

# —— 库存预警演示专用覆盖 ——
# 原 drugs_data 的库存是"正常业务值"，我们针对几款高频处方药人为压低库存，
# 让数据分析页能展示 ⚠ 补货 / ⚠ 关注 / ✓ 正常 三档状态。
# 不变式 (drug.stock = Σ pharmacy_drugs) 由后续 distribute_stock 保证。
_low_stock_override = {
    "DR0004": 15,   # 兰索拉唑  月均~7 → 2.1月 → ⚠ 补货
    "DR0009": 30,   # 扶他林    月均~7 → 4.3月 → ⚠ 关注
    "DR0028": 25,   # 硝酸甘油  月均~6 → 4.4月 → ⚠ 关注
    # DR0029 吗啡已经是 5，加上后面注入的 5 条处方 → ⚠ 补货
}
drugs_data = [
    (did, gn, tn, al, price, _low_stock_override.get(did, stock), dept)
    for did, gn, tn, al, price, stock, dept in drugs_data
]
# Fix first drug typo
drugs_data[0] = ("DR0001","缬沙坦胶囊","代文","缬沙坦",45.00,1000,"内科")
drug_lines = [f"{did}|{gn}|{tn}|{al}|{price:.2f}|{stock}|{dept}"
              for did, gn, tn, al, price, stock, dept in drugs_data]
write_file("drugs.txt", drug_lines)

# 7. 药房
print("生成药房数据...")
pharmacies_data = [
    ("PH0001","门诊西药房","门诊楼一楼"),
    ("PH0002","急诊专属药房","急诊楼大厅"),
    ("PH0003","中心住院药房","住院部二楼"),
]
write_file("pharmacies.txt", [f"{pid}|{n}|{loc}" for pid, n, loc in pharmacies_data])

# 8. 药房库存（关键不变式: drug.stock = Σ pharmacy_drugs.quantity）
#
# 分配策略按业务场景：
#   科室药 (内/外/妇/儿)：门诊西药房 55% + 中心住院药房 45%
#   急诊科药:             急诊专属药房 70% + 中心住院药房 30%
#   通用药:               门诊 40% + 急诊 20% + 住院 40%
#
# 整数分配算法: 前 n-1 份按比例 floor，最后一份 = 总量 - 已分配，消除舍入误差。
# 极低库存药品 (如吗啡 stock=3) 也能得到整数分配，不会塌缩到单一药房。
print("生成药房库存...")

def distribute_stock(total, shares):
    """按比例 shares=[(pharmacy_id, ratio), ...] 把 total 分配成整数。
    ratio 之和必须 == 1.0，最后一份吸收舍入差。"""
    assert abs(sum(r for _, r in shares) - 1.0) < 1e-6
    out = []
    remaining = total
    for i, (pid, r) in enumerate(shares):
        if i == len(shares) - 1:
            qty = remaining
        else:
            qty = int(total * r)
            remaining -= qty
        out.append((pid, qty))
    return out

pharmacy_drugs_lines = []
for did, _, _, _, _, stock, dept in drugs_data:
    if dept in ("内科", "外科", "妇产科", "儿科"):
        shares = [("PH0001", 0.55), ("PH0003", 0.45)]
    elif dept == "急诊科":
        shares = [("PH0002", 0.70), ("PH0003", 0.30)]
    else:  # 通用
        shares = [("PH0001", 0.40), ("PH0002", 0.20), ("PH0003", 0.40)]

    for pid, qty in distribute_stock(stock, shares):
        if qty > 0:   # 跳过 0 件的行，保持文件干净
            pharmacy_drugs_lines.append(f"{pid}|{did}|{qty}")

write_file("pharmacy_drugs.txt", pharmacy_drugs_lines)

# 9. 业务记录生成
print("生成业务记录...")
registrations = []; visits = []; exams = []; hosps = []; prescriptions = []
_rc = [1]; _vc = [1]; _ec = [1]; _hc = [1]; _pc = [1]

def add_reg(p, d, w, s):
    rid = f"R{_rc[0]:04d}"; registrations.append((rid, p, d, w, s)); _rc[0] += 1; return rid
def add_visit(r, w, s, diag):
    vid = f"V{_vc[0]:04d}"; visits.append((vid, r, w, s, diag)); _vc[0] += 1; return vid
def add_exam(v, i, r):
    eid = f"E{_ec[0]:04d}"; exams.append((eid, v, i, r)); _ec[0] += 1; return eid
def add_hosp(v, p, w, b, ad, di, s):
    hid = f"H{_hc[0]:04d}"; hosps.append((hid, v, p, w, b, ad, di, s)); _hc[0] += 1; return hid
def add_pr(v, p, d, drug, dose, freq):
    prid = f"PR{_pc[0]:04d}"; prescriptions.append((prid, v, p, d, drug, dose, freq)); _pc[0] += 1; return prid

# ===== 主线：P0001 =====
r_p1_1 = add_reg("P0001", "D0001", today_time(9, 30), 0)              # 未就诊 → 今天
r_p1_2 = add_reg("P0001", "D0001", days_ago(35, 10, 15), 1)
r_p1_3 = add_reg("P0001", "D0002", days_ago(20, 14, 0), 2)
r_p1_4 = add_reg("P0001", "D0002", days_ago(60, 10, 0), 1)
r_p1_5 = add_reg("P0001", "D0001", today_time(11, 0), 1)              # 对应看诊中 → 今天
v_p1_1 = add_visit(r_p1_2, days_ago(35, 12, 15), 1, "原发性高血压 2 级（中危）")
v_p1_2 = add_visit(r_p1_4, days_ago(60, 11, 0), 1, "2 型糖尿病伴酮症酸中毒")
v_p1_3 = add_visit(r_p1_5, today_time(12, 0), 0, "高血压危象，需紧急处理")  # 看诊中 → 今天
for item, res in [("血压测量","148/95 mmHg，偏高"),("血常规","正常"),("血脂四项","总胆固醇 6.2 mmol/L，偏高")]:
    add_exam(v_p1_1, item, res)
for item, res in [("空腹血糖","14.5 mmol/L，显著升高"),("糖化血红蛋白","9.8%，控制不佳"),("尿酮体","阳性(++)")]:
    add_exam(v_p1_2, item, res)
for item, res in [("动态血压监测","平均 172/105 mmHg，高血压3级"),("心电图","T波倒置，考虑左心室劳损"),("胸部X线","心影增大，余未见明显异常")]:
    add_exam(v_p1_3, item, res)
for dg, ds, fq in [("DR0001","1粒","1日1次"),("DR0002","1片","1日1次"),("DR0005","1片","1日1次")]:
    add_pr(v_p1_1, "P0001", "D0001", dg, ds, fq)
for dg, ds, fq in [("DR0003","2片","1日2次"),("DR0031","500ml","1日1次"),("DR0032","250ml","1日2次")]:
    add_pr(v_p1_2, "P0001", "D0002", dg, ds, fq)
for dg, ds, fq in [("DR0001","2粒","1日1次"),("DR0002","1片","1日2次"),("DR0005","1片","1日1次"),
                   ("DR0006","1片","1日2次"),("DR0007","1片","1日1次"),("DR0028","1片","舌下含服必要时"),
                   ("DR0031","500ml","1日2次"),("DR0033","1片","1日3次"),("DR0004","1粒","1日1次"),
                   ("DR0034","1粒","1日2次"),("DR0032","500ml","1日1次"),("DR0035","1片","必要时")]:
    add_pr(v_p1_3, "P0001", "D0001", dg, ds, fq)

# D0001 其他名下患者
r_p7 = add_reg("P0007", "D0001", days_ago(15, 10, 0), 1)
r_p15 = add_reg("P0015", "D0001", today_time(11, 0), 1)              # 对应看诊中 → 今天
r_p20 = add_reg("P0020", "D0001", today_time(14, 30), 0)             # 未就诊 → 今天
v_p7 = add_visit(r_p7, days_ago(15, 12, 0), 1, "冠心病稳定性心绞痛")
v_p15 = add_visit(r_p15, today_time(12, 0), 0, "慢性阻塞性肺疾病急性加重")  # 看诊中 → 今天
add_exam(v_p7, "冠脉CTA", "前降支中段 50% 狭窄")
add_exam(v_p15, "肺功能", "FEV1/FVC 58%，重度阻塞")
for dg, ds, fq in [("DR0007","1片","1日1次"),("DR0005","1片","1日1次"),("DR0028","1片","必要时")]:
    add_pr(v_p7, "P0007", "D0001", dg, ds, fq)

# P0001 住院
add_hosp(v_p1_2, "P0001", "W0003", "B0024", days_ago(60, 12, 0), days_ago(53, 10, 0), 1)
add_hosp(v_p1_3, "P0001", "W0001", "B0001", days_ago(7, 12, 0), None, 0)

# ===== 批量填充 =====
_diagnoses_pool = {
    "内科":[("慢性胃炎",[("DR0004","1粒","1日1次")]),("高血压2级",[("DR0001","1粒","1日1次"),("DR0002","1片","1日1次")]),
           ("2型糖尿病",[("DR0003","1片","1日2次")]),("高脂血症",[("DR0005","1片","1日1次")]),
           ("心律失常",[("DR0006","1片","1日2次")]),("冠心病",[("DR0007","1片","1日1次"),("DR0028","1片","必要时")]),
           ("消化道溃疡",[("DR0004","1粒","1日2次"),("DR0033","1片","1日3次")])],
    "外科":[("急性阑尾炎术后",[("DR0010","1粒","1日3次"),("DR0009","1片","1日2次")]),("腹股沟疝",[("DR0008","1片","1日2次")]),
           ("骨关节炎",[("DR0011","1粒","1日1次")]),("外伤术后感染",[("DR0013","1片","1日2次"),("DR0012","适量","外用")]),
           ("胆囊结石",[("DR0009","1片","1日2次")]),("腰椎间盘突出",[("DR0011","1粒","1日1次"),("DR0009","1片","1日2次")])],
    "妇产科":[("先兆流产",[("DR0020","1片","1日2次"),("DR0016","1粒","1日2次")]),("妊娠期糖尿病",[("DR0015","1片","1日1次")]),
             ("围产期保健",[("DR0015","1片","1日1次"),("DR0017","1片","1日1次")]),("多囊卵巢综合征",[("DR0020","1片","1日2次")]),
             ("功能性子宫出血",[("DR0014","1片","1日3次"),("DR0017","1片","1日1次")]),("阴道炎",[("DR0018","1粒","睡前")])],
    "儿科":[("小儿上呼吸道感染",[("DR0021","2滴","发热时"),("DR0023","5ml","1日3次")]),("小儿手足口病",[("DR0022","5ml","1日3次")]),
           ("小儿腹泻",[("DR0026","1包","1日3次"),("DR0025","1包","1日2次")]),("过敏性鼻炎",[("DR0024","5ml","1日1次")]),
           ("小儿哮喘",[("DR0023","5ml","1日2次")]),("小儿发热",[("DR0022","5ml","发热时"),("DR0021","2滴","必要时")])],
    "急诊科":[("急性心肌梗死",[("DR0028","1片","必要时"),("DR0031","500ml","静滴")]),("急性过敏反应",[("DR0027","0.5ml","肌注")]),
             ("癫痫发作",[("DR0030","10mg","必要时")]),("外伤出血",[("DR0014","1片","1日3次")]),
             ("急性胸痛",[("DR0028","1片","舌下含服")])],
}
doctors_by_dept = {}
for did, _, _, dept in doctors_data:
    doctors_by_dept.setdefault(dept, []).append(did)

no_record_patients = {"P0030", "P0031", "P0032", "P0065"}
main_patients = {"P0001", "P0007", "P0015", "P0020"}
other_pids = [p[0] for p in all_patients if p[0] not in main_patients and p[0] not in no_record_patients]
dept_weights = {"内科": 40, "外科": 22, "儿科": 18, "妇产科": 12, "急诊科": 8}

for pid in other_pids:
    age = next(p[3] for p in all_patients if p[0] == pid)
    gender = next(p[2] for p in all_patients if p[0] == pid)
    if age < 16:
        pref = "儿科"
    elif age > 55:
        pref = random.choices(["内科","外科","急诊科"], weights=[5,2,1])[0]
    elif gender == "女" and 18 <= age <= 45:
        pref = random.choices(list(dept_weights.keys()), weights=[40,20,25,10,5])[0]
    else:
        pref = random.choices(list(dept_weights.keys()), weights=list(dept_weights.values()))[0]
    for _ in range(random.choices([1,2,3], weights=[55,35,10])[0]):
        dept = pref if random.random() < 0.7 else random.choice(list(dept_weights.keys()))
        did = random.choice(doctors_by_dept[dept])
        status = random.choices([1,0,2], weights=[80,12,8])[0]

        # 先决定看诊状态(仅当挂号已就诊时有效)
        v_status = 0 if (status == 1 and random.random() < 0.03) else 1

        # 根据业务状态决定时间：未就诊 / 看诊中 都必须发生在今天
        if status == 0:
            # 未就诊 → 今天某时挂号
            reg_time = today_time(random.randint(8, 17), random.choice([0,15,30,45]))
        elif status == 1 and v_status == 0:
            # 已就诊 + 看诊中 → 挂号和看诊都在今天，挂号在前看诊在后
            reg_time = today_time(random.randint(8, 12), random.choice([0,15,30,45]))
        else:
            # 已就诊 + 看诊已完成 / 已取消 → 历史时间
            reg_time = days_ago(random.randint(1, 88), random.randint(8,17), random.choice([0,15,30,45]))

        rid = add_reg(pid, did, reg_time, status)
        if status == 1:
            diag, drugs_list = random.choice(_diagnoses_pool[dept])
            visit_time = reg_time + timedelta(hours=random.randint(1, 3))   # 看诊中时仍在今天范围内
            vid = add_visit(rid, visit_time, v_status, diag)
            if v_status == 1 and random.random() < 0.8:
                add_exam(vid, random.choice(["血常规","尿常规","B超","心电图","X光片","生化全套","CRP"]),
                         random.choice(["未见明显异常","轻度异常","数值偏高","数值偏低","建议复查"]))
            if v_status == 1:
                for dg, ds, fq in drugs_list:
                    add_pr(vid, pid, did, dg, ds, fq)

# 住院池
hosp_patient_pool = [
    # ongoing
    ("P0010","W0002","B0009",12,None,"内科"),
    ("P0021","W0002","B0010",55,None,"内科"),
    ("P0014","W0006","B0046",33,None,"妇产科"),
    ("P0012","W0007","B0056",8,None,"儿科"),
    ("P0025","W0008","B0066",26,None,"综合"),
    ("P0006","W0008","B0067",23,None,"综合"),
    ("P0050","W0001","B0002",5,None,"内科"),
    ("P0080","W0004","B0028",2,None,"外科"),
    ("P0095","W0005","B0040",4,None,"外科"),
    # discharged
    ("P0040","W0001","B0003",85,7,"内科"),
    ("P0045","W0001","B0004",70,5,"内科"),
    ("P0055","W0001","B0005",45,9,"内科"),
    ("P0004","W0002","B0011",88,4,"内科"),
    ("P0035","W0002","B0012",82,6,"内科"),
    ("P0042","W0002","B0013",65,8,"内科"),
    ("P0060","W0002","B0014",55,3,"内科"),
    ("P0070","W0002","B0015",48,12,"内科"),
    ("P0075","W0002","B0009",40,5,"内科"),
    ("P0083","W0002","B0016",30,7,"内科"),
    ("P0091","W0002","B0017",20,4,"内科"),
    ("P0013","W0003","B0025",75,14,"内科"),
    ("P0036","W0003","B0026",35,10,"内科"),
    ("P0004","W0004","B0029",30,8,"外科"),
    ("P0038","W0004","B0030",62,6,"外科"),
    ("P0044","W0004","B0031",50,5,"外科"),
    ("P0057","W0004","B0032",42,10,"外科"),
    ("P0063","W0004","B0033",28,4,"外科"),
    ("P0072","W0004","B0034",18,7,"外科"),
    ("P0047","W0005","B0041",60,3,"外科"),
    ("P0068","W0005","B0042",22,5,"外科"),
    ("P0008","W0006","B0047",38,4,"妇产科"),
    ("P0017","W0006","B0048",28,3,"妇产科"),
    ("P0052","W0006","B0049",65,5,"妇产科"),
    ("P0077","W0006","B0050",15,3,"妇产科"),
    # —— 额外妇产科住院（已出院，床位复用）→ 拉高需求占比触发"扩容"建议 ——
    ("P0026","W0006","B0047",72,5,"妇产科"),
    ("P0022","W0006","B0048",60,4,"妇产科"),
    ("P0064","W0006","B0049",50,3,"妇产科"),
    ("P0076","W0006","B0050",42,4,"妇产科"),
    ("P0089","W0006","B0051",35,6,"妇产科"),
    ("P0097","W0006","B0052",28,5,"妇产科"),
    ("P0102","W0006","B0053",20,3,"妇产科"),
    ("P0106","W0006","B0054",10,4,"妇产科"),
    ("P0110","W0006","B0055", 5,2,"妇产科"),
    ("P0018","W0006","B0046",68,4,"妇产科"),
    ("P0024","W0006","B0047",58,3,"妇产科"),
    ("P0028","W0006","B0048",48,5,"妇产科"),
    ("P0016","W0006","B0049",36,3,"妇产科"),
    ("P0096","W0006","B0050",20,4,"妇产科"),
    ("P0003","W0006","B0046",78,3,"妇产科"),
    ("P0074","W0006","B0047",55,2,"妇产科"),
    ("P0081","W0006","B0048",44,4,"妇产科"),
    ("P0093","W0006","B0049",32,3,"妇产科"),
    ("P0107","W0006","B0050",12,2,"妇产科"),
    ("P0011","W0007","B0057",45,4,"儿科"),
    ("P0019","W0007","B0058",20,3,"儿科"),
    ("P0054","W0007","B0059",30,5,"儿科"),
    ("P0085","W0007","B0060",10,3,"儿科"),
    # —— 额外儿科住院（已出院）→ 拉高需求占比触发"扩容"建议 ——
    ("P0061","W0007","B0057",80,5,"儿科"),
    ("P0073","W0007","B0058",70,4,"儿科"),
    ("P0086","W0007","B0059",62,3,"儿科"),
    ("P0092","W0007","B0060",48,5,"儿科"),
    ("P0100","W0007","B0061",38,4,"儿科"),
    ("P0104","W0007","B0062",25,3,"儿科"),
    ("P0108","W0007","B0063",15,4,"儿科"),
    ("P0109","W0007","B0064", 7,2,"儿科"),
    ("P0067","W0007","B0056",86,3,"儿科"),
    ("P0071","W0007","B0057",72,2,"儿科"),
    ("P0084","W0007","B0058",58,4,"儿科"),
    ("P0090","W0007","B0059",40,3,"儿科"),
    ("P0098","W0007","B0060",28,2,"儿科"),
    ("P0046","W0007","B0056",76,3,"儿科"),
    ("P0051","W0007","B0057",64,4,"儿科"),
    ("P0056","W0007","B0058",52,2,"儿科"),
    ("P0062","W0007","B0059",44,3,"儿科"),
    ("P0069","W0007","B0060",32,4,"儿科"),
    ("P0078","W0007","B0061",22,3,"儿科"),
    ("P0087","W0007","B0062",14,2,"儿科"),
    ("P0015","W0008","B0068",50,15,"综合"),
    ("P0059","W0008","B0069",35,20,"综合"),
    ("P0079","W0008","B0070",20,8,"综合"),
    ("P0082","W0008","B0071",75,35,"综合"),  # 超长期 35 天 → 30+天档
    # extra ongoing to push W0008 VIP near-full (4/6)
    ("P0037","W0008","B0068",6,None,"综合"),
    ("P0048","W0008","B0069",3,None,"综合"),
]
for pid, wid, bid, ad_d, stay, dept in hosp_patient_pool:
    admit_time = days_ago(ad_d, 12, 0)
    disc_time = None if stay is None else admit_time + timedelta(days=stay)
    status = 0 if stay is None else 1
    dept_for = dept if dept in doctors_by_dept else "内科"
    did = random.choice(doctors_by_dept[dept_for])
    reg_time = admit_time - timedelta(hours=random.randint(2, 6))
    rid = add_reg(pid, did, reg_time, 1)
    v_status = 0 if status == 0 else 1
    diag_pool = _diagnoses_pool.get(dept_for, _diagnoses_pool["内科"])
    diag, drugs_list = random.choice(diag_pool)
    diag += "（住院）"
    vid = add_visit(rid, admit_time - timedelta(hours=1), v_status, diag)
    add_exam(vid, "入院检查", "详见病历")
    for dg, ds, fq in drugs_list:
        add_pr(vid, pid, did, dg, ds, fq)
    add_hosp(vid, pid, wid, bid, admit_time, disc_time, status)

# —— 为 DR0029 吗啡注入使用记录（触发库存预警 ⚠ 补货 分档演示）——
# 分析模块只把 prescription_count > 0 的药品纳入预警统计。
# DR0029 没注入就永远不会出现在预警列表里，演示就少一条关键证据。
emergency_doctor_ids = set(doctors_by_dept["急诊科"])
morphine_added = 0
for vid, reg_id, _vtime, v_status, _diag in visits:
    if morphine_added >= 7:   # 7 次 / 3 月 ≈ 2.33/月 → 5/2.33 = 2.14 月 → ⚠ 补货 (< 3)
        break
    if v_status != 1:
        continue
    reg = next((r for r in registrations if r[0] == reg_id), None)
    if reg and reg[2] in emergency_doctor_ids:
        add_pr(vid, reg[1], reg[2], "DR0029", "5mg", "必要时")
        morphine_added += 1
print(f"  为 DR0029 吗啡 注入 {morphine_added} 条处方（触发库存预警演示）")

# 写出业务表
write_file("registrations.txt", [f"{rid}|{p}|{d}|{ts(w)}|{s}" for rid,p,d,w,s in registrations])
write_file("visits.txt", [f"{vid}|{rid}|{ts(w)}|{s}|{diag}" for vid,rid,w,s,diag in visits])
write_file("exams.txt", [f"{eid}|{vid}|{i}|{r}" for eid,vid,i,r in exams])
h_lines = []
for hid, vid, pid, wid, bid, adm, dis, st in hosps:
    h_lines.append(f"{hid}|{vid}|{pid}|{wid}|{bid}|{ts(adm)}|{ts(dis) if dis else 0}|{st}")
write_file("hospitalizations.txt", h_lines)
write_file("prescriptions.txt", [f"{prid}|{v}|{d}|{p}|{drug}|{dose}|{freq}|0"
                                  for prid,v,p,d,drug,dose,freq in prescriptions])

# 床位 + 病房
occupied_beds = {h[4] for h in hosps if h[7] == 0}
bed_lines = []
ward_lines = []
bed_counter = 1
for ward_id, name, wtype, dept, capacity in wards_data:
    occ = 0
    for bed_no in range(1, capacity + 1):
        bid = f"B{bed_counter:04d}"
        st = 1 if bid in occupied_beds else 0
        if st == 1:
            occ += 1
        bed_lines.append(f"{bid}|{ward_id}|{bed_no}|{st}")
        bed_counter += 1
    ward_lines.append(f"{ward_id}|{name}|{wtype}|{dept}|{capacity}|{occ}")
write_file("beds.txt", bed_lines)
write_file("wards.txt", ward_lines)

# 汇总
print()
print("=" * 60)
print("演示数据生成完成！")
print("=" * 60)
unique_in = len({h[2] for h in hosps})
outpatients = len({r[1] for r in registrations})
ongoing = sum(1 for h in hosps if h[7] == 0)
discharged = sum(1 for h in hosps if h[7] == 1)
print(f"患者总数: {len(patient_lines)}  (门诊参与: {outpatients}, 唯一住院患者: {unique_in})")
print(f"医生: {len(doctor_lines)}, 科室: {len(departments)}, 病房: {len(ward_lines)}, 床位: {len(bed_lines)}")
print(f"药品: {len(drug_lines)}, 药房: {len(pharmacies_data)}")
print(f"挂号: {len(registrations)}, 看诊: {len(visits)}, 检查: {len(exams)}")
print(f"住院: {len(hosps)} ({ongoing} 住院中 + {discharged} 已出院), 处方: {len(prescriptions)}")
print()
print("课设要求对照：")
print(f"  门诊患者 ≥100: {outpatients} {'✓' if outpatients >= 100 else '✗'}")
print(f"  住院患者 ≥30:  {unique_in} {'✓' if unique_in >= 30 else '✗'}")
print(f"  医生 ≥20:      {len(doctor_lines)} {'✓' if len(doctor_lines) >= 20 else '✗'}")
print(f"  每科 ≥3 医生: ", {dept: sum(1 for d in doctors_data if d[3]==dept) for dept in departments})
p1_prs = sum(1 for p in prescriptions if p[2] == "P0001")
p1_hosp = sum(1 for h in hosps if h[2] == "P0001")
print(f"\nP0001 主角: {sum(1 for r in registrations if r[1]=='P0001')} 挂号 / {p1_hosp} 住院 / {p1_prs} 处方")
print(f"D0001 主角: {sum(1 for r in registrations if r[2]=='D0001')} 名下挂号")
