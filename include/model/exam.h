/*
 * 检查模块
 */
#ifndef EXAM_H
#define EXAM_H

#include "core/config.h"
#include "core/structs.h"

/*
 * 检查基础功能
 */
Exam *load_exams_from_file(void);                               // 从文件中加载检查数据
int save_exams_to_file(Exam *head);                             // 将检查数据保存到文件
void free_exams(Exam *head);                                    // 释放检查数据内存
Exam *find_exam_by_e_id(Exam *exam_head, const char *exam_id);  // 根据检查ID查找检查
Exam *find_exam_by_v_id(Exam *exam_head, const char *visit_id); // 根据看诊ID查找检查
Exam *find_exam_by_p_id(Exam *exam_head, Visit *visit_head, Registration *reg_head,
                        const char *p_id); // 根据患者ID查找检查
Exam *find_exam_by_d_id(Exam *exam_head, Visit *visit_head, Registration *reg_head,
                        const char *d_id); // 根据医生ID查找检查
int generate_next_exam_id(Exam *head);     // 生成下一个检查ID
void print_exam(Exam *e, Visit *v_head, Registration *r_head, Patient *p_head, Doctor *d_head, int exam_w, int visit_w,
                int p_w, int d_w, int dept_w, int item_w, int result_w);                                 // 打印检查信息行
void print_exam_header(int exam_w, int visit_w, int p_w, int d_w, int dept_w, int item_w, int result_w); // 打印检查表头
void print_exam_line(int exam_w, int visit_w, int p_w, int d_w, int dept_w, int item_w, int result_w);   // 打印检查分隔线
void calc_exam_width(Exam *e_head, Visit *v_head, Registration *r_head, Patient *p_head, Doctor *d_head, int *exam_w,
                     int *visit_w, int *p_w, int *d_w, int *dept_w, int *item_w, int *result_w); // 计算检查表格列宽
int count_exams(Exam *head);                                                                     // 统计检查数量
Exam *get_nth_exam(Exam *head, int n);                                                           // 获取第n个检查节点
int count_exams_for_doctor(Exam *e_head, Visit *v_head, Registration *reg_head,
                           const char *d_id); // 统计医生名下的检查数量
Exam *get_nth_exam_for_doctor(Exam *e_head, Visit *v_head, Registration *reg_head, const char *d_id,
                              int n); // 获取医生名下的第n个检查节点

/*
 *检查功能函数
 */
Exam create_exam(const char *exam_id, const char *visit_id, const char *item, const char *result); // 创建检查
void append_exam(Exam **head, Exam *new_exam);                                                     // 尾插检查
void exam_remove(Exam **head, Exam *target);                                                       // 删除检查

#endif