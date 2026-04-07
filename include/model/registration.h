/*
 * 挂号预约模块
 */
#ifndef REGISTRATION_H
#define REGISTRATION_H

#include "core/config.h"
#include "core/structs.h"

/*
 * 挂号基础功能
 */
Registration *load_registrations_from_file(void);                                // 从文件中加载挂号数据
int save_registrations_to_file(Registration *head);                              // 将挂号数据保存到文件
void free_registrations(Registration *head);                                     // 释放挂号数据内存
Registration *find_registration_by_r_id(Registration *head, const char *reg_id); // 根据挂号ID查找挂号
Registration *find_registration_by_p_id(Registration *head, const char *p_id);   // 根据患者ID查找挂号
Registration *find_registration_by_d_id(Registration *head, const char *d_id);   // 根据医生ID查找挂号
int generate_next_registration_id(Registration *head);                           // 生成下一个挂号ID
void print_registration(Registration *r, Patient *p_head, Doctor *d_head, int reg_w, int p_w, int d_w, int dept_w,
                        int when_w, int st_w);                                                 // 打印挂号信息行
void print_registration_header(int reg_w, int p_w, int d_w, int dept_w, int when_w, int st_w); // 打印挂号表头
void print_registration_line(int reg_w, int p_w, int d_w, int dept_w, int when_w, int st_w);   // 打印挂号分隔线
void calc_registration_width(Registration *r_head, Patient *p_head, Doctor *d_head, int *reg_w, int *p_w, int *d_w,
                             int *dept_w, int *when_w, int *st_w);                          // 计算挂号表格列宽
int count_registrations_for_doctor(Registration *head, const char *d_id);                   // 统计医生的挂号数量
Registration *get_nth_registration_for_doctor(Registration *head, const char *d_id, int n); // 获取医生的第n个挂号节点

/*
 * 挂号功能函数
 */
Registration create_registration(const char *reg_id, const char *p_id, const char *d_id, time_t when,
                                 int status);                                  // 创建挂号
void append_registration(Registration **head, Registration *new_registration); // 尾插挂号

#endif