/*
 * 菜单展示模块
 */
#ifndef MENU_H
#define MENU_H

#include "core/config.h"
#include "core/session.h"

#define MENU_WIDTH 36      // 方框内部宽度
#define ITEM_TEXT_WIDTH 20 // 菜单文字区域

/*
 * 菜单展示函数声明
 */
void login_menu(void);        // 登录界面
void patient_pre_menu(void);  // 患者注册登录界面
void patient_main_menu(void); // 患者系统主界面
void doctor_main_menu(void);  // 医护系统主界面
void manager_main_menu(void); // 管理员系统主界面

/*
 *管理员系统子菜单
 */
void manager_patient_menu(void);                            // 患者信息管理
void manager_doctor_and_department_menu(void);              // 医生和科室管理
void manager_doctor_menu(void);                             // 医生管理
void manager_department_menu(void);                         // 科室管理
void manager_ward_and_bed_menu(void);                       // 病房和床位管理
void manager_ward_menu(void);                               // 病房管理
void manager_bed_menu(void);                                // 床位管理
void manager_drug_menu(void);                               // 药品和药房管理
void manager_medical_records_menu(void);                    // 管理员医疗记录管理
void manager_delete_medical_records_by_category_menu(void); // 医疗记录分类删除
void manager_analytics_menu(void);                          // 数据分析

/*
 *患者系统子菜单
 */
void patient_personal_info_menu(void); // 患者个人信息管理
void patient_registration_menu(void);  // 患者挂号预约

/*
 *医护系统子菜单
 */
void doctor_personal_info_menu(void);   // 医生个人信息管理
void doctor_view_patients_menu(void);   // 医生查看患者信息
void doctor_visit_menu(void);           // 医生看诊管理
void doctor_medical_records_menu(void); // 医生医疗记录管理

/*
 *共用子菜单
 */
void query_medical_records_by_category_menu(void); // 分类记录查询
void print_medical_records_by_category(void);      // 分类记录打印

#endif