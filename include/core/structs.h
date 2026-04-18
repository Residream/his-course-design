/*
 * 结构体定义模块
 */
#ifndef STRUCTS_H
#define STRUCTS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "core/config.h"

/* 患者结构体 */
typedef struct Patient
{
    char id[MAX_ID_LEN];
    char name[MAX_NAME_LEN];
    char gender[8];
    int age;

    char pwd_hash[MAX_PWD_HASH];
    char salt[SALT_HEX_LEN];

    struct Patient *next;
} Patient;

/* 医生结构体 */
typedef struct Doctor
{
    char id[MAX_ID_LEN];
    char name[MAX_NAME_LEN];
    char gender[8];
    char department[32]; // 科室

    char pwd_hash[MAX_PWD_HASH];
    char salt[SALT_HEX_LEN];

    struct Doctor *next;
} Doctor;

/* 科室结构体 */
typedef struct Department
{
    char name[MAX_NAME_LEN];
    struct Department *next;
} Department;

/* 挂号结构体 */
typedef struct Registration
{
    char reg_id[MAX_ID_LEN];
    char p_id[MAX_ID_LEN];
    char d_id[MAX_ID_LEN];
    time_t when; // 挂号时间
    int status;  // 0=未就诊，1=已就诊, 2=已取消

    struct Registration *next;
} Registration;

/* 看诊结构体 */
typedef struct Visit
{
    char visit_id[MAX_ID_LEN];
    char reg_id[MAX_ID_LEN];
    time_t when;                  // 看诊时间
    int status;                   // 0=看诊中，1=已完成
    char diagnosis[MAX_LINE_LEN]; // 诊断结果

    struct Visit *next;
} Visit;

/* 检查结构体 */
typedef struct Exam
{
    char exam_id[MAX_ID_LEN];
    char visit_id[MAX_ID_LEN];
    char item[MAX_ITEM_LEN];   // 检查项目
    char result[MAX_LINE_LEN]; // 检查结果

    struct Exam *next;
} Exam;

/* 病房结构体 */
typedef struct Ward
{
    char ward_id[MAX_ID_LEN];
    char name[MAX_NAME_LEN];
    char type[MAX_NAME_LEN];       // 病房类型: ICU/普通/VIP
    char department[MAX_NAME_LEN]; // 关联科室
    int capacity;                  // 床位数量
    int occupied;                  // 已占用床位数量

    struct Ward *next;
} Ward;

/* 床位结构体 */
typedef struct Bed
{
    char bed_id[MAX_ID_LEN];
    char ward_id[MAX_ID_LEN];
    int bed_no; // 床位号
    int status; // 0=空闲，1=已占用

    struct Bed *next;
} Bed;

/* 住院结构体 */
typedef struct Hospitalization
{
    char hosp_id[MAX_ID_LEN];
    char visit_id[MAX_ID_LEN];
    char p_id[MAX_ID_LEN];
    char ward_id[MAX_ID_LEN]; // 病房ID
    char bed_id[MAX_ID_LEN];  // 床位ID
    time_t admit_date;        // 入院日期
    time_t discharge_date;    // 出院日期
    int status;               // 0=住院中，1=已出院

    struct Hospitalization *next;
} Hospitalization;

/* 药品结构体 */
typedef struct Drug
{
    char id[MAX_ID_LEN];
    char generic_name[MAX_NAME_LEN];
    char trade_name[MAX_NAME_LEN];
    char alias[MAX_NAME_LEN];
    float price;
    int stock;
    char department[MAX_NAME_LEN];

    struct Drug *next;
} Drug;

/* 药房结构体 */
typedef struct Pharmacy
{
    char id[MAX_ID_LEN];
    char name[MAX_NAME_LEN];
    char location[MAX_NAME_LEN];

    struct Pharmacy *next;
} Pharmacy;

/* 药房药品关联结构体 */
typedef struct PharmacyDrug
{
    char pharmacy_id[MAX_ID_LEN];
    char drug_id[MAX_ID_LEN];
    int quantity;

    struct PharmacyDrug *next;
} PharmacyDrug;

/* 处方结构体 */
typedef struct Prescription
{
    char pr_id[MAX_ID_LEN];
    char visit_id[MAX_ID_LEN];
    char p_id[MAX_ID_LEN];
    char d_id[MAX_ID_LEN];
    char drug_id[MAX_ID_LEN];
    char dose[MAX_DOSE_LEN];      // 剂量
    char frequency[MAX_FREQ_LEN]; // 频次
    int dispensed;                // 0=未发药，1=已发药

    struct Prescription *next;
} Prescription;

#endif