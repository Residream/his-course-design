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
void login_menu();        // 登录界面
void patient_pre_menu();  // 患者注册登录界面
void patient_main_menu(); // 患者系统主界面
void doctor_main_menu();  // 医护系统主界面
void manager_main_menu(); // 管理员系统主界面

/*
 *管理员系统子菜单
 */
void manager_patient_menu();                 // 患者信息管理
void manager_doctor_and_department_menu();   // 医生和科室管理
void manager_ward_and_bed_menu();            // 病房和床位管理
void manager_drug_menu();                    // 药品和药房管理
void manager_medical_records_menu();         // 医疗记录管理
void manager_doctor_menu();                  // 医生管理
void manager_department_menu();              // 科室管理
void manager_ward_menu();                    // 病房管理
void manager_bed_menu();                     // 床位管理
void manager_exam_records_menu();            // 检查记录管理
void manager_hospitalization_records_menu(); // 住院记录管理
void manager_prescription_records_menu();    // 处方记录管理

/*
 *患者系统子菜单
 */
void patient_personal_info_menu(); // 患者个人信息管理
void patient_registration_menu();  // 患者挂号预约

/*
 *医护系统子菜单
 */
void doctor_personal_info_menu();           // 医生个人信息管理
void doctor_view_patients_menu();           // 医生查看患者信息
void doctor_visit_menu();                   // 医生看诊管理
void doctor_medical_records_menu();         // 医生医疗记录管理
void doctor_exam_records_menu();            // 医生检查记录管理
void doctor_hospitalization_records_menu(); // 医生住院记录管理
void doctor_prescription_records_menu();    // 医生处方记录管理

#endif