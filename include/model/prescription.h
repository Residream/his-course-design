/*
 * 处方功能模块
 */
#ifndef PRESCRIPTION_H
#define PRESCRIPTION_H

#include "core/config.h"
#include "core/structs.h"

/*
 * 处方基础操作
 */
Prescription *load_prescriptions_from_file(void);                                      // 从文件中加载处方数据
int save_prescriptions_to_file(Prescription *head);                                    // 将处方数据保存
void free_prescriptions(Prescription *head);                                           // 释放处方数据内存
Prescription *find_prescription_by_pr_id(Prescription *head, const char *pr_id);       // 根据处方ID查找处方
Prescription *find_prescription_by_visit_id(Prescription *head, const char *visit_id); // 根据看诊ID查找处方
Prescription *find_prescription_by_p_id(Prescription *head, const char *p_id);         // 根据患者ID查找处方
Prescription *find_prescription_by_d_id(Prescription *head, const char *d_id);         // 根据医生ID查找处方
int generate_next_prescription_id(Prescription *head);                                 // 生成下一个处方ID
void print_prescription(Prescription *pr, Patient *p_head, Doctor *d_head, Drug *drug_head, int pr_w, int visit_w,
                        int d_w, int p_w, int drug_w, int dose_w, int freq_w); // 打印处方信息行
void print_prescription_header(int pr_w, int visit_w, int d_w, int p_w, int drug_w, int dose_w,
                               int freq_w); // 打印处方表头
void print_prescription_line(int pr_w, int visit_w, int d_w, int p_w, int drug_w, int dose_w,
                             int freq_w); // 打印处方分隔线
void calc_prescription_width(Prescription *head, Patient *p_head, Doctor *d_head, Drug *drug_head, int *pr_w,
                             int *visit_w, int *d_w, int *p_w, int *drug_w, int *dose_w,
                             int *freq_w);                                   // 计算处方表格列宽
int count_prescriptions(Prescription *head);                                 // 统计处方数量
Prescription *get_nth_prescription(Prescription *head, int n);               // 获取第n个处方节点
int count_prescriptions_for_doctor(Prescription *pr_head, const char *d_id); // 统计医生名下的处方数量
Prescription *get_nth_prescription_for_doctor(Prescription *pr_head, const char *d_id,
                                              int n); // 获取医生名下的第n个处方节点

/*
 * 处方功能函数
 */
Prescription create_prescription(const char *pr_id, const char *visit_id, const char *p_id, const char *d_id,
                                 const char *drug_id, const char *dose, const char *frequency); // 创建处方
void append_prescription(Prescription **head, Prescription *new_pr);                            // 尾插处方
void prescription_remove(Prescription **head, Prescription *target);                            // 删除处方

#endif