/*
 * 患者功能模块
 */
#ifndef PATIENT_H
#define PATIENT_H

#include "core/config.h"
#include "core/structs.h"

/*
 * 患者基础操作
 */
Patient *load_patients_from_file(void);                                                 // 从文件中加载患者数据
int save_patients_to_file(Patient *head);                                               // 将患者数据保存到文件
void free_patients(Patient *head);                                                      // 释放患者数据内存
void print_patient_header(int id_w, int name_w, int gen_w, int age_w);                  // 打印患者表头
void print_patient_line(int id_w, int name_w, int gen_w, int age_w);                    // 打印患者表格分隔线
void print_patient_align(const char *s, int width);                                     // 打印对齐的患者信息
Patient *find_patient_by_p_id(Patient *head, const char *id);                           // 根据ID查找患者
int generate_next_patient_id(Patient *head);                                            // 生成下一个患者ID
void print_patient(Patient *p, int id_w, int name_w, int gen_w, int age_w);             // 打印患者信息行
void calc_patient_width(Patient *head, int *id_w, int *name_w, int *gen_w, int *age_w); // 计算患者信息表格列宽
int count_patients(Patient *head);                                                      // 统计患者数量
Patient *get_nth_patient(Patient *head, int n);                                         // 获取第n个患者节点

/*
 * 患者功能函数
 */
Patient create_patient(const char *id, const char *name, const char *gender, int age, const char *pwd_hash); // 创建患者
void append_patient(Patient **head, Patient *new_patient);                                                   // 尾插患者

/*
 *患者系统功能
 */
void add_patient();                             // 添加患者
void delete_patient();                          // 删除患者
void update_patient();                          // 更新患者信息
void query_patient();                           // 查询患者信息
void show_all_patients();                       // 显示所有患者信息
void patient_view_my_info();                    // 患者查看个人信息
void patient_update_my_info();                  // 患者修改个人信息
void patient_registration();                    // 患者挂号预约
void patient_query_my_registrations();          // 查询我的挂号
void patient_cancel_registration();             // 取消挂号预约
void patient_view_my_visits_records();          // 查看我的就诊记录
void patient_view_my_exams_records();           // 查看我的检查记录
void patient_view_my_hospitalization_records(); // 查看我的住院记录
void patient_view_my_prescription_records();    // 查看我的处方记录

#endif