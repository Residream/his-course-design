/*
 * 科室功能模块
 */
#ifndef DEPARTMENT_H
#define DEPARTMENT_H

#include "core/structs.h"

/*
 * 科室基础功能
 */
Department *load_departments_from_file(void);                            // 从文件中加载科室数据
int save_departments_to_file(Department *head);                          // 将科室数据保存到文件
void free_departments(Department *head);                                 // 释放科室数据内存
Department *find_department_by_name(Department *head, const char *name); // 根据名称查找科室
int is_valid_department(const char *dept);                               // 校验科室是否合法

/*
 * 科室功能函数
 */
Department create_department(const char *name);                  // 创建科室
void append_department(Department **head, Department *new_dept); // 尾插科室

/*
 * 科室系统功能
 */
void add_department(void);          // 添加科室
void delete_department(void);       // 删除科室
void show_department_doctors(void); // 显示科室医生
void show_all_departments(void);    // 显示所有科室
void print_all_departments(void);   // 打印所有科室
void print_department_hint(void);   // 打印所有科室可选项

#endif