/*
 * 菜单展示模块
 */
#include "ui/menu.h"
#include "core/auth.h"
#include "core/session.h"
#include "core/sha256.h"
#include "core/structs.h"
#include "core/utils.h"
#include "model/analytics.h"
#include "model/bed.h"
#include "model/department.h"
#include "model/doctor.h"
#include "model/drug.h"
#include "model/exam.h"
#include "model/hospitalization.h"
#include "model/patient.h"
#include "model/prescription.h"
#include "model/registration.h"
#include "model/visit.h"
#include "model/ward.h"

/*
 * 菜单工具函数实现
 */

/* 顶部边框 */
static void menu_top()
{
    printf("╔");
    for (int i = 0; i < MENU_WIDTH; i++)
        printf("═");
    printf("╗\n");
}

/* 底部边框 */
static void menu_bottom()
{
    printf("╚");
    for (int i = 0; i < MENU_WIDTH; i++)
        printf("═");
    printf("╝\n");
}

/* 中间分隔线 */
static void menu_split()
{
    printf("╠");
    for (int i = 0; i < MENU_WIDTH; i++)
        printf("═");
    printf("╣\n");
}

/* 空行 */
static void menu_blank()
{
    printf("║");
    for (int i = 0; i < MENU_WIDTH; i++)
        printf(" ");
    printf("║\n");
}

/* 居中标题 */
static void menu_title(const char *text)
{
    int len = str_width(text);

    int left = (MENU_WIDTH - len) / 2;
    int right = MENU_WIDTH - len - left;

    printf("║");

    for (int i = 0; i < left; i++)
        printf(" ");

    printf("%s", text);

    for (int i = 0; i < right; i++)
        printf(" ");

    printf("║\n");
}

/* 菜单项 */
static void menu_item(int num, const char *text)
{
    char buf[256];

    snprintf(buf, sizeof(buf), "%2d. %s", num, text);

    int len = str_width(buf);

    int left = (MENU_WIDTH - len) / 2;
    int right = MENU_WIDTH - len - left;

    printf("║");

    for (int i = 0; i < left; i++)
        printf(" ");

    printf("%s", buf);

    for (int i = 0; i < right; i++)
        printf(" ");

    printf("║\n");
}

/*
 * 菜单展示函数实现
 */

/* 登录界面 */
void login_menu()
{
    char buf[MAX_INPUT_LEN];
    int select;

    while (1)
    {
        clear_screen();
        menu_top();
        menu_title("医疗管理系统");
        menu_split();
        menu_item(1, "患者系统");
        menu_item(2, "医护系统");
        menu_item(3, "管理员系统");
        menu_blank();
        menu_item(0, "退出系统");
        menu_bottom();

        printf("请输入您的选择: ");
        safe_input(buf, MAX_INPUT_LEN);

        if (!validate_choice(buf, 3))
        {
            printf("输入有误，请重新选择！\n");
            wait_enter();
            clear_screen();
            continue;
        }

        select = atoi(buf);

        switch (select)
        {
        case 1: {
            patient_pre_menu();
            break;
        }
        case 2: {
            char id[MAX_ID_LEN];
            printf("请输入医生ID(输入0返回): ");
            safe_input(id, sizeof(id));
            if (strcmp(id, "0") == 0)
            {
                clear_screen();
                break;
            }
            char password[MAX_INPUT_LEN];
            printf("请输入医生密码(输入0返回): ");
            safe_input(password, sizeof(password));
            if (strcmp(password, "0") == 0)
            {
                clear_screen();
                break;
            }
            if (doctor_login_by_file(DOCTORS_FILE, id, password))
            {
                session_set("doctor", id);
                printf("登录成功！\n");
                wait_enter();
                clear_screen();
                doctor_main_menu();
            }
            else
            {
                printf("登录失败！\n");
                wait_enter();
                clear_screen();
            }
            break;
        }
        case 3: {
            char name[MAX_NAME_LEN];
            printf("请输入管理员用户名(输入0返回): ");
            safe_input(name, sizeof(name));
            if (strcmp(name, "0") == 0)
            {
                clear_screen();
                break;
            }
            char password[MAX_INPUT_LEN];
            printf("请输入管理员密码(输入0返回): ");
            safe_input(password, sizeof(password));
            if (strcmp(password, "0") == 0)
            {
                clear_screen();
                break;
            }
            if (admin_login_by_file(ADMINS_FILE, name, password))
            {
                printf("登录成功！\n");
                session_set("admin", name);
                wait_enter();
                clear_screen();
                manager_main_menu();
            }
            else
            {
                printf("登录失败！\n");
                wait_enter();
                clear_screen();
            }
            break;
        }
        case 0:
            printf("欢迎下次使用\n");
            wait_enter();
            clear_screen();
            return;
        default:
            printf("未知错误: %d\n", select);
            break;
        }
    }
}

/* 患者注册登录界面 */
void patient_pre_menu()
{
    char buf[MAX_INPUT_LEN];
    int select;

    while (1)
    {
        clear_screen();
        menu_top();
        menu_title("患者系统");
        menu_split();
        menu_item(1, "注册");
        menu_item(2, "登录");
        menu_blank();
        menu_item(0, "返回上级");
        menu_bottom();

        printf("请输入您的选择: ");
        safe_input(buf, MAX_INPUT_LEN);

        if (!validate_choice(buf, 2))
        {
            printf("输入有误，请重新选择！\n");
            wait_enter();
            clear_screen();
            continue;
        }

        select = atoi(buf);

        switch (select)
        {
        case 1: {
            patient_register();
            break;
        }
        case 2: {
            char id[MAX_ID_LEN];
            printf("请输入患者ID(输入0返回): ");
            safe_input(id, sizeof(id));
            if (strcmp(id, "0") == 0)
            {
                clear_screen();
                break;
            }
            char password[MAX_INPUT_LEN];
            printf("请输入患者密码(输入0返回): ");
            safe_input(password, sizeof(password));
            if (strcmp(password, "0") == 0)
            {
                clear_screen();
                break;
            }
            if (patient_login_by_file(PATIENTS_FILE, id, password))
            {
                session_set("patient", id);
                printf("登录成功！\n");
                wait_enter();
                clear_screen();
                patient_main_menu();
            }
            else
            {
                printf("登录失败！\n");
                wait_enter();
                clear_screen();
            }
            break;
        }
        case 0:
            clear_screen();
            return;
        default:
            printf("未知错误: %d\n", select);
            break;
        }
    }
}

/* 患者系统主界面 */
void patient_main_menu()
{
    char buf[MAX_INPUT_LEN];
    int select;

    while (1)
    {
        clear_screen();
        menu_top();
        menu_title("患者系统");
        menu_split();
        menu_item(1, "个人信息管理");
        menu_item(2, "挂号预约");
        menu_item(3, "查看看诊记录");
        menu_item(4, "查看检查记录");
        menu_item(5, "查看住院记录");
        menu_item(6, "查看处方记录");
        menu_blank();
        menu_item(0, "退出系统");
        menu_bottom();

        printf("请输入您的选择: ");
        safe_input(buf, MAX_INPUT_LEN);

        if (!validate_choice(buf, 6))
        {
            printf("输入有误，请重新选择！\n");
            wait_enter();
            clear_screen();
            continue;
        }

        select = atoi(buf);

        switch (select)
        {
        case 1:
            patient_personal_info_menu();
            break;
        case 2:
            patient_registration_menu();
            break;
        case 3:
            patient_view_my_visits_records();
            break;
        case 4:
            patient_view_my_exams_records();
            break;
        case 5:
            patient_view_my_hospitalization_records();
            break;
        case 6:
            patient_view_my_prescription_records();
            break;
        case 0:
            session_clear();
            clear_screen();
            return;
        default:
            printf("未知错误: %d\n", select);
            break;
        }
    }
}

/* 医护系统主界面 */
void doctor_main_menu()
{
    char buf[MAX_INPUT_LEN];
    int select;

    while (1)
    {
        clear_screen();
        menu_top();
        menu_title("医护系统");
        menu_split();
        menu_item(1, "个人信息管理");
        menu_item(2, "查看患者信息");
        menu_item(3, "查看挂号信息");
        menu_item(4, "看诊管理");
        menu_item(5, "医疗记录管理");
        menu_item(6, "处方发药");
        menu_blank();
        menu_item(0, "退出系统");
        menu_bottom();

        printf("请输入您的选择: ");
        safe_input(buf, MAX_INPUT_LEN);

        if (!validate_choice(buf, 6))
        {
            printf("输入有误，请重新选择！\n");
            wait_enter();
            clear_screen();
            continue;
        }

        select = atoi(buf);

        switch (select)
        {
        case 1:
            doctor_personal_info_menu();
            break;
        case 2:
            doctor_view_patients_menu();
            break;
        case 3:
            doctor_view_patient_registrations();
            break;
        case 4:
            doctor_visit_menu();
            break;
        case 5:
            doctor_medical_records_menu();
            break;
        case 6:
            dispense_prescription_drug();
            break;
        case 0:
            session_clear();
            clear_screen();
            return;
        default:
            printf("未知错误: %d\n", select);
            break;
        }
    }
}

/* 管理员系统主界面 */
void manager_main_menu()
{
    char buf[MAX_INPUT_LEN];
    int select;

    while (1)
    {
        clear_screen();
        menu_top();
        menu_title("管理员系统");
        menu_split();
        menu_item(1, "患者信息管理");
        menu_item(2, "医生和科室管理");
        menu_item(3, "病房和床位管理");
        menu_item(4, "药品和药房管理");
        menu_item(5, "医疗记录管理");
        menu_item(6, "数据分析");
        menu_blank();
        menu_item(0, "退出系统");
        menu_bottom();

        printf("请输入您的选择: ");
        safe_input(buf, MAX_INPUT_LEN);

        if (!validate_choice(buf, 6))
        {
            printf("输入有误，请重新选择！\n");
            wait_enter();
            clear_screen();
            continue;
        }

        select = atoi(buf);

        switch (select)
        {
        case 1:
            manager_patient_menu();
            break;
        case 2:
            manager_doctor_and_department_menu();
            break;
        case 3:
            manager_ward_and_bed_menu();
            break;
        case 4:
            manager_drug_menu();
            break;
        case 5:
            manager_medical_records_menu();
            break;
        case 6:
            manager_analytics_menu();
            break;
        case 0:
            session_clear();
            clear_screen();
            return;
        default:
            printf("未知错误: %d\n", select);
            break;
        }
    }
}

/*
 *管理员系统子菜单
 */

/* 患者信息管理 */
void manager_patient_menu()
{
    char buf[MAX_INPUT_LEN];
    int select;
    while (1)
    {
        clear_screen();
        menu_top();
        menu_title("患者信息管理");
        menu_split();
        menu_item(1, "添加患者");
        menu_item(2, "删除患者");
        menu_item(3, "修改患者信息");
        menu_item(4, "查询患者信息");
        menu_item(5, "展示所有患者");
        menu_blank();
        menu_item(0, "返回上级菜单");
        menu_bottom();

        printf("请输入您的选择: ");
        safe_input(buf, MAX_INPUT_LEN);

        if (!validate_choice(buf, 5))
        {
            printf("输入有误，请重新选择！\n");
            wait_enter();
            clear_screen();
            continue;
        }

        select = atoi(buf);

        switch (select)
        {
        case 1:
            add_patient();
            break;
        case 2:
            delete_patient();
            break;
        case 3:
            update_patient();
            break;
        case 4:
            query_patient();
            break;
        case 5:
            show_all_patients();
            break;
        case 0:
            clear_screen();
            return;
        default:
            printf("未知错误: %d\n", select);
            break;
        }
    }
}

/* 医生和科室管理 */
void manager_doctor_and_department_menu()
{
    char buf[MAX_INPUT_LEN];
    int select;
    while (1)
    {
        clear_screen();
        menu_top();
        menu_title("医生和科室管理");
        menu_split();
        menu_item(1, "医生管理");
        menu_item(2, "科室管理");
        menu_blank();
        menu_item(0, "返回上级菜单");
        menu_bottom();

        printf("请输入您的选择: ");
        safe_input(buf, MAX_INPUT_LEN);

        if (!validate_choice(buf, 2))
        {
            printf("输入有误，请重新选择！\n");
            wait_enter();
            clear_screen();
            continue;
        }

        select = atoi(buf);

        switch (select)
        {
        case 1:
            manager_doctor_menu();
            break;
        case 2:
            manager_department_menu();
            break;
        case 0:
            clear_screen();
            return;
        default:
            printf("未知错误: %d\n", select);
            break;
        }
    }
}

/* 病房和床位管理 */
void manager_ward_and_bed_menu()
{
    char buf[MAX_INPUT_LEN];
    int select;
    while (1)
    {
        clear_screen();
        menu_top();
        menu_title("病房和床位管理");
        menu_split();
        menu_item(1, "病房管理");
        menu_item(2, "床位管理");
        menu_blank();
        menu_item(0, "返回上级菜单");
        menu_bottom();

        printf("请输入您的选择: ");
        safe_input(buf, MAX_INPUT_LEN);

        if (!validate_choice(buf, 2))
        {
            printf("输入有误，请重新选择！\n");
            wait_enter();
            clear_screen();
            continue;
        }

        select = atoi(buf);

        switch (select)
        {
        case 1:
            manager_ward_menu();
            break;
        case 2:
            manager_bed_menu();
            break;
        case 0:
            clear_screen();
            return;
        default:
            printf("未知错误: %d\n", select);
            break;
        }
    }
}

/* 药品和药房管理 */
void manager_drug_menu()
{
    char buf[MAX_INPUT_LEN];
    int select;

    while (1)
    {
        clear_screen();
        menu_top();
        menu_title("药品和药房管理");
        menu_split();
        menu_item(1, "药品管理");
        menu_item(2, "药房管理");
        menu_item(3, "药房药品管理");
        menu_blank();
        menu_item(0, "返回上级菜单");
        menu_bottom();

        printf("请输入您的选择: ");
        safe_input(buf, MAX_INPUT_LEN);

        if (!validate_choice(buf, 3))
        {
            printf("输入有误，请重新选择！\n");
            wait_enter();
            clear_screen();
            continue;
        }

        select = atoi(buf);

        switch (select)
        {
        case 1: {
            // 药品管理子菜单
            while (1)
            {
                clear_screen();
                menu_top();
                menu_title("药品管理");
                menu_split();
                menu_item(1, "添加药品");
                menu_item(2, "删除药品");
                menu_item(3, "修改药品信息");
                menu_item(4, "查询药品信息");
                menu_item(5, "展示所有药品");
                menu_blank();
                menu_item(0, "返回上级菜单");
                menu_bottom();

                printf("请输入您的选择: ");
                safe_input(buf, sizeof(buf));

                if (!validate_choice(buf, 5))
                {
                    printf("输入有误，请重新选择！\n");
                    wait_enter();
                    clear_screen();
                    continue;
                }

                int drug_select = atoi(buf);

                switch (drug_select)
                {
                case 1:
                    add_drug();
                    break;
                case 2:
                    delete_drug();
                    break;
                case 3:
                    update_drug();
                    break;
                case 4:
                    query_drug();
                    break;
                case 5:
                    show_all_drugs();
                    break;
                case 0:
                    clear_screen();
                    goto drug_menu_end;
                default:
                    printf("未知错误: %d\n", drug_select);
                    break;
                }
            }
        drug_menu_end:
            break;
        }
        case 2: {
            // 药房管理子菜单
            while (1)
            {
                clear_screen();
                menu_top();
                menu_title("药房管理");
                menu_split();
                menu_item(1, "添加药房");
                menu_item(2, "删除药房");
                menu_item(3, "查询药房信息");
                menu_item(4, "展示所有药房");
                menu_blank();
                menu_item(0, "返回上级菜单");
                menu_bottom();

                printf("请输入您的选择: ");
                safe_input(buf, sizeof(buf));

                if (!validate_choice(buf, 4))
                {
                    printf("输入有误，请重新选择！\n");
                    wait_enter();
                    clear_screen();
                    continue;
                }

                int pharmacy_select = atoi(buf);

                switch (pharmacy_select)
                {
                case 1:
                    add_pharmacy();
                    break;
                case 2:
                    delete_pharmacy();
                    break;
                case 3:
                    query_pharmacy();
                    break;
                case 4:
                    show_all_pharmacies();
                    break;
                case 0:
                    clear_screen();
                    goto pharmacy_menu_end;
                default:
                    printf("未知错误: %d\n", pharmacy_select);
                    break;
                }
            }
        pharmacy_menu_end:
            break;
        }
        case 3: {
            // 药房药品管理子菜单
            while (1)
            {
                clear_screen();
                menu_top();
                menu_title("药房药品管理");
                menu_split();
                menu_item(1, "向药房添加药品");
                menu_item(2, "查询药房中的药品");
                menu_item(3, "处方发药");
                menu_blank();
                menu_item(0, "返回上级菜单");
                menu_bottom();

                printf("请输入您的选择: ");
                safe_input(buf, sizeof(buf));

                if (!validate_choice(buf, 3))
                {
                    printf("输入有误，请重新选择！\n");
                    wait_enter();
                    clear_screen();
                    continue;
                }

                int pd_select = atoi(buf);

                switch (pd_select)
                {
                case 1:
                    stock_in_pharmacy();
                    break;
                case 2:
                    show_pharmacy_drugs();
                    break;
                case 3:
                    dispense_prescription_drug();
                    break;
                case 0:
                    clear_screen();
                    goto pd_menu_end;
                default:
                    printf("未知错误: %d\n", pd_select);
                    break;
                }
            }
        pd_menu_end:
            break;
        }
        case 0:
            clear_screen();
            return;
        default:
            printf("未知错误: %d\n", select);
            break;
        }
    }
}

/* 医疗记录管理 */
void manager_medical_records_menu()
{
    char buf[MAX_INPUT_LEN];
    int select;
    while (1)
    {
        clear_screen();
        menu_top();
        menu_title("医疗记录管理");
        menu_split();
        menu_item(1, "检查记录管理");
        menu_item(2, "住院记录管理");
        menu_item(3, "处方记录管理");
        menu_blank();
        menu_item(0, "返回上级菜单");
        menu_bottom();

        printf("请输入您的选择: ");
        safe_input(buf, MAX_INPUT_LEN);

        if (!validate_choice(buf, 3))
        {
            printf("输入有误，请重新选择！\n");
            wait_enter();
            clear_screen();
            continue;
        }

        select = atoi(buf);

        switch (select)
        {
        case 1:
            manager_exam_records_menu();
            break;
        case 2:
            manager_hospitalization_records_menu();
            break;
        case 3:
            manager_prescription_records_menu();
            break;
        case 0:
            clear_screen();
            return;
        default:
            printf("未知错误: %d\n", select);
            break;
        }
    }
}

/* 医生管理 */
void manager_doctor_menu()
{
    char buf[MAX_INPUT_LEN];
    int select;
    while (1)
    {
        clear_screen();
        menu_top();
        menu_title("医生管理");
        menu_split();
        menu_item(1, "添加医生");
        menu_item(2, "删除医生");
        menu_item(3, "修改医生信息");
        menu_item(4, "查询医生信息");
        menu_item(5, "展示所有医生");
        menu_blank();
        menu_item(0, "返回上级菜单");
        menu_bottom();

        printf("请输入您的选择: ");
        safe_input(buf, MAX_INPUT_LEN);

        if (!validate_choice(buf, 5))
        {
            printf("输入有误，请重新选择！\n");
            wait_enter();
            clear_screen();
            continue;
        }

        select = atoi(buf);

        switch (select)
        {
        case 1:
            add_doctor();
            break;
        case 2:
            delete_doctor();
            break;
        case 3:
            update_doctor();
            break;
        case 4:
            query_doctor();
            break;
        case 5:
            show_all_doctors();
            break;
        case 0:
            clear_screen();
            return;
        default:
            printf("未知错误: %d\n", select);
            break;
        }
    }
}

/* 科室管理 */
void manager_department_menu()
{
    char buf[MAX_INPUT_LEN];
    int select;
    while (1)
    {
        clear_screen();
        menu_top();
        menu_title("科室管理");
        menu_split();
        menu_item(1, "添加科室");
        menu_item(2, "删除科室");
        menu_item(3, "展示科室医生");
        menu_item(4, "展示所有科室");
        menu_blank();
        menu_item(0, "返回上级菜单");
        menu_bottom();

        printf("请输入您的选择: ");
        safe_input(buf, MAX_INPUT_LEN);

        if (!validate_choice(buf, 4))
        {
            printf("输入有误，请重新选择！\n");
            wait_enter();
            clear_screen();
            continue;
        }

        select = atoi(buf);

        switch (select)
        {
        case 1:
            add_department();
            break;
        case 2:
            delete_department();
            break;
        case 3:
            show_department_doctors();
            break;
        case 4:
            show_all_departments();
            break;
        case 0:
            clear_screen();
            return;
        default:
            printf("未知错误: %d\n", select);
            break;
        }
    }
}

/* 病房管理 */
void manager_ward_menu()
{
    char buf[MAX_INPUT_LEN];
    int select;
    while (1)
    {
        clear_screen();
        menu_top();
        menu_title("病房管理");
        menu_split();
        menu_item(1, "添加病房");
        menu_item(2, "删除病房");
        menu_item(3, "展示所有病房");
        menu_blank();
        menu_item(0, "返回上级菜单");
        menu_bottom();

        printf("请输入您的选择: ");
        safe_input(buf, MAX_INPUT_LEN);

        if (!validate_choice(buf, 3))
        {
            printf("输入有误，请重新选择！\n");
            wait_enter();
            clear_screen();
            continue;
        }

        select = atoi(buf);

        switch (select)
        {
        case 1:
            add_ward();
            break;
        case 2:
            delete_ward();
            break;
        case 3:
            show_all_wards();
            break;
        case 0:
            clear_screen();
            return;
        default:
            printf("未知错误: %d\n", select);
            break;
        }
    }
}

/* 床位管理 */
void manager_bed_menu()
{
    char buf[MAX_INPUT_LEN];
    int select;
    while (1)
    {
        clear_screen();
        menu_top();
        menu_title("床位管理");
        menu_split();
        menu_item(1, "添加床位");
        menu_item(2, "删除床位");
        menu_item(3, "展示所有床位");
        menu_blank();
        menu_item(0, "返回上级菜单");
        menu_bottom();

        printf("请输入您的选择: ");
        safe_input(buf, MAX_INPUT_LEN);

        if (!validate_choice(buf, 3))
        {
            printf("输入有误，请重新选择！\n");
            wait_enter();
            clear_screen();
            continue;
        }

        select = atoi(buf);

        switch (select)
        {
        case 1:
            add_bed();
            break;
        case 2:
            delete_bed();
            break;
        case 3:
            show_all_beds();
            break;
        case 0:
            clear_screen();
            return;
        default:
            printf("未知错误: %d\n", select);
            break;
        }
    }
}

/* 检查记录管理 */
void manager_exam_records_menu()
{
    char buf[MAX_INPUT_LEN];
    int select;
    while (1)
    {
        clear_screen();
        menu_top();
        menu_title("检查记录管理");
        menu_split();
        menu_item(1, "添加检查记录");
        menu_item(2, "删除检查记录");
        menu_item(3, "修改检查记录");
        menu_item(4, "查询检查记录");
        menu_item(5, "显示所有检查记录");
        menu_blank();
        menu_item(0, "返回上级菜单");
        menu_bottom();

        printf("请输入您的选择: ");
        safe_input(buf, MAX_INPUT_LEN);

        if (!validate_choice(buf, 5))
        {
            printf("输入有误，请重新选择！\n");
            wait_enter();
            clear_screen();
            continue;
        }

        select = atoi(buf);

        switch (select)
        {
        case 1:
            add_exam_record();
            break;
        case 2:
            delete_exam_record();
            break;
        case 3:
            update_exam_record();
            break;
        case 4:
            query_exam_record();
            break;
        case 5:
            show_all_exam_records();
            break;
        case 0:
            clear_screen();
            return;
        default:
            printf("未知错误: %d\n", select);
            break;
        }
    }
}

/* 住院记录管理 */
void manager_hospitalization_records_menu()
{
    char buf[MAX_INPUT_LEN];
    int select;
    while (1)
    {
        clear_screen();
        menu_top();
        menu_title("住院记录管理");
        menu_split();
        menu_item(1, "添加住院记录");
        menu_item(2, "删除住院记录");
        menu_item(3, "修改住院记录");
        menu_item(4, "查询住院记录");
        menu_item(5, "显示所有住院记录");
        menu_blank();
        menu_item(0, "返回上级菜单");
        menu_bottom();

        printf("请输入您的选择: ");
        safe_input(buf, MAX_INPUT_LEN);

        if (!validate_choice(buf, 5))
        {
            printf("输入有误，请重新选择！\n");
            wait_enter();
            clear_screen();
            continue;
        }

        select = atoi(buf);

        switch (select)
        {
        case 1:
            add_hospitalization_record();
            break;
        case 2:
            delete_hospitalization_record();
            break;
        case 3:
            update_hospitalization_record();
            break;
        case 4:
            query_hospitalization_record();
            break;
        case 5:
            show_all_hospitalization_records();
            break;
        case 0:
            clear_screen();
            return;
        default:
            printf("未知错误: %d\n", select);
            break;
        }
    }
}

/* 处方记录管理 */
void manager_prescription_records_menu()
{
    char buf[MAX_INPUT_LEN];
    int select;
    while (1)
    {
        clear_screen();
        menu_top();
        menu_title("处方记录管理");
        menu_split();
        menu_item(1, "添加处方记录");
        menu_item(2, "删除处方记录");
        menu_item(3, "修改处方记录");
        menu_item(4, "查询处方记录");
        menu_item(5, "显示所有处方记录");
        menu_blank();
        menu_item(0, "返回上级菜单");
        menu_bottom();

        printf("请输入您的选择: ");
        safe_input(buf, MAX_INPUT_LEN);

        if (!validate_choice(buf, 5))
        {
            printf("输入有误，请重新选择！\n");
            wait_enter();
            clear_screen();
            continue;
        }

        select = atoi(buf);

        switch (select)
        {
        case 1:
            add_prescription_record();
            break;
        case 2:
            delete_prescription_record();
            break;
        case 3:
            update_prescription_record();
            break;
        case 4:
            query_prescription_record();
            break;
        case 5:
            show_all_prescription_records();
            break;
        case 0:
            clear_screen();
            return;
        default:
            printf("未知错误: %d\n", select);
            break;
        }
    }
}

/* 数据分析 */
void manager_analytics_menu()
{
    char buf[MAX_INPUT_LEN];
    int select;
    while (1)
    {
        clear_screen();
        menu_top();
        menu_title("数据分析");
        menu_split();
        menu_item(1, "病房利用率分析");
        menu_item(2, "科室门诊量与趋势分析");
        menu_item(3, "住院分析和病房优化");
        menu_item(4, "药品使用分析");
        menu_item(5, "住院时长分布与预测");
        menu_blank();
        menu_item(0, "返回上级菜单");
        menu_bottom();

        printf("请输入您的选择: ");
        safe_input(buf, MAX_INPUT_LEN);

        if (!validate_choice(buf, 5))
        {
            printf("输入有误，请重新选择！\n");
            wait_enter();
            clear_screen();
            continue;
        }

        select = atoi(buf);

        switch (select)
        {
        case 1:
            analytics_ward_utilization();
            break;
        case 2:
            analytics_department_workload();
            break;
        case 3:
            analytics_ward_optimization();
            break;
        case 4:
            analytics_drug_usage();
            break;
        case 5:
            analytics_hospitalization_duration();
            break;
        case 0:
            clear_screen();
            return;
        default:
            printf("未知错误: %d\n", select);
            break;
        }
    }
}

/*
 *患者系统子菜单
 */

/* 患者个人信息管理 */
void patient_personal_info_menu()
{
    char buf[MAX_INPUT_LEN];
    int select;

    while (1)
    {
        clear_screen();
        menu_top();
        menu_title("个人信息管理");
        menu_split();
        menu_item(1, "查看个人信息");
        menu_item(2, "修改个人信息");
        menu_blank();
        menu_item(0, "返回上级菜单");
        menu_bottom();

        printf("请输入您的选择: ");
        safe_input(buf, MAX_INPUT_LEN);

        if (!validate_choice(buf, 2))
        {
            printf("输入有误，请重新选择！\n");
            wait_enter();
            clear_screen();
            continue;
        }

        select = atoi(buf);

        switch (select)
        {
        case 1:
            patient_view_my_info();
            break;
        case 2:
            patient_update_my_info();
            break;
        case 0:
            clear_screen();
            return;
        default:
            printf("未知错误: %d\n", select);
            break;
        }
    }
}

/* 患者挂号预约 */
void patient_registration_menu()
{
    char buf[MAX_INPUT_LEN];
    int select;

    while (1)
    {
        clear_screen();
        menu_top();
        menu_title("挂号预约");
        menu_split();
        menu_item(1, "挂号预约");
        menu_item(2, "查询我的挂号");
        menu_item(3, "取消挂号预约");
        menu_blank();
        menu_item(0, "返回上级菜单");
        menu_bottom();

        printf("请输入您的选择: ");
        safe_input(buf, MAX_INPUT_LEN);

        if (!validate_choice(buf, 3))
        {
            printf("输入有误，请重新选择！\n");
            wait_enter();
            clear_screen();
            continue;
        }

        select = atoi(buf);

        switch (select)
        {
        case 1:
            patient_registration();
            break;
        case 2:
            patient_query_my_registrations();
            break;
        case 3:
            patient_cancel_registration();
            break;
        case 0:
            clear_screen();
            return;
        default:
            printf("未知错误: %d\n", select);
            break;
        }
    }
}

/*
 *医护系统子菜单
 */

/* 医生个人信息管理 */
void doctor_personal_info_menu()
{
    char buf[MAX_INPUT_LEN];
    int select;

    while (1)
    {
        clear_screen();
        menu_top();
        menu_title("个人信息管理");
        menu_split();
        menu_item(1, "查看个人信息");
        menu_item(2, "修改个人信息");
        menu_blank();
        menu_item(0, "返回上级菜单");
        menu_bottom();

        printf("请输入您的选择: ");
        safe_input(buf, MAX_INPUT_LEN);

        if (!validate_choice(buf, 2))
        {
            printf("输入有误，请重新选择！\n");
            wait_enter();
            clear_screen();
            continue;
        }

        select = atoi(buf);

        switch (select)
        {
        case 1:
            doctor_view_my_info();
            break;
        case 2:
            doctor_update_my_info();
            break;
        case 0:
            clear_screen();
            return;
        default:
            printf("未知错误: %d\n", select);
            break;
        }
    }
}

/* 医生查看患者信息 */
void doctor_view_patients_menu()
{
    char buf[MAX_INPUT_LEN];
    int select;

    while (1)
    {
        clear_screen();
        menu_top();
        menu_title("查看患者信息");
        menu_split();
        menu_item(1, "查询患者");
        menu_item(2, "显示所有患者");
        menu_blank();
        menu_item(0, "返回上级菜单");
        menu_bottom();

        printf("请输入您的选择: ");
        safe_input(buf, MAX_INPUT_LEN);

        if (!validate_choice(buf, 2))
        {
            printf("输入有误，请重新选择！\n");
            wait_enter();
            clear_screen();
            continue;
        }

        select = atoi(buf);

        switch (select)
        {
        case 1:
            query_patient();
            break;
        case 2:
            show_all_patients();
            break;
        case 0:
            clear_screen();
            return;
        default:
            printf("未知错误: %d\n", select);
            break;
        }
    }
}

/* 医生看诊管理 */
void doctor_visit_menu()
{
    if (!g_session.logged_in || strcmp(g_session.role, "doctor") != 0)
    {
        printf("请先登录医生账号！\n");
        wait_enter();
        clear_screen();
        return;
    }

    while (1)
    {
        /* 每次循环顶部加载数据 */
        Registration *r_head = load_registrations_from_file();
        Visit *v_head = load_visits_from_file();
        Patient *p_head = load_patients_from_file();
        Doctor *d_head = load_doctors_from_file();
        Exam *e_head = load_exams_from_file();

        int should_return = 0; /* 标记是否退出整个函数 */

        if (!r_head)
        {
            printf("暂无挂号数据！\n");
            wait_enter();
            should_return = 1;
            goto cleanup;
        }
        int reg_w, v_w, p_w, d_w, dept_w, when_w, st_w, diag_w;
        calc_registration_width(r_head, p_head, d_head, &reg_w, &p_w, &d_w, &dept_w, &when_w, &st_w);

        clear_screen();
        printf("===== 看诊管理 =====\n");

        printf("[1]待接诊患者列表: \n");
        print_registration_header(reg_w, p_w, d_w, dept_w, when_w, st_w);
        Registration *r_cur = find_registration_by_d_id(r_head, g_session.user_id);
        while (r_cur)
        {
            if (r_cur->status == REG_STATUS_PENDING)
            {
                print_registration(r_cur, p_head, d_head, reg_w, p_w, d_w, dept_w, when_w, st_w);
            }
            r_cur = find_registration_by_d_id(r_cur->next, g_session.user_id);
        }
        print_registration_line(reg_w, p_w, d_w, dept_w, when_w, st_w);

        printf("[2]进行中看诊患者列表: \n");
        if (v_head)
        {
            calc_visit_width(v_head, r_head, p_head, d_head, &v_w, &p_w, &d_w, &dept_w, &when_w, &st_w, &diag_w);
            print_visit_header(v_w, p_w, d_w, dept_w, when_w, st_w, diag_w);
            Visit *v_cur = find_visit_by_d_id(v_head, r_head, g_session.user_id);
            while (v_cur)
            {
                if (v_cur->status == VISIT_STATUS_ONGOING)
                {
                    print_visit(v_cur, r_head, p_head, d_head, v_w, p_w, d_w, dept_w, when_w, st_w, diag_w);
                }
                v_cur = find_visit_by_d_id(v_cur->next, r_head, g_session.user_id);
            }
            print_visit_line(v_w, p_w, d_w, dept_w, when_w, st_w, diag_w);
        }
        else
        {
            printf("（暂无进行中的看诊记录）\n");
        }

        printf("1.选择待接诊患者\n");
        printf("2.选择进行中看诊患者\n");
        printf("0.返回上级菜单\n");
        printf("请输入您的选择: ");

        char buf[MAX_INPUT_LEN];
        safe_input(buf, MAX_INPUT_LEN);

        if (!validate_choice(buf, 2))
        {
            printf("输入有误，请重新选择！\n");
            wait_enter();
            goto cleanup;
        }

        int select = atoi(buf);

        switch (select)
        {
        case 1: {
            char reg_id[MAX_ID_LEN];
            char v_id[MAX_ID_LEN];
            printf("请输入挂号ID开始看诊(输入0返回): ");
            safe_input(reg_id, sizeof(reg_id));

            if (strcmp(reg_id, "0") == 0)
                goto cleanup;

            Registration *reg = find_registration_by_r_id(r_head, reg_id);
            if (!reg || strcmp(reg->d_id, g_session.user_id) != 0 || reg->status != REG_STATUS_PENDING)
            {
                printf("输入无效，请重新选择！\n");
                wait_enter();
                goto cleanup;
            }
            snprintf(v_id, sizeof(v_id), "V%04d", generate_next_visit_id(v_head));
            printf("生成看诊ID: %s\n", v_id);
            reg->status = REG_STATUS_DONE;
            if (save_registrations_to_file(r_head) != 0)
                printf("保存挂号信息失败！\n");
            else
                printf("挂号状态已更新\n");
            Visit *new_node = (Visit *)malloc(sizeof(Visit));
            if (!new_node)
            {
                printf("内存分配失败！\n");
                wait_enter();
                goto cleanup;
            }
            *new_node = create_visit(v_id, reg_id, time(NULL), VISIT_STATUS_ONGOING, "");
            new_node->next = NULL;
            append_visit(&v_head, new_node);
            if (save_visits_to_file(v_head) != 0)
                printf("保存看诊信息失败！\n");
            else
                printf("看诊信息已保存\n");

            wait_enter();
            doctor_visit_patient(v_head, v_id, &e_head, r_head, p_head, d_head);
            goto cleanup;
        }
        case 2: {
            char v_id[MAX_ID_LEN];
            printf("请输入看诊ID继续看诊(输入0返回): ");
            safe_input(v_id, sizeof(v_id));

            if (strcmp(v_id, "0") == 0)
                goto cleanup;

            Visit *visit = find_visit_by_v_id(v_head, v_id);
            if (!visit)
            {
                printf("输入无效，请重新选择！\n");
                wait_enter();
                goto cleanup;
            }

            Registration *reg = find_registration_by_r_id(r_head, visit->reg_id);
            if (!reg)
            {
                printf("该看诊记录关联挂号不存在，数据异常！\n");
                wait_enter();
                goto cleanup;
            }

            if (strcmp(reg->d_id, g_session.user_id) != 0 || visit->status != VISIT_STATUS_ONGOING)
            {
                printf("输入无效，请重新选择！\n");
                wait_enter();
                goto cleanup;
            }

            printf("找到看诊记录！\n");
            wait_enter();
            doctor_visit_patient(v_head, v_id, &e_head, r_head, p_head, d_head);
            goto cleanup;
        }
        case 0:
            should_return = 1;
            goto cleanup;
        default:
            printf("未知错误: %d\n", select);
            goto cleanup;
        }

    cleanup:
        free_registrations(r_head);
        free_visits(v_head);
        free_exams(e_head);
        free_patients(p_head);
        free_doctors(d_head);

        if (should_return)
        {
            clear_screen();
            return;
        }
    }
}

/* 医生医疗记录管理 */
void doctor_medical_records_menu()
{
    char buf[MAX_INPUT_LEN];
    int select;
    while (1)
    {
        clear_screen();
        menu_top();
        menu_title("医疗记录管理");
        menu_split();
        menu_item(1, "检查记录管理");
        menu_item(2, "住院记录管理");
        menu_item(3, "处方记录管理");
        menu_blank();
        menu_item(0, "返回上级菜单");
        menu_bottom();

        printf("请输入您的选择: ");
        safe_input(buf, MAX_INPUT_LEN);

        if (!validate_choice(buf, 3))
        {
            printf("输入有误，请重新选择！\n");
            wait_enter();
            clear_screen();
            continue;
        }

        select = atoi(buf);

        switch (select)
        {
        case 1:
            doctor_exam_records_menu();
            break;
        case 2:
            doctor_hospitalization_records_menu();
            break;
        case 3:
            doctor_prescription_records_menu();
            break;
        case 0:
            clear_screen();
            return;
        default:
            printf("未知错误: %d\n", select);
            break;
        }
    }
}

/* 医生检查记录管理 */
void doctor_exam_records_menu()
{
    char buf[MAX_INPUT_LEN];
    int select;
    while (1)
    {
        clear_screen();
        menu_top();
        menu_title("检查记录管理");
        menu_split();
        menu_item(1, "添加检查记录");
        menu_item(2, "删除检查记录");
        menu_item(3, "修改检查记录");
        menu_item(4, "查询检查记录");
        menu_item(5, "显示所有检查记录");
        menu_blank();
        menu_item(0, "返回上级菜单");
        menu_bottom();

        printf("请输入您的选择: ");
        safe_input(buf, MAX_INPUT_LEN);

        if (!validate_choice(buf, 5))
        {
            printf("输入有误，请重新选择！\n");
            wait_enter();
            clear_screen();
            continue;
        }

        select = atoi(buf);

        switch (select)
        {
        case 1:
            add_exam_record();
            break;
        case 2:
            delete_exam_record();
            break;
        case 3:
            update_exam_record();
            break;
        case 4:
            query_exam_record();
            break;
        case 5:
            show_all_exam_records();
            break;
        case 0:
            clear_screen();
            return;
        default:
            printf("未知错误: %d\n", select);
            break;
        }
    }
}

/* 医生住院记录管理 */
void doctor_hospitalization_records_menu()
{
    char buf[MAX_INPUT_LEN];
    int select;
    while (1)
    {
        clear_screen();
        menu_top();
        menu_title("住院记录管理");
        menu_split();
        menu_item(1, "添加住院记录");
        menu_item(2, "删除住院记录");
        menu_item(3, "修改住院记录");
        menu_item(4, "查询住院记录");
        menu_item(5, "显示所有住院记录");
        menu_blank();
        menu_item(0, "返回上级菜单");
        menu_bottom();

        printf("请输入您的选择: ");
        safe_input(buf, MAX_INPUT_LEN);

        if (!validate_choice(buf, 5))
        {
            printf("输入有误，请重新选择！\n");
            wait_enter();
            clear_screen();
            continue;
        }

        select = atoi(buf);

        switch (select)
        {
        case 1:
            add_hospitalization_record();
            break;
        case 2:
            delete_hospitalization_record();
            break;
        case 3:
            update_hospitalization_record();
            break;
        case 4:
            query_hospitalization_record();
            break;
        case 5:
            show_all_hospitalization_records();
            break;
        case 0:
            clear_screen();
            return;
        default:
            printf("未知错误: %d\n", select);
            break;
        }
    }
}

/* 医生处方记录管理 */
void doctor_prescription_records_menu()
{
    char buf[MAX_INPUT_LEN];
    int select;
    while (1)
    {
        clear_screen();
        menu_top();
        menu_title("处方记录管理");
        menu_split();
        menu_item(1, "添加处方记录");
        menu_item(2, "删除处方记录");
        menu_item(3, "修改处方记录");
        menu_item(4, "查询处方记录");
        menu_item(5, "显示所有处方记录");
        menu_blank();
        menu_item(0, "返回上级菜单");
        menu_bottom();

        printf("请输入您的选择: ");
        safe_input(buf, MAX_INPUT_LEN);

        if (!validate_choice(buf, 5))
        {
            printf("输入有误，请重新选择！\n");
            wait_enter();
            clear_screen();
            continue;
        }

        select = atoi(buf);

        switch (select)
        {
        case 1:
            add_prescription_record();
            break;
        case 2:
            delete_prescription_record();
            break;
        case 3:
            update_prescription_record();
            break;
        case 4:
            query_prescription_record();
            break;
        case 5:
            show_all_prescription_records();
            break;
        case 0:
            clear_screen();
            return;
        default:
            printf("未知错误: %d\n", select);
            break;
        }
    }
}