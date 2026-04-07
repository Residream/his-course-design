#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
全流程医院测试数据生成器
"""

import os
import random
import hashlib
from datetime import datetime, timedelta
from pathlib import Path

# ================= 配置区 =================
DATA_DIR = Path(__file__).resolve().parent.parent / "data"
NUM_PATIENTS = 130
NUM_HOSP_PATIENTS = 30
NUM_DOCTORS = 30

DEPARTMENTS = ["内科", "外科", "妇产科", "儿科", "急诊科"]
WARD_NAMES = ["内科一病区", "内科二病区", "外科一病区", "外科二病区", "妇产科病区", "儿科病区", "急诊留观病区", "综合病区"]

# 科室专属药品池 (id, 通用名, 商品名, 别名, 价格, 总库存)
DRUG_DICT = {
    "内科": [
        ("DR0001", "缬沙坦胶囊", "代文", "缬沙坦", 45.00, 1000),
        ("DR0002", "氨氯地平片", "络活喜", "氨氯地平", 33.50, 800),
        ("DR0003", "二甲双胍片", "格华止", "降糖药", 25.00, 1500),
        ("DR0004", "兰索拉唑肠溶片", "达克普隆", "兰索拉唑", 40.00, 600),
        ("DR0005", "阿托伐他汀钙片", "立普妥", "降脂药", 50.00, 900)
    ],
    "外科": [
        ("DR0006", "头孢克肟分散片", "世福素", "头孢", 32.00, 1200),
        ("DR0007", "双氯芬酸钠缓释片", "扶他林", "止痛片", 28.00, 800),
        ("DR0008", "阿莫西林胶囊", "阿莫仙", "阿莫西林", 15.50, 2000),
        ("DR0009", "塞来昔布胶囊", "西乐葆", "塞来昔布", 45.50, 500),
        ("DR0010", "莫匹罗星软膏", "百多邦", "消炎药膏", 18.00, 400)
    ],
    "妇产科": [
        ("DR0011", "叶酸片", "斯利安", "叶酸", 12.00, 1000),
        ("DR0012", "黄体酮胶囊", "益玛欣", "黄体酮", 35.00, 600),
        ("DR0013", "硫酸亚铁片", "速力菲", "铁剂", 22.00, 800),
        ("DR0014", "甲硝唑栓", "达克宁", "甲硝唑", 18.50, 500),
        ("DR0015", "维生素E软胶囊", "来益", "维E", 25.00, 600)
    ],
    "儿科": [
        ("DR0016", "对乙酰氨基酚滴剂", "泰诺林", "退烧药", 20.00, 800),
        ("DR0017", "布洛芬混悬液", "美林", "布洛芬", 25.00, 1000),
        ("DR0018", "盐酸氨溴索溶液", "沐舒坦", "化痰药", 28.00, 600),
        ("DR0019", "氯雷他定糖浆", "开瑞坦", "抗过敏药", 30.00, 500),
        ("DR0020", "枯草杆菌颗粒", "妈咪爱", "益生菌", 35.00, 700)
    ],
    "急诊科": [
        ("DR0021", "盐酸肾上腺素注射液", "肾上腺素", "副肾碱", 15.00, 200),
        ("DR0022", "硝酸甘油片", "硝酸甘油", "硝甘", 12.50, 300),
        ("DR0023", "硫酸吗啡注射液", "吗啡", "吗啡", 55.00, 100)
    ],
    "通用": [
        ("DR0024", "0.9%氯化钠注射液", "生理盐水", "盐水", 5.50, 5000),
        ("DR0025", "5%葡萄糖注射液", "葡萄糖", "糖水", 6.00, 5000),
        ("DR0026", "维生素C片", "维C", "维C", 10.00, 2000),
        ("DR0027", "布洛芬缓释胶囊", "芬必得", "布洛芬", 22.00, 1500),
        ("DR0028", "对乙酰氨基酚片", "泰诺林", "退烧药", 18.00, 1000)
    ]
}

# 药房设定 (ID, 名称, 位置)
PHARMACIES = [
    ("PH0001", "门诊西药房", "门诊楼一楼"),
    ("PH0002", "急诊专属药房", "急诊楼大厅"),
    ("PH0003", "中心住院药房", "住院部二楼")
]

# ================= 辅助函数 =================
FAMILY_NAMES = list("赵钱孙李周吴郑王冯陈褚卫蒋沈韩杨朱秦尤许何吕施张孔曹严华金魏陶姜戚谢邹喻柏水窦章云苏潘葛奚范彭郎鲁韦昌马苗凤花方俞任袁柳刘郭黄林高梁罗宋唐邓萧曾程蔡于董余叶田杜丁江傅钟卢汪戴崔陆廖姚邱夏谭贾石熊孟阎薛侯雷白龙段郝邵史毛常万顾赖武康贺")
MALE_NAMES = ["伟","强","磊","军","洋","勇","杰","涛","明","超","鹏","华","飞","鑫","波","宇","浩","凯","俊","博","志强","建华","浩然","子轩","俊杰","天宇"]
FEMALE_NAMES = ["芳","娜","敏","静","丽","艳","娟","霞","玲","华","丹","萍","雪","琳","婷","倩","颖","洁","晶","璐","子涵","雨桐","欣怡","思雨","梦瑶"]
DIAG_POOL = ["上呼吸道感染", "急性咽炎", "胃炎", "高血压病", "2型糖尿病", "偏头痛", "泌尿系感染", "过敏性鼻炎", "支气管炎", "颈椎病", "冠心病", "脂肪肝", "慢性胃炎", "湿疹", "贫血"]

def random_name(gender: str) -> str:
    family = random.choice(FAMILY_NAMES)
    given = random.choice(MALE_NAMES) if gender == "男" else random.choice(FEMALE_NAMES)
    return family + given

def hash_password(password: str) -> tuple[str, str]:
    salt_raw = os.urandom(16)
    m = hashlib.sha256()
    m.update(salt_raw)
    m.update(password.encode("utf-8"))
    return m.hexdigest(), salt_raw.hex()

def random_past_time(days_ago_min=1, days_ago_max=30) -> datetime:
    now = datetime.now()
    delta = timedelta(days=random.randint(days_ago_min, days_ago_max), 
                      hours=random.randint(0, 23), minutes=random.randint(0, 59))
    return now - delta

# ================= 数据生成逻辑 =================
def generate_all():
    DATA_DIR.mkdir(parents=True, exist_ok=True)
    random.seed(2026) 

    # 1. 基础配置
    root_hash, root_salt = hash_password("root")
    admins = [("root", root_hash, root_salt)]
    departments = [(d,) for d in DEPARTMENTS]

    # 2. 药品、药房与库存关联 (附带 department)
    drugs = []
    pharmacies = PHARMACIES
    pharmacy_drugs = []
    
    for dept, dept_drugs in DRUG_DICT.items():
        for d in dept_drugs:
            # d 是 (id, generic_name, trade_name, alias, price, stock)
            # 拼接科室字段形成新的 tuple: (id, generic_name, ..., stock, department)
            drugs.append(d + (dept,)) 
            drug_id = d[0]
            total_stock = d[5]
            
            # 按科室特性分配库存
            if dept == "急诊科":
                pharmacy_drugs.append(("PH0002", drug_id, total_stock))
            elif dept == "通用":
                # 通用药分配给所有药房
                s1 = int(total_stock * 0.4)
                s2 = int(total_stock * 0.3)
                s3 = total_stock - s1 - s2
                pharmacy_drugs.append(("PH0001", drug_id, s1))
                pharmacy_drugs.append(("PH0002", drug_id, s2))
                pharmacy_drugs.append(("PH0003", drug_id, s3))
            else:
                stock_out = int(total_stock * 0.7)
                stock_in = total_stock - stock_out
                pharmacy_drugs.append(("PH0001", drug_id, stock_out))
                pharmacy_drugs.append(("PH0003", drug_id, stock_in))

    # 3. 生成医生 (Doctor) - 建立 ID 到 科室的映射
    doctors = []
    doc_dept_map = {}
    for i in range(1, NUM_DOCTORS + 1):
        did = f"D{i:04d}"
        gender = random.choice(["男", "女"])
        pwd_hash, salt_hex = hash_password(f"d{i:04d}")
        dept = DEPARTMENTS[i % len(DEPARTMENTS)]
        doctors.append((did, random_name(gender), gender, dept, pwd_hash, salt_hex))
        doc_dept_map[did] = dept

    # 4. 生成患者 (Patient)
    patients = []
    for i in range(1, NUM_PATIENTS + 1):
        pid = f"P{i:04d}"
        gender = random.choice(["男", "女"])
        pwd_hash, salt_hex = hash_password(f"p{i:04d}")
        patients.append((pid, random_name(gender), gender, random.randint(5, 85), pwd_hash, salt_hex))

    # 5. 生成病房与床位 (Ward & Bed)
    wards = []
    beds = []
    bed_seq = 1
    for i, w_name in enumerate(WARD_NAMES, 1):
        wid = f"W{i:04d}"
        capacity = random.randint(10, 20)
        wards.append({"ward_id": wid, "name": w_name, "capacity": capacity, "occupied": 0})
        for b_no in range(1, capacity + 1):
            beds.append({"bed_id": f"B{bed_seq:04d}", "ward_id": wid, "bed_no": b_no, "status": 0})
            bed_seq += 1

    # 6. 生成挂号、看诊、检查、处方
    registrations, visits, exams, prescriptions = [], [], [], []
    reg_seq, visit_seq, exam_seq, pr_seq = 1, 1, 1, 1
    hosp_candidates = {} 

    for p in patients:
        pid = p[0]
        num_regs = random.randint(1, 3)
        for _ in range(num_regs):
            rid = f"R{reg_seq:04d}"
            did = random.choice(doctors)[0]
            doc_dept = doc_dept_map[did] 
            
            reg_dt = random_past_time(1, 30)
            reg_status = random.choices([0, 1, 2], weights=[10, 80, 10])[0]
            registrations.append((rid, pid, did, int(reg_dt.timestamp()), reg_status))
            reg_seq += 1

            if reg_status == 1:
                vid = f"V{visit_seq:04d}"
                visit_dt = reg_dt + timedelta(minutes=random.randint(10, 60))
                
                is_last_30 = int(pid[1:]) > (NUM_PATIENTS - NUM_HOSP_PATIENTS)
                v_status = 1 if is_last_30 else random.choices([0, 1], weights=[30, 70])[0]
                
                diagnosis = random.choice(DIAG_POOL) if v_status == 1 else f"初步考虑：{random.choice(DIAG_POOL)}"
                visits.append((vid, rid, int(visit_dt.timestamp()), v_status, diagnosis))
                visit_seq += 1
                
                if v_status == 1:
                    hosp_candidates.setdefault(pid, []).append((vid, visit_dt))

                    # 检查
                    if random.random() < 0.8:
                        eid = f"E{exam_seq:04d}"
                        item = random.choice(["血常规", "尿常规", "心电图", "胸部X线", "肝功能"])
                        result = random.choice(["未见异常", "轻度异常", "正常"])
                        exams.append((eid, vid, item, result))
                        exam_seq += 1

                    # 处方（专属科室药 + 通用药 混合开具）
                    if random.random() < 0.85: 
                        num_drugs = random.randint(1, 2)
                        
                        # 把该医生的专属科室药和通用药合并成可选池
                        dept_drug_pool = DRUG_DICT.get(doc_dept, []) + DRUG_DICT.get("通用", [])
                        if not dept_drug_pool:
                            dept_drug_pool = DRUG_DICT["内科"] # 兜底
                            
                        chosen_drugs = random.sample(dept_drug_pool, min(num_drugs, len(dept_drug_pool)))
                        
                        for drug in chosen_drugs:
                            prid = f"PR{pr_seq:04d}"
                            drug_id = drug[0]
                            # 根据不同药品类型生成更拟真的用法
                            if "注射液" in drug[1]:
                                dose, freq = "250ml", "静脉滴注"
                            elif "软膏" in drug[1] or "栓" in drug[1]:
                                dose, freq = "适量", "外用1日2次"
                            else:
                                dose, freq = random.choice(["1粒", "2片", "10ml"]), random.choice(["1日3次", "1日2次", "必要时服"])
                                
                            prescriptions.append((prid, vid, did, pid, drug_id, dose, freq))
                            pr_seq += 1

    # 7. 生成住院 
    hospitalizations = []
    hosp_seq = 1
    last_30_pids = [f"P{i:04d}" for i in range(NUM_PATIENTS - NUM_HOSP_PATIENTS + 1, NUM_PATIENTS + 1)]
    free_beds = [b for b in beds if b["status"] == 0]
    random.shuffle(free_beds)

    for pid in last_30_pids:
        candidates = hosp_candidates.get(pid)
        if not candidates or not free_beds: continue
            
        vid, visit_dt = candidates[-1]
        bed = free_beds.pop()
        bed["status"] = 1
        for w in wards:
            if w["ward_id"] == bed["ward_id"]:
                w["occupied"] += 1
                break

        hid = f"H{hosp_seq:04d}"
        admit_dt = visit_dt + timedelta(hours=random.randint(1, 5))
        h_status = random.choices([0, 1], weights=[40, 60])[0]
        if h_status == 1:
            discharge_dt = admit_dt + timedelta(days=random.randint(2, 15))
            if discharge_dt > datetime.now(): discharge_dt = datetime.now() - timedelta(hours=1)
            discharge_ts = int(discharge_dt.timestamp())
            
            bed["status"] = 0
            for w in wards:
                if w["ward_id"] == bed["ward_id"]:
                    w["occupied"] -= 1
                    break
        else:
            discharge_ts = 0

        hospitalizations.append((hid, vid, pid, bed["ward_id"], bed["bed_id"], int(admit_dt.timestamp()), discharge_ts, h_status))
        hosp_seq += 1

    # ================= 统一写入文件模块 =================
    def write_txt(filename, data_list, separator="|"):
        filepath = DATA_DIR / filename
        with open(filepath, "w", encoding="utf-8") as f:
            for row in data_list:
                if isinstance(row, dict):
                    f.write(separator.join(str(v) for v in row.values()) + "\n")
                else:
                    # 针对浮点数价格保留两位小数
                    formatted_row = [f"{x:.2f}" if isinstance(x, float) else str(x) for x in row]
                    f.write(separator.join(formatted_row) + "\n")
        print(f"[{'OK':<4}] {filename:<22} 共生成 {len(data_list):>4} 条记录")

    print("开始生成全关联测试数据...")
    # 全部统一使用 | 分隔符
    write_txt("admins.txt", admins, "|")
    write_txt("departments.txt", departments, "|")
    write_txt("doctors.txt", doctors, "|")
    write_txt("patients.txt", patients, "|")
    write_txt("wards.txt", wards, "|")
    write_txt("beds.txt", beds, "|")
    write_txt("registrations.txt", registrations, "|")
    write_txt("visits.txt", visits, "|")
    write_txt("exams.txt", exams, "|")
    write_txt("prescriptions.txt", prescriptions, "|")
    write_txt("hospitalizations.txt", hospitalizations, "|")
    write_txt("drugs.txt", drugs, "|")
    write_txt("pharmacies.txt", pharmacies, "|")
    write_txt("pharmacy_drugs.txt", pharmacy_drugs, "|")
    
    print("生成完毕！科室与药品开具逻辑、药房库存均已完全适配。")

if __name__ == "__main__":
    generate_all()