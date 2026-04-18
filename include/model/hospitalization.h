/*
 * 住院模块
 */
#ifndef HOSPITALIZATION_H
#define HOSPITALIZATION_H

#include "core/config.h"
#include "core/structs.h"
#include "model/bed.h"
#include "model/patient.h"
#include "model/visit.h"
#include "model/ward.h"

/*
 * 住院基础功能
 */
Hospitalization *load_hospitalizations_from_file(void);                                         // 加载住院数据
int save_hospitalizations_to_file(Hospitalization *head);                                       // 保存住院数据
void free_hospitalizations(Hospitalization *head);                                              // 释放住院链表
Hospitalization *find_hospitalization_by_h_id(Hospitalization *head, const char *hosp_id);      // 按ID查找
Hospitalization *find_hospitalization_by_v_id(Hospitalization *head, const char *visit_id);     // 按看诊ID查找
Hospitalization *find_hospitalization_by_p_id(Hospitalization *head, const char *p_id);         // 根据患者ID查找住院记录
Hospitalization *find_ongoing_hospitalization_by_p_id(Hospitalization *head, const char *p_id); // 查找患者住院中记录
int generate_next_hospitalization_id(Hospitalization *head);                                    // 生成下一个住院ID
void print_hospitalization(Hospitalization *h, Visit *v_head, Registration *r_head, Patient *p_head, Ward *w_head,
                           Bed *b_head, int h_w, int v_w, int p_w, int ward_w, int bed_w, int in_w, int out_w,
                           int st_w); // 打印住院信息行
void print_hospitalization_header(int h_w, int v_w, int p_w, int ward_w, int bed_w, int in_w, int out_w,
                                  int st_w); // 打印住院表头
void print_hospitalization_line(int h_w, int v_w, int p_w, int ward_w, int bed_w, int in_w, int out_w,
                                int st_w); // 打印住院分隔线
void calc_hospitalization_width(Hospitalization *h_head, Visit *v_head, Registration *r_head, Patient *p_head,
                                Ward *w_head, Bed *b_head, int *h_w, int *v_w, int *p_w, int *ward_w, int *bed_w,
                                int *in_w, int *out_w, int *st_w);      // 计算住院表格列宽
int count_hospitalizations(Hospitalization *head);                      // 统计数量
Hospitalization *get_nth_hospitalization(Hospitalization *head, int n); // 获取第n个住院节点
int count_hospitalizations_for_doctor(Hospitalization *h_head, Visit *v_head, Registration *reg_head,
                                      const char *d_id); // 统计医生名下的住院数量
Hospitalization *get_nth_hospitalization_for_doctor(Hospitalization *h_head, Visit *v_head, Registration *reg_head,
                                                    const char *d_id, int n); // 获取医生名下的第n个住院节点

/*
 * 住院功能函数
 */
Hospitalization create_hospitalization(const char *hosp_id, const char *visit_id, const char *p_id, const char *ward_id,
                                       const char *bed_id, time_t admit_date, time_t discharge_date,
                                       int status);                             // 创建住院
void append_hospitalization(Hospitalization **head, Hospitalization *new_hosp); // 尾插住院
void hospitalization_remove(Hospitalization **head, Hospitalization *target);   // 删除住院
int admit_patient(Hospitalization **h_head, Ward *w_head, Bed *b_head, const char *visit_id, const char *p_id,
                  const char *preferred_ward_id);                                               // 入院
int discharge_patient(Hospitalization *h_head, Ward *w_head, Bed *b_head, const char *hosp_id); // 出院

/*
 * 住院系统功能
 */
void doctor_admit_patient_hospitalization(const char *v_id);     // 医生办理住院
void doctor_discharge_patient_hospitalization(const char *v_id); // 医生办理出院

#endif