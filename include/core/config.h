/*
 * 全局配置宏
 */
#ifndef CONFIG_H
#define CONFIG_H

/* 常量定义 */
#define MAX_LINE_LEN 256
#define MAX_ID_LEN 16
#define MAX_NAME_LEN 32
#define MAX_GENDER_LEN 8
#define MAX_PWD_LEN 32
#define MAX_ITEM_LEN 64
#define MAX_DOSE_LEN 32
#define MAX_FREQ_LEN 32
#define MAX_PWD_HASH 65                     // sha256输出的64位16进制字符串+'\0'
#define SALT_RAW_LEN 16                     // 原始盐字节长度
#define SALT_HEX_LEN (SALT_RAW_LEN * 2 + 1) // 十六进制盐字符串长度(32+1)
#define MAX_INPUT_LEN 100                   // 最大输入长度
#define PAGE_SIZE 10                        // 分页显示时每页记录数
#define ROLE_MAX_LEN 16
#define HASH_HEX_LEN 64

/* 数据文件路径 */
#define PATIENTS_FILE "../data/patients.txt"
#define DOCTORS_FILE "../data/doctors.txt"
#define ADMINS_FILE "../data/admins.txt"
#define DEPARTMENTS_FILE "../data/departments.txt"
#define REGISTRATIONS_FILE "../data/registrations.txt"
#define VISITS_FILE "../data/visits.txt"
#define EXAMS_FILE "../data/exams.txt"
#define WARDS_FILE "../data/wards.txt"
#define BEDS_FILE "../data/beds.txt"
#define HOSPITALIZATIONS_FILE "../data/hospitalizations.txt"
#define DRUGS_FILE "../data/drugs.txt"
#define PHARMACIES_FILE "../data/pharmacies.txt"
#define PHARMACY_DRUGS_FILE "../data/pharmacy_drugs.txt"
#define PRESCRIPTIONS_FILE "../data/prescriptions.txt"

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