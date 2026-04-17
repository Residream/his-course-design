/*
 * 医疗记录模块
 */
#ifndef MEDICAL_H
#define MEDICAL_H

#include "core/config.h"
#include "core/structs.h"

/*
 * 医疗记录系统功能
 */
/* 全程查询 */
void query_patients_medical_records(void); // 患者就诊全程查询
/* 分类打印 */
void print_medical_records_by_category(void); // 分类打印医疗记录
/* 分类查询 */
void query_registrations(void);    // 挂号记录查询
void query_visits(void);           // 看诊记录查询
void query_exams(void);            // 检查记录查询
void query_hospitalizations(void); // 住院记录查询
void query_prescriptions(void);    // 处方记录查询
/* 分类删除 */
void delete_registration(void);    // 挂号记录删除
void delete_visit(void);           // 看诊记录删除
void delete_exam(void);            // 检查记录删除
void delete_hospitalization(void); // 住院记录删除
void delete_prescription(void);    // 处方记录删除

#endif