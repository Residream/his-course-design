/*
 * 看诊模块
 */
#ifndef VISIT_H
#define VISIT_H

#include "core/config.h"
#include "core/structs.h"

/*
 * 看诊基础功能
 */
Visit *load_visits_from_file(void);                                                 // 从文件中加载看诊数据
int save_visits_to_file(Visit *head);                                               // 将看诊数据保存到文件
void free_visits(Visit *head);                                                      // 释放看诊数据内存
Visit *find_visit_by_v_id(Visit *v_head, const char *v_id);                         // 根据看诊ID查找看诊
Visit *find_visit_by_p_id(Visit *v_head, Registration *reg_head, const char *p_id); // 根据患者ID查找看诊
Visit *find_visit_by_d_id(Visit *v_head, Registration *reg_head, const char *d_id); // 根据医生ID查找看诊
int generate_next_visit_id(Visit *head);                                            // 生成下一个看诊ID
void print_visit(Visit *v, Registration *reg_head, Patient *p_head, Doctor *d_head, int v_w, int p_w, int d_w,
                 int dept_w, int when_w, int st_w, int diag_w);                                   // 打印看诊信息行
void print_visit_header(int v_w, int p_w, int d_w, int dept_w, int when_w, int st_w, int diag_w); // 打印看诊表头
void print_visit_line(int v_w, int p_w, int d_w, int dept_w, int when_w, int st_w, int diag_w);   // 打印看诊分隔线
void calc_visit_width(Visit *v_head, Registration *reg_head, Patient *p_head, Doctor *d_head, int *v_w, int *p_w,
                      int *d_w, int *dept_w, int *when_w, int *st_w, int *diag_w);    // 计算看诊表格列宽
int count_visits(Visit *head);                                                        // 统计看诊数量
Visit *get_nth_visit(Visit *head, int n);                                             // 获取第n个看诊节点
int count_visits_for_doctor(Visit *v_head, Registration *reg_head, const char *d_id); // 统计医生名下的看诊数量
Visit *get_nth_visit_for_doctor(Visit *v_head, Registration *reg_head, const char *d_id,
                                int n); // 获取医生名下的第n个看诊节点

/*
 * 看诊功能函数
 */
Visit create_visit(const char *visit_id, const char *reg_id, time_t when, int status,
                   const char *diagnosis);                  // 创建看诊
void append_visit(Visit **head, Visit *new_visit);          // 尾插看诊
void visit_remove(Visit **head, Visit *target);             // 删除看诊
void update_diagnosis(Visit *visit, const char *diagnosis); // 更新诊断结果

/*
 *看诊系统功能
 */
void doctor_visit_patient(Visit *v_head, const char *v_id, Exam **e_head, Registration *r_head, Patient *p_head,
                          Doctor *d_head); // 医生看诊患者

#endif