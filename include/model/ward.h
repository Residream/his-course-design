/*
 * 病房模块
 */
#ifndef WARD_H
#define WARD_H

#include "core/config.h"
#include "core/structs.h"

/*
 * 病房基础功能
 */
Ward *load_wards_from_file(void);                                                             // 从文件中加载病房数据
int save_wards_to_file(Ward *head);                                                           // 保存病房数据到文件
void free_wards(Ward *head);                                                                  // 释放病房链表
Ward *find_ward_by_w_id(Ward *head, const char *ward_id);                                     // 按ID查找病房
Ward *find_ward_by_name(Ward *head, const char *name);                                        // 按名称查找病房
int generate_next_ward_id(Ward *head);                                                        // 生成下一个病房ID
int count_wards(Ward *head);                                                                  // 统计病房数量
Ward *get_nth_ward(Ward *head, int n);                                                        // 获取第n个病房节点
void print_ward(Ward *w, int id_w, int name_w, int type_w, int dept_w, int cap_w, int occ_w); // 打印病房信息行
void print_ward_header(int id_w, int name_w, int type_w, int dept_w, int cap_w, int occ_w);   // 打印病房表头
void print_ward_line(int id_w, int name_w, int type_w, int dept_w, int cap_w, int occ_w);     // 打印病房分隔线
void calc_ward_width(Ward *head, int *id_w, int *name_w, int *type_w, int *dept_w, int *cap_w, int *occ_w); // 计算列宽

/*
 *病房功能函数
 */
Ward create_ward(const char *ward_id, const char *name, const char *type, const char *department, int capacity,
                 int occupied);                // 创建病房
void append_ward(Ward **head, Ward *new_ward); // 尾插病房

/*
 * 病房系统功能
 */
void add_ward();       // 添加病房
void delete_ward();    // 删除病房
void show_all_wards(); // 显示所有病房

#endif