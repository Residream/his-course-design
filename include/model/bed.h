/*
 * 床位模块
 */
#ifndef BED_H
#define BED_H

#include "core/config.h"
#include "core/structs.h"
#include "model/ward.h"

/*
 * 床位基础功能
 */
Bed *load_beds_from_file(void);                                                               // 从文件中加载床位数据
int save_beds_to_file(Bed *head);                                                             // 保存床位数据到文件
void free_beds(Bed *head);                                                                    // 释放床位链表
Bed *find_bed_by_b_id(Bed *head, const char *bed_id);                                         // 按ID查找床位
Bed *find_first_free_bed(Bed *head);                                                          // 查找首个空闲床位
Bed *find_first_free_bed_in_ward(Bed *head, const char *ward_id);                             // 查找病房内首个空闲床位
int generate_next_bed_id(Bed *head);                                                          // 生成下一个床位ID
int count_beds(Bed *head);                                                                    // 统计床位数量
int count_free_beds(Bed *head);                                                               // 统计空闲床位数
void print_bed(Bed *b, Ward *w_head, int id_w, int ward_w, int no_w, int st_w);               // 打印床位信息行
void print_free_bed(Bed *b, Ward *w_head, int id_w, int ward_w, int no_w, int st_w);          // 打印空闲床位信息行
void print_bed_header(int id_w, int ward_w, int no_w, int st_w);                              // 打印床位表头
void print_bed_line(int id_w, int ward_w, int no_w, int st_w);                                // 打印床位分隔线
void calc_bed_width(Bed *b_head, Ward *w_head, int *id_w, int *ward_w, int *no_w, int *st_w); // 计算列宽
Bed *get_nth_bed(Bed *head, int n);                                                           // 获取第n个床位节点

/*
 *床位功能函数
 */
Bed create_bed(const char *bed_id, const char *ward_id, int bed_no, int status); // 创建床位
void append_bed(Bed **head, Bed *new_bed);                                       // 尾插床位

/*
 * 床位业务函数
 */
void add_bed();       // 添加床位
void delete_bed();    // 删除床位
void show_all_beds(); // 显示所有床位

#endif