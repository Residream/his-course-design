/*
 * 医生功能模块
 */
#ifndef DOCTOR_H
#define DOCTOR_H

#include "core/config.h"
#include "core/structs.h"

/*
 * 医生基础功能
 */
Doctor *load_doctors_from_file(void);                                                  // 从文件中加载医生数据
int save_doctors_to_file(Doctor *head);                                                // 将医生数据保存到文件
void free_doctors(Doctor *head);                                                       // 释放医生数据内存
void print_doctor_align(const char *s, int width);                                     // 打印医生信息并对齐
void print_doctor_line(int id_w, int name_w, int gen_w, int dept_w);                   // 打印医生表格分隔线
void print_doctor_header(int id_w, int name_w, int gen_w, int dept_w);                 // 打印医生表头
Doctor *find_doctor_by_d_id(Doctor *head, const char *id);                             // 根据ID查找医生
int generate_next_doctor_id(Doctor *head);                                             // 生成下一个医生ID(Dxxxx)
void print_doctor(Doctor *d, int id_w, int name_w, int gen_w, int dept_w);             // 打印医生信息行
void calc_doctor_width(Doctor *head, int *id_w, int *name_w, int *gen_w, int *dept_w); // 计算医生信息表格列宽
int count_doctors(Doctor *head);                                                       // 统计医生数量
Doctor *get_nth_doctor(Doctor *head, int n);                                           // 获取第n个医生节点

/*
 * 医生功能函数
 */
Doctor create_doctor(const char *id, const char *name, const char *gender, const char *department,
                     const char *pwd_hash);            // 创建医生
void append_doctor(Doctor **head, Doctor *new_doctor); // 尾插医生

/*
 * 医生系统功能
 */
void add_doctor();                        // 添加医生
void delete_doctor();                     // 删除医生
void update_doctor();                     // 更新医生信息
void query_doctor();                      // 查询医生信息
void show_all_doctors();                  // 显示所有医生信息
void doctor_view_my_info();               // 医生查看个人信息
void doctor_update_my_info();             // 医生修改个人信息
void doctor_view_patient_registrations(); // 医生查看挂号信息

#endif