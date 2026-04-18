/*
 * 药品和药房管理模块
 */
#ifndef DRUG_H
#define DRUG_H

#include "core/structs.h"

/*
 * 药品基础操作
 */
Drug *load_drugs_from_file(void);                  // 从文件中加载药品数据
int save_drugs_to_file(Drug *head);                // 将药品数据保存到文件(0成功,-1失败)
void free_drugs(Drug *head);                       // 释放药品数据内存
Drug *find_drug_by_id(Drug *head, const char *id); // 根据ID查找药品
int generate_next_drug_id(Drug *head);             // 生成下一个药品ID
void calc_drug_width(Drug *head, int *id_w, int *gn_w, int *tn_w, int *al_w, int *pr_w, int *st_w,
                     int *dept_w);                                                                // 计算药品表格列宽
void print_drug_line(int id_w, int gn_w, int tn_w, int al_w, int pr_w, int st_w, int dept_w);     // 打印药品表格分隔线
void print_drug_header(int id_w, int gn_w, int tn_w, int al_w, int pr_w, int st_w, int dept_w);   // 打印药品表头
void print_drug(Drug *d, int id_w, int gn_w, int tn_w, int al_w, int pr_w, int st_w, int dept_w); // 打印药品信息行
int count_drugs(Drug *head);                                                                      // 统计药品数量
Drug *get_nth_drug(Drug *head, int n);                                                            // 获取第n个药品节点
void print_drug_selection_hint(void);                                                             // 打印药品选择提示

/*
 * 药房基础操作
 */
Pharmacy *load_pharmacies_from_file(void);                                  // 从文件中加载药房数据
int save_pharmacies_to_file(Pharmacy *head);                                // 将药房数据保存到文件
void free_pharmacies(Pharmacy *head);                                       // 释放药房数据内存
Pharmacy *find_pharmacy_by_id(Pharmacy *head, const char *id);              // 根据ID查找药房
int generate_next_pharmacy_id(Pharmacy *head);                              // 生成下一个药房ID
void calc_pharmacy_width(Pharmacy *head, int *id_w, int *nm_w, int *loc_w); // 计算药房表格列宽
void print_pharmacy_line(int id_w, int nm_w, int loc_w);                    // 打印药房表格分隔线
void print_pharmacy_header(int id_w, int nm_w, int loc_w);                  // 打印药房表头
void print_pharmacy(Pharmacy *p, int id_w, int nm_w, int loc_w);            // 打印药房信息行
int count_pharmacies(Pharmacy *head);                                       // 统计药房数量
Pharmacy *get_nth_pharmacy(Pharmacy *head, int n);                          // 获取第n个药房节点
void print_pharmacy_hint(void);                                             // 打印药房选择提示

/*
 * 药房药品关联操作
 */
PharmacyDrug *load_pharmacy_drugs_from_file(void);   // 从文件中加载药房药品数据
int save_pharmacy_drugs_to_file(PharmacyDrug *head); // 将药房药品数据保存到文件
void free_pharmacy_drugs(PharmacyDrug *head);        // 释放药房药品数据内存

/*
 * 药品系统功能
 */
void add_drug(void);       // 添加药品
void delete_drug(void);    // 删除药品
void update_drug(void);    // 修改药品信息
void query_drug(void);     // 查询药品信息
void show_all_drugs(void); // 显示所有药品

void add_pharmacy(void);        // 添加药房
void delete_pharmacy(void);     // 删除药房
void query_pharmacy(void);      // 查询药房信息
void show_all_pharmacies(void); // 显示所有药房

void stock_in_pharmacy(void);          // 药房入库
void delete_drug_from_pharmacy(void);  // 从药房删除药品
void dispense_prescription_drug(void); // 处方发药
void show_pharmacy_drugs(void);        // 药房库存查询

#endif
