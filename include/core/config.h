/*
 * 全局配置宏
 */
#ifndef CONFIG_H
#define CONFIG_H

/* 常量定义 */
#define MAX_LINE_LEN 256                    // 通用文本行缓冲区长度
#define MAX_ID_LEN 16                       // 通用ID字符串最大长度
#define MAX_NAME_LEN 32                     // 通用名称字符串最大长度
#define MAX_GENDER_LEN 8                    // 性别字符串最大长度
#define MAX_PWD_LEN 32                      // 明文密码最大长度
#define MAX_ITEM_LEN 64                     // 检查项目等字段最大长度
#define MAX_DOSE_LEN 32                     // 剂量字段最大长度
#define MAX_FREQ_LEN 32                     // 频次字段最大长度
#define MAX_PWD_HASH 65                     // sha256输出的64位16进制字符串+'\0'
#define SALT_RAW_LEN 16                     // 原始盐字节长度
#define SALT_HEX_LEN (SALT_RAW_LEN * 2 + 1) // 十六进制盐字符串长度(32+1)
#define MAX_INPUT_LEN 100                   // 交互输入缓冲区最大长度
#define PAGE_SIZE 10                        // 分页显示时每页记录数
#define ROLE_MAX_LEN 16                     // 会话角色字符串最大长度
#define HASH_HEX_LEN 64                     // sha256十六进制字符串有效长度(不含'\0')

/* 数据文件路径 */
#define PATIENTS_FILE "../data/patients.txt"                 // 患者数据文件
#define DOCTORS_FILE "../data/doctors.txt"                   // 医生数据文件
#define ADMINS_FILE "../data/admins.txt"                     // 管理员数据文件
#define DEPARTMENTS_FILE "../data/departments.txt"           // 科室数据文件
#define REGISTRATIONS_FILE "../data/registrations.txt"       // 挂号数据文件
#define VISITS_FILE "../data/visits.txt"                     // 看诊数据文件
#define EXAMS_FILE "../data/exams.txt"                       // 检查数据文件
#define WARDS_FILE "../data/wards.txt"                       // 病房数据文件
#define BEDS_FILE "../data/beds.txt"                         // 床位数据文件
#define HOSPITALIZATIONS_FILE "../data/hospitalizations.txt" // 住院数据文件
#define DRUGS_FILE "../data/drugs.txt"                       // 药品数据文件
#define PHARMACIES_FILE "../data/pharmacies.txt"             // 药房数据文件
#define PHARMACY_DRUGS_FILE "../data/pharmacy_drugs.txt"     // 药房药品关联数据文件
#define PRESCRIPTIONS_FILE "../data/prescriptions.txt"       // 处方数据文件

/* 挂号状态 */
#define REG_STATUS_PENDING 0                          // 未就诊
#define REG_STATUS_DONE 1                             // 已就诊
#define REG_STATUS_CANCELED 2                         // 已取消
#define REG_STATUS_COUNT 3                            // 挂号状态数量
extern const char *REG_STATUS_TEXT[REG_STATUS_COUNT]; // 挂号状态文本描述数组

/* 看诊状态 */
#define VISIT_STATUS_ONGOING 0                            // 看诊中
#define VISIT_STATUS_DONE 1                               // 已完成
#define VISIT_STATUS_COUNT 2                              // 看诊状态数量
extern const char *VISIT_STATUS_TEXT[VISIT_STATUS_COUNT]; // 看诊状态文本描述数组

/* 床位状态 */
#define BED_STATUS_FREE 0                             // 空闲
#define BED_STATUS_OCCUPIED 1                         // 已占用
#define BED_STATUS_COUNT 2                            // 床位状态数量
extern const char *BED_STATUS_TEXT[BED_STATUS_COUNT]; // 床位状态文本描述数组

/* 住院状态 */
#define HOSP_STATUS_ONGOING 0                           // 住院中
#define HOSP_STATUS_DISCHARGED 1                        // 已出院
#define HOSP_STATUS_COUNT 2                             // 住院状态数量
extern const char *HOSP_STATUS_TEXT[HOSP_STATUS_COUNT]; // 住院状态文本描述数组

#endif
