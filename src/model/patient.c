/*
 * 患者功能模块
 */
#include "model/patient.h"
#include "core/auth.h"
#include "model/department.h"
#include "model/doctor.h"
#include "model/exam.h"
#include "model/hospitalization.h"
#include "model/registration.h"
#include "core/session.h"
#include "core/utils.h"
#include "model/visit.h"
#include "model/prescription.h"
#include "model/drug.h"

/*
 * 患者基础操作
 */

/* 从文件中加载患者数据 */
Patient *load_patients_from_file(void)
{
    FILE *fp = fopen(PATIENTS_FILE, "r");
    if (!fp)
        return NULL;

    Patient *head = NULL, *tail = NULL;
    char line[MAX_LINE_LEN];

    while (fgets(line, sizeof(line), fp))
    {
        trim_newline(line); // 去掉行末换行符
        if (!line[0])       // 如果行为空，跳过
            continue;

        Patient *node = (Patient *)malloc(sizeof(Patient)); // 分配内存
        if (!node)                                          // 如果内存分配失败，清理资源并返回NULL
        {
            fclose(fp);
            free_patients(head);
            return NULL;
        }
        memset(node, 0, sizeof(Patient)); // 初始化内存

        char *token = strtok(line, "|"); // 读取id
        if (!token)
        {
            free(node);
            continue;
        }
        strncpy(node->id, token, sizeof(node->id) - 1);
        node->id[sizeof(node->id) - 1] = '\0';

        token = strtok(NULL, "|"); // 读取name
        if (!token)
        {
            free(node);
            continue;
        }
        strncpy(node->name, token, sizeof(node->name) - 1);
        node->name[sizeof(node->name) - 1] = '\0';

        token = strtok(NULL, "|"); // 读取gender
        if (!token)
        {
            free(node);
            continue;
        }
        strncpy(node->gender, token, sizeof(node->gender) - 1);
        node->gender[sizeof(node->gender) - 1] = '\0';

        token = strtok(NULL, "|"); // 读取age
        if (!token)
        {
            free(node);
            continue;
        }
        node->age = atoi(token);

        token = strtok(NULL, "|"); // 读取pwd_hash
        if (!token)
        {
            free(node);
            continue;
        }
        strncpy(node->pwd_hash, token, sizeof(node->pwd_hash) - 1);
        node->pwd_hash[sizeof(node->pwd_hash) - 1] = '\0';

        token = strtok(NULL, "|"); // 读取salt
        if (!token)
        {
            free(node);
            continue;
        }
        strncpy(node->salt, token, sizeof(node->salt) - 1);
        node->salt[sizeof(node->salt) - 1] = '\0';

        node->next = NULL; // 初始化next指针
        if (!head)         // 如果链表为空，设置head和tail指向新节点
            head = tail = node;
        else // 否则将新节点添加到链表末尾，并更新tail指针
        {
            tail->next = node;
            tail = node;
        }
    }

    fclose(fp);
    return head;
}

/* 将患者数据保存到文件 */
int save_patients_to_file(Patient *head)
{
    char tmp_path[MAX_LINE_LEN];
    FILE *fp = safe_fopen_tmp(PATIENTS_FILE, tmp_path, sizeof(tmp_path));
    if (!fp)
        return -1;
    Patient *current = head;
    while (current)
    {
        fprintf(fp, "%s|%s|%s|%d|%s|%s\n", current->id, current->name, current->gender, current->age, current->pwd_hash,
                current->salt);
        current = current->next;
    }
    return safe_fclose_commit(fp, tmp_path, PATIENTS_FILE);
}

/* 释放患者数据内存 */
void free_patients(Patient *head)
{
    Patient *current = head;
    while (current)
    {
        Patient *temp = current;
        current = current->next;
        free(temp);
    }
}

/* 打印患者信息并对齐 */
void print_patient_align(const char *s, int width)
{
    int w = str_width(s);
    printf("%s", s);

    for (int i = 0; i < width - w; i++)
        putchar(' ');
}

/* 打印患者表格分隔线 */
void print_patient_line(int id_w, int name_w, int gen_w, int age_w)
{
    printf("+");
    for (int i = 0; i < id_w + 2; i++)
        printf("-");
    printf("+");
    for (int i = 0; i < name_w + 2; i++)
        printf("-");
    printf("+");
    for (int i = 0; i < gen_w + 2; i++)
        printf("-");
    printf("+");
    for (int i = 0; i < age_w + 2; i++)
        printf("-");
    printf("+\n");
}

/* 打印患者表头 */
void print_patient_header(int id_w, int name_w, int gen_w, int age_w)
{
    print_patient_line(id_w, name_w, gen_w, age_w);

    printf("| ");
    print_patient_align("患者ID", id_w);
    printf(" | ");
    print_patient_align("姓名", name_w);
    printf(" | ");
    print_patient_align("性别", gen_w);
    printf(" | ");
    print_patient_align("年龄", age_w);
    printf(" |\n");

    print_patient_line(id_w, name_w, gen_w, age_w);
}

/* 根据ID查找患者 */
Patient *find_patient_by_p_id(Patient *head, const char *id)
{
    Patient *current = head;
    while (current)
    {
        if (strcmp(current->id, id) == 0)
            return current;
        current = current->next;
    }
    return NULL;
}

/* 生成下一个患者ID(Pxxxx) */
int generate_next_patient_id(Patient *head)
{
    int max_id = 0;
    Patient *current = head;
    while (current)
    {
        const char *id = current->id;
        if (id[0] == 'P')
        {
            int valid = 1;
            for (int i = 1; id[i] != '\0'; i++)
            {
                if (id[i] < '0' || id[i] > '9')
                {
                    valid = 0;
                    break;
                }
            }
            if (valid && id[1] != '\0')
            {
                int id_num = atoi(id + 1);
                if (id_num > max_id)
                    max_id = id_num;
            }
        }
        current = current->next;
    }
    return max_id + 1;
}

/* 打印患者信息行 */
void print_patient(Patient *p, int id_w, int name_w, int gen_w, int age_w)
{
    printf("| ");
    print_patient_align(p->id, id_w);
    printf(" | ");
    print_patient_align(p->name, name_w);
    printf(" | ");
    print_patient_align(p->gender, gen_w);
    printf(" | ");
    char age_str[10];
    sprintf(age_str, "%d", p->age);
    print_patient_align(age_str, age_w);
    printf(" |\n");
}

/* 计算患者信息表格列宽 */
void calc_patient_width(Patient *head, int *id_w, int *name_w, int *gen_w, int *age_w)
{
    *id_w = 6;
    *name_w = 4;
    *gen_w = 6;
    *age_w = 3;
    for (Patient *p = head; p; p = p->next)
    {
        int w;
        w = str_width(p->id);
        if (w > *id_w)
            *id_w = w;
        w = str_width(p->name);
        if (w > *name_w)
            *name_w = w;
        w = str_width(p->gender);
        if (w > *gen_w)
            *gen_w = w;
        char buf[10];
        sprintf(buf, "%d", p->age);
        w = str_width(buf);
        if (w > *age_w)
            *age_w = w;
    }
}

/* 统计患者链表长度 */
int count_patients(Patient *head)
{
    int count = 0;
    while (head)
    {
        count++;
        head = head->next;
    }
    return count;
}

/* 跳到第n个患者 */
Patient *get_nth_patient(Patient *head, int n)
{
    int i = 0;
    while (head && i < n)
    {
        head = head->next;
        i++;
    }
    return head;
}

/*
 * 患者功能函数
 */

/* 创建患者 */
Patient create_patient(const char *id, const char *name, const char *gender, int age, const char *pwd_hash)
{
    Patient patient;
    memset(&patient, 0, sizeof(Patient));

    strncpy(patient.id, id, sizeof(patient.id) - 1);
    patient.id[sizeof(patient.id) - 1] = '\0';

    strncpy(patient.name, name, sizeof(patient.name) - 1);
    patient.name[sizeof(patient.name) - 1] = '\0';

    strncpy(patient.gender, gender, sizeof(patient.gender) - 1);
    patient.gender[sizeof(patient.gender) - 1] = '\0';

    patient.age = age;

    strncpy(patient.pwd_hash, pwd_hash, sizeof(patient.pwd_hash) - 1);
    patient.pwd_hash[sizeof(patient.pwd_hash) - 1] = '\0';

    patient.next = NULL;
    return patient;
}

/* 尾插患者 */
void append_patient(Patient **head, Patient *new_patient)
{
    if (!head || !new_patient)
        return;

    new_patient->next = NULL;

    if (!*head)
    {
        *head = new_patient;
        return;
    }

    Patient *current = *head;
    while (current->next)
        current = current->next;
    current->next = new_patient;
}

/*
 *患者系统功能
 */

/* 添加患者 */
void add_patient()
{
    char name[MAX_NAME_LEN];
    char gender[MAX_GENDER_LEN];
    char password[MAX_INPUT_LEN];
    int age = 0;

    /* 姓名 */
    while (1)
    {
        printf("请输入患者姓名(汉字 | 输入0返回): ");
        safe_input(name, sizeof(name));

        if (strcmp(name, "0") == 0)
            return;

        if (name[0] == '\0')
        {
            printf("输入错误！姓名不能为空，请重新输入。\n");
            continue;
        }
        if (!is_all_chinese_utf8(name))
        {
            printf("输入错误！姓名只能包含汉字，请重新输入。\n");
            continue;
        }
        break;
    }

    /* 性别 */
    while (1)
    {
        printf("请选择患者性别(男/女 | 输入0返回): ");
        safe_input(gender, sizeof(gender));

        if (strcmp(gender, "0") == 0)
            return;

        if (gender[0] == '\0')
        {
            printf("输入错误！性别不能为空，请重新输入。\n");
            continue;
        }
        if (strcmp(gender, "男") != 0 && strcmp(gender, "女") != 0)
        {
            printf("输入错误！请输入'男'或'女'。\n");
            continue;
        }
        break;
    }

    /* 年龄 */
    while (1)
    {
        char age_buf[MAX_INPUT_LEN];
        int start = 0;
        int is_number = 1;

        printf("请输入患者年龄(1-150 | 输入0返回): ");
        safe_input(age_buf, sizeof(age_buf));

        if (strcmp(age_buf, "0") == 0)
            return;

        if (age_buf[0] == '\0')
        {
            printf("输入错误！年龄不能为空，请重新输入。\n");
            continue;
        }

        /* 允许首字符为正负号 */
        if (age_buf[0] == '-' || age_buf[0] == '+')
            start = 1;

        if (age_buf[start] == '\0')
        {
            is_number = 0;
        }
        else
        {
            for (int i = start; age_buf[i] != '\0'; i++)
            {
                if (age_buf[i] < '0' || age_buf[i] > '9')
                {
                    is_number = 0;
                    break;
                }
            }
        }

        if (!is_number)
        {
            printf("输入错误！年龄只能输入数字，请重新输入。\n");
            continue;
        }

        age = atoi(age_buf);
        if (age < 1 || age > 150)
        {
            printf("输入错误！年龄范围必须在1-150之间，请重新输入。\n");
            continue;
        }

        break;
    }

    /* 密码 */
    while (1)
    {
        printf("请输入患者密码(输入0返回): ");
        safe_input(password, sizeof(password));

        if (strcmp(password, "0") == 0)
            return;

        if (password[0] == '\0')
        {
            printf("输入错误！密码不能为空，请重新输入。\n");
            continue;
        }
        break;
    }

    /* 只加载一次 */
    Patient *patient_head = load_patients_from_file();

    char id[MAX_ID_LEN];
    int next_num = generate_next_patient_id(patient_head);
    snprintf(id, sizeof(id), "P%04d", next_num);

    /* 生成盐与哈希 */
    unsigned char salt[SALT_RAW_LEN];
    char salt_hex[SALT_HEX_LEN];
    char pwd_hash[MAX_PWD_HASH];

    generate_salt(salt, SALT_RAW_LEN);
    bytes_to_hex(salt, SALT_RAW_LEN, salt_hex);
    build_pass_hash(salt, password, pwd_hash);

    /* 动态分配新节点 */
    Patient *new_node = (Patient *)malloc(sizeof(Patient));
    if (!new_node)
    {
        printf("内存分配失败！\n");
        free_patients(patient_head);
        wait_enter();
        return;
    }

    *new_node = create_patient(id, name, gender, age, pwd_hash);
    strncpy(new_node->salt, salt_hex, sizeof(new_node->salt) - 1);
    new_node->salt[sizeof(new_node->salt) - 1] = '\0';
    new_node->next = NULL;

    append_patient(&patient_head, new_node);

    if (save_patients_to_file(patient_head) != 0)
        printf("保存患者信息失败！\n");
    else
        printf("添加成功！添加的患者ID是: %s\n", id);

    free_patients(patient_head);
    wait_enter();
    clear_screen();
}

/* 删除患者 */
void delete_patient()
{
    char id[MAX_ID_LEN];
    printf("请输入要删除的患者ID(输入0返回): ");
    safe_input(id, sizeof(id));
    if (strcmp(id, "0") == 0)
    {
        clear_screen();
        return;
    }

    if (id[0] == '\0')
    {
        printf("输入错误！患者ID不能为空。\n");
        wait_enter();
        clear_screen();
        return;
    }

    Patient *patient_head = load_patients_from_file();
    Patient *current = patient_head, *prev = NULL;
    while (current)
    {
        if (strcmp(current->id, id) == 0)
        {
            printf("找到患者: %s (%s)，确定要删除吗？(y/n): ", current->name, current->id);
            char confirm[MAX_INPUT_LEN];
            safe_input(confirm, sizeof(confirm));
            if (strcmp(confirm, "n") == 0 || strcmp(confirm, "N") == 0) // 用户输入n/N取消删除
            {
                printf("已取消删除。\n");
                free_patients(patient_head);
                wait_enter();
                clear_screen();
                return;
            }
            else if (strcmp(confirm, "y") != 0 &&
                     strcmp(confirm, "Y") != 0) // 用户输入非y/Y/n/N的其他内容，视为无效输入并取消删除
            {
                printf("输入无效，已取消删除。\n");
                free_patients(patient_head);
                wait_enter();
                clear_screen();
                return;
            }
            else // 用户输入y/Y确认删除
            {
                // 检查是否有关联的未完成挂号记录
                Registration *reg_head = load_registrations_from_file();
                int has_active = 0;
                for (Registration *r = reg_head; r; r = r->next)
                {
                    if (strcmp(r->p_id, id) == 0 && r->status == REG_STATUS_PENDING)
                    {
                        has_active = 1;
                        break;
                    }
                }
                free_registrations(reg_head);

                if (!has_active)
                {
                    // 检查是否有进行中的住院记录
                    Hospitalization *h_head = load_hospitalizations_from_file();
                    for (Hospitalization *h = h_head; h; h = h->next)
                    {
                        if (strcmp(h->p_id, id) == 0 && h->status == HOSP_STATUS_ONGOING)
                        {
                            has_active = 1;
                            break;
                        }
                    }
                    free_hospitalizations(h_head);
                }

                if (has_active)
                {
                    printf("无法删除！该患者仍有未完成的挂号或住院记录，请先处理。\n");
                    free_patients(patient_head);
                    wait_enter();
                    clear_screen();
                    return;
                }

                if (prev)
                    prev->next = current->next;
                else
                    patient_head = current->next;

                free(current);
                if (save_patients_to_file(patient_head) != 0)
                    printf("保存患者信息失败！\n");
                else
                    printf("删除成功！\n");

                free_patients(patient_head);
                wait_enter();
                clear_screen();
                return;
            }
        }
        prev = current;
        current = current->next;
    }
    printf("未找到指定ID的患者！\n");
    wait_enter();
    clear_screen();
}

/* 更新患者信息 */
void update_patient()
{
    char id[MAX_ID_LEN];
    printf("请输入要更新的患者ID(输入0返回): ");
    safe_input(id, sizeof(id));
    if (strcmp(id, "0") == 0)
    {
        clear_screen();
        return;
    }

    Patient *patient_head = load_patients_from_file();
    if (!patient_head)
    {
        printf("没有患者数据！\n");
        wait_enter();
        clear_screen();
        return;
    }

    Patient *patient = find_patient_by_p_id(patient_head, id);
    if (!patient)
    {
        printf("未找到指定ID的患者！\n");
        free_patients(patient_head);
        wait_enter();
        clear_screen();
        return;
    }

    int id_w, name_w, gen_w, age_w;
    calc_patient_width(patient_head, &id_w, &name_w, &gen_w, &age_w);

    char buf[MAX_INPUT_LEN];
    int select;

    while (1)
    {
        clear_screen();
        printf("找到患者: %s (%s)\n", patient->name, patient->id);
        print_patient_header(id_w, name_w, gen_w, age_w);
        print_patient(patient, id_w, name_w, gen_w, age_w);
        print_patient_line(id_w, name_w, gen_w, age_w);

        printf("请选择要更新的信息:\n");
        printf("1. 姓名\n");
        printf("2. 性别\n");
        printf("3. 年龄\n");
        printf("4. 密码\n");
        printf("0. 退出更新\n");

        printf("请输入您的选择: ");
        safe_input(buf, sizeof(buf));

        if (!validate_choice(buf, 4))
        {
            printf("输入有误，请重新选择！\n");
            wait_enter();
            continue;
        }

        select = atoi(buf);

        switch (select)
        {
        /* 更新姓名 */
        case 1:
        {
            char name[MAX_NAME_LEN];
            while (1)
            {
                printf("请输入患者姓名(汉字 | 输入0返回): ");
                safe_input(name, sizeof(name));

                if (strcmp(name, "0") == 0)
                    break;

                if (name[0] == '\0')
                {
                    printf("输入错误！姓名不能为空，请重新输入。\n");
                    continue;
                }
                if (!is_all_chinese_utf8(name))
                {
                    printf("输入错误！姓名只能包含汉字，请重新输入。\n");
                    continue;
                }
                strncpy(patient->name, name, sizeof(patient->name) - 1);
                patient->name[sizeof(patient->name) - 1] = '\0';

                if (save_patients_to_file(patient_head) != 0)
                    printf("保存患者信息失败！\n");
                else
                    printf("姓名更新成功！\n");

                wait_enter();
                break;
            }
            break;
        }
        /* 更新性别 */
        case 2:
        {
            char gender[MAX_GENDER_LEN];
            while (1)
            {
                printf("请输入患者性别(男/女 | 输入0返回): ");
                safe_input(gender, sizeof(gender));

                if (strcmp(gender, "0") == 0)
                    break;

                if (strcmp(gender, "男") != 0 && strcmp(gender, "女") != 0)
                {
                    printf("输入错误！性别只能为男或女。\n");
                    continue;
                }

                strncpy(patient->gender, gender, sizeof(patient->gender) - 1);
                patient->gender[sizeof(patient->gender) - 1] = '\0';

                if (save_patients_to_file(patient_head) != 0)
                    printf("保存患者信息失败！\n");
                else
                    printf("性别更新成功！\n");

                wait_enter();
                break;
            }
            break;
        }
        /* 更新年龄 */
        case 3:
        {
            while (1)
            {
                char age_buf[MAX_INPUT_LEN];
                int start = 0;
                int is_number = 1;

                printf("请输入患者年龄(1-150 | 输入0返回): ");
                safe_input(age_buf, sizeof(age_buf));

                if (strcmp(age_buf, "0") == 0)
                    break;

                if (age_buf[0] == '\0')
                {
                    printf("输入错误！年龄不能为空，请重新输入。\n");
                    continue;
                }

                /* 允许首字符为正负号 */
                if (age_buf[0] == '-' || age_buf[0] == '+')
                    start = 1;

                if (age_buf[start] == '\0')
                {
                    is_number = 0;
                }
                else
                {
                    for (int i = start; age_buf[i] != '\0'; i++)
                    {
                        if (age_buf[i] < '0' || age_buf[i] > '9')
                        {
                            is_number = 0;
                            break;
                        }
                    }
                }

                if (!is_number)
                {
                    printf("输入错误！年龄只能输入数字，请重新输入。\n");
                    continue;
                }

                int age = atoi(age_buf);
                if (age < 1 || age > 150)
                {
                    printf("输入错误！年龄范围必须在1-150之间，请重新输入。\n");
                    continue;
                }

                patient->age = age;

                if (save_patients_to_file(patient_head) != 0)
                    printf("保存患者信息失败！\n");
                else
                    printf("年龄更新成功！\n");

                wait_enter();
                break;
            }
            break;
        }
        /* 更新密码 */
        case 4:
        {
            while (1)
            {
                char new_password[MAX_INPUT_LEN];

                printf("请输入患者新密码(输入0返回): ");
                safe_input(new_password, sizeof(new_password));

                if (strcmp(new_password, "0") == 0)
                    break;

                if (new_password[0] == '\0')
                {
                    printf("输入错误！密码不能为空，请重新输入。\n");
                    continue;
                }

                unsigned char new_salt[SALT_RAW_LEN];
                char new_salt_hex[SALT_HEX_LEN];
                char new_pwd_hash[MAX_PWD_HASH];

                generate_salt(new_salt, SALT_RAW_LEN);
                bytes_to_hex(new_salt, SALT_RAW_LEN, new_salt_hex);
                build_pass_hash(new_salt, new_password, new_pwd_hash);

                strncpy(patient->salt, new_salt_hex, sizeof(patient->salt) - 1);
                patient->salt[sizeof(patient->salt) - 1] = '\0';
                strncpy(patient->pwd_hash, new_pwd_hash, sizeof(patient->pwd_hash) - 1);
                patient->pwd_hash[sizeof(patient->pwd_hash) - 1] = '\0';

                if (save_patients_to_file(patient_head) != 0)
                    printf("保存患者信息失败！\n");
                else
                    printf("密码更新成功！\n");

                wait_enter();
                break;
            }
            break;
        }
        case 0:
            printf("欢迎下次使用\n");
            free_patients(patient_head);
            wait_enter();
            clear_screen();
            return;
        default:
            printf("未知错误: %d\n", select);
            break;
        }
    }
}

/* 查询患者信息（支持ID精确查询和姓名模糊查询） */
void query_patient()
{
    Patient *patient_head = load_patients_from_file();
    if (!patient_head)
    {
        printf("没有患者数据！\n");
        wait_enter();
        clear_screen();
        return;
    }

    char buf[MAX_INPUT_LEN];
    while (1)
    {
        clear_screen();
        printf("===== 查询患者信息 =====\n");
        printf("1. 按患者ID精确查询\n");
        printf("2. 按姓名模糊查询\n");
        printf("0. 返回\n");
        printf("请输入您的选择: ");
        safe_input(buf, sizeof(buf));

        if (!validate_choice(buf, 2))
        {
            printf("输入有误，请重新选择！\n");
            wait_enter();
            continue;
        }

        int select = atoi(buf);
        if (select == 0)
            break;

        char query[MAX_INPUT_LEN];
        printf("请输入查询关键字(输入0返回): ");
        safe_input(query, sizeof(query));
        if (strcmp(query, "0") == 0)
            continue;

        int id_w, name_w, gen_w, age_w;
        calc_patient_width(patient_head, &id_w, &name_w, &gen_w, &age_w);

        int found = 0;
        for (Patient *cur = patient_head; cur; cur = cur->next)
        {
            int match = 0;
            if (select == 1 && strcmp(cur->id, query) == 0) /* 精确匹配ID */
                match = 1;
            else if (select == 2 && strstr(cur->name, query) != NULL) /* 姓名子串匹配 */
                match = 1;

            if (match)
            {
                if (!found)
                {
                    printf("查询结果:\n");
                    print_patient_header(id_w, name_w, gen_w, age_w);
                    found = 1;
                }
                print_patient(cur, id_w, name_w, gen_w, age_w);
            }
        }

        if (found)
            print_patient_line(id_w, name_w, gen_w, age_w);
        else
            printf("未找到匹配的患者！\n");

        wait_enter();
    }

    free_patients(patient_head);
    clear_screen();
}

/* 显示所有患者信息 */
void show_all_patients()
{
    Patient *patient_head = load_patients_from_file();
    if (!patient_head)
    {
        printf("暂无患者数据\n");
        wait_enter();
        clear_screen();
        return;
    }

    int page_size = PAGE_SIZE; // 每页显示的患者数量
    int total = count_patients(patient_head);
    int total_pages = (total + page_size - 1) / page_size;

    int current_page = 1;

    int id_w, name_w, gen_w, age_w;
    calc_patient_width(patient_head, &id_w, &name_w, &gen_w, &age_w);

    while (1)
    {
        clear_screen();

        printf("===== 患者列表（第 %d/%d 页）=====\n", current_page, total_pages);

        print_patient_header(id_w, name_w, gen_w, age_w);

        int start = (current_page - 1) * page_size;
        Patient *cur = get_nth_patient(patient_head, start);

        for (int i = 0; i < page_size && cur; i++)
        {
            print_patient(cur, id_w, name_w, gen_w, age_w);
            cur = cur->next;
        }

        print_patient_line(id_w, name_w, gen_w, age_w);

        printf("\n[n]下一页  [p]上一页  [q]退出\n> ");

        char buf[MAX_INPUT_LEN];
        safe_input(buf, sizeof(buf));

        if (strcmp(buf, "n") == 0 || strcmp(buf, "N") == 0)
        {
            if (current_page < total_pages)
                current_page++;
        }
        else if (strcmp(buf, "p") == 0 || strcmp(buf, "P") == 0)
        {
            if (current_page > 1)
                current_page--;
        }
        else if (strcmp(buf, "q") == 0 || strcmp(buf, "Q") == 0)
        {
            break;
        }
        else
        {
            printf("输入无效，请重新输入！\n");
            wait_enter();
        }
    }
}

/* 患者查看个人信息 */
void patient_view_my_info()
{
    if (!g_session.logged_in || strcmp(g_session.role, "patient") != 0)
    {
        printf("请先登录患者账号！\n");
        wait_enter();
        clear_screen();
        return;
    }

    Patient *patient_head = load_patients_from_file();
    if (!patient_head)
    {
        printf("没有患者数据！\n");
        wait_enter();
        clear_screen();
        return;
    }

    Patient *me = find_patient_by_p_id(patient_head, g_session.user_id);
    if (!me)
    {
        printf("未找到当前登录患者信息(ID: %s)\n", g_session.user_id);
        free_patients(patient_head);
        wait_enter();
        clear_screen();
        return;
    }

    int id_w, name_w, gen_w, age_w;
    calc_patient_width(patient_head, &id_w, &name_w, &gen_w, &age_w);

    printf("我的信息：\n");
    print_patient_header(id_w, name_w, gen_w, age_w);
    print_patient(me, id_w, name_w, gen_w, age_w);
    print_patient_line(id_w, name_w, gen_w, age_w);

    free_patients(patient_head);
    wait_enter();
    clear_screen();
}

/* 患者修改个人信息 */
void patient_update_my_info()
{
    if (!g_session.logged_in || strcmp(g_session.role, "patient") != 0)
    {
        printf("请先登录患者账号！\n");
        wait_enter();
        clear_screen();
        return;
    }

    Patient *patient_head = load_patients_from_file();
    if (!patient_head)
    {
        printf("没有患者数据！\n");
        wait_enter();
        clear_screen();
        return;
    }

    Patient *me = find_patient_by_p_id(patient_head, g_session.user_id);
    if (!me)
    {
        printf("未找到当前登录患者信息(ID: %s)\n", g_session.user_id);
        free_patients(patient_head);
        wait_enter();
        clear_screen();
        return;
    }

    int id_w, name_w, gen_w, age_w;
    calc_patient_width(patient_head, &id_w, &name_w, &gen_w, &age_w);

    char buf[MAX_INPUT_LEN];
    int select;

    while (1)
    {
        clear_screen();
        printf("我的信息：\n");
        print_patient_header(id_w, name_w, gen_w, age_w);
        print_patient(me, id_w, name_w, gen_w, age_w);
        print_patient_line(id_w, name_w, gen_w, age_w);

        printf("请选择要更新的信息:\n");
        printf("1. 姓名\n");
        printf("2. 性别\n");
        printf("3. 年龄\n");
        printf("4. 密码\n");
        printf("0. 退出更新\n");

        printf("请输入您的选择: ");
        safe_input(buf, sizeof(buf));

        if (!validate_choice(buf, 4))
        {
            printf("输入有误，请重新选择！\n");
            wait_enter();
            continue;
        }

        select = atoi(buf);

        switch (select)
        {
        /* 更新姓名 */
        case 1:
        {
            char name[MAX_NAME_LEN];
            while (1)
            {
                printf("请输入患者姓名(汉字 | 输入0返回): ");
                safe_input(name, sizeof(name));

                if (strcmp(name, "0") == 0)
                    break;

                if (name[0] == '\0')
                {
                    printf("输入错误！姓名不能为空，请重新输入。\n");
                    continue;
                }
                if (!is_all_chinese_utf8(name))
                {
                    printf("输入错误！姓名只能包含汉字，请重新输入。\n");
                    continue;
                }
                strncpy(me->name, name, sizeof(me->name) - 1);
                me->name[sizeof(me->name) - 1] = '\0';

                if (save_patients_to_file(patient_head) != 0)
                    printf("保存患者信息失败！\n");
                else
                    printf("姓名更新成功！\n");

                wait_enter();
                break;
            }
            break;
        }
        /* 更新性别 */
        case 2:
        {
            char gender[MAX_GENDER_LEN];
            while (1)
            {
                printf("请输入患者性别(男/女 | 输入0返回): ");
                safe_input(gender, sizeof(gender));

                if (strcmp(gender, "0") == 0)
                    break;

                if (strcmp(gender, "男") != 0 && strcmp(gender, "女") != 0)
                {
                    printf("输入错误！性别只能为男或女。\n");
                    continue;
                }

                strncpy(me->gender, gender, sizeof(me->gender) - 1);
                me->gender[sizeof(me->gender) - 1] = '\0';

                if (save_patients_to_file(patient_head) != 0)
                    printf("保存患者信息失败！\n");
                else
                    printf("性别更新成功！\n");

                wait_enter();
                break;
            }
            break;
        }
        /* 更新年龄 */
        case 3:
        {
            while (1)
            {
                char age_buf[MAX_INPUT_LEN];
                int start = 0;
                int is_number = 1;

                printf("请输入患者年龄(1-150 | 输入0返回): ");
                safe_input(age_buf, sizeof(age_buf));

                if (strcmp(age_buf, "0") == 0)
                    break;

                if (age_buf[0] == '\0')
                {
                    printf("输入错误！年龄不能为空，请重新输入。\n");
                    continue;
                }

                /* 允许首字符为正负号 */
                if (age_buf[0] == '-' || age_buf[0] == '+')
                    start = 1;

                if (age_buf[start] == '\0')
                {
                    is_number = 0;
                }
                else
                {
                    for (int i = start; age_buf[i] != '\0'; i++)
                    {
                        if (age_buf[i] < '0' || age_buf[i] > '9')
                        {
                            is_number = 0;
                            break;
                        }
                    }
                }

                if (!is_number)
                {
                    printf("输入错误！年龄只能输入数字，请重新输入。\n");
                    continue;
                }

                int age = atoi(age_buf);
                if (age < 1 || age > 150)
                {
                    printf("输入错误！年龄范围必须在1-150之间，请重新输入。\n");
                    continue;
                }

                me->age = age;

                if (save_patients_to_file(patient_head) != 0)
                    printf("保存患者信息失败！\n");
                else
                    printf("年龄更新成功！\n");

                wait_enter();
                break;
            }
            break;
        }
        /* 更新密码 */
        case 4:
        {
            while (1)
            {
                char old_password[MAX_INPUT_LEN];
                char new_password[MAX_INPUT_LEN];

                printf("请输入患者旧密码(输入0返回): ");
                safe_input(old_password, sizeof(old_password));

                if (strcmp(old_password, "0") == 0)
                    break;

                if (!(verify_password(old_password, me->salt, me->pwd_hash)))
                {
                    printf("输入错误！旧密码不正确，请重新输入。\n");
                    continue;
                }

                int cancel_new_pwd = 0;
                while (1)
                {
                    printf("请输入患者新密码(输入0返回): ");
                    safe_input(new_password, sizeof(new_password));

                    if (strcmp(new_password, "0") == 0)
                    {
                        cancel_new_pwd = 1;
                        break;
                    }

                    if (new_password[0] == '\0')
                    {
                        printf("输入错误！密码不能为空，请重新输入。\n");
                        continue;
                    }
                    break;
                }

                if (cancel_new_pwd)
                    break;

                unsigned char new_salt[SALT_RAW_LEN];
                char new_salt_hex[SALT_HEX_LEN];
                char new_pwd_hash[MAX_PWD_HASH];

                generate_salt(new_salt, SALT_RAW_LEN);
                bytes_to_hex(new_salt, SALT_RAW_LEN, new_salt_hex);
                build_pass_hash(new_salt, new_password, new_pwd_hash);

                strncpy(me->salt, new_salt_hex, sizeof(me->salt) - 1);
                me->salt[sizeof(me->salt) - 1] = '\0';
                strncpy(me->pwd_hash, new_pwd_hash, sizeof(me->pwd_hash) - 1);
                me->pwd_hash[sizeof(me->pwd_hash) - 1] = '\0';

                if (save_patients_to_file(patient_head) != 0)
                    printf("保存患者信息失败！\n");
                else
                    printf("密码更新成功！\n");

                wait_enter();
                break;
            }
            break;
        }
        case 0:
            printf("欢迎下次使用\n");
            free_patients(patient_head);
            wait_enter();
            clear_screen();
            return;
        default:
            printf("未知错误: %d\n", select);
            break;
        }
    }
}

void patient_registration()
{
    if (!g_session.logged_in || strcmp(g_session.role, "patient") != 0)
    {
        printf("请先登录患者账号！\n");
        wait_enter();
        clear_screen();
        return;
    }

    char department[MAX_INPUT_LEN];
    char doctor_id[MAX_ID_LEN];
    // 输入科室名称并显示该科室医生列表
    while (1)
    {
        print_department_hint();
        printf("请选择要挂号预约的科室(输入0返回): ");
        safe_input(department, sizeof(department));

        if (strcmp(department, "0") == 0)
        {
            clear_screen();
            return;
        }

        Department *dept_head = load_departments_from_file();
        if (!dept_head)
        {
            printf("没有科室数据！\n");
            wait_enter();
            clear_screen();
            return;
        }

        int dept_found = 0;
        Department *current = dept_head;
        while (current)
        {
            if (strcmp(current->name, department) == 0)
            {
                dept_found = 1;
                break;
            }
            current = current->next;
        }
        free_departments(dept_head);

        if (!dept_found)
        {
            printf("输入错误！科室不存在，请重新输入。\n");
            continue;
        }

        Doctor *doctor_head = load_doctors_from_file();
        if (!doctor_head)
        {
            printf("没有医生数据！\n");
            wait_enter();
            clear_screen();
            return;
        }

        printf("科室: %s\n", department);
        int id_w, name_w, gen_w, dept_w;
        calc_doctor_width(doctor_head, &id_w, &name_w, &gen_w, &dept_w);
        print_doctor_header(id_w, name_w, gen_w, dept_w);

        Doctor *doc_current = doctor_head;
        while (doc_current)
        {
            if (strcmp(doc_current->department, department) == 0)
            {
                print_doctor(doc_current, id_w, name_w, gen_w, dept_w);
            }
            doc_current = doc_current->next;
        }

        print_doctor_line(id_w, name_w, gen_w, dept_w);

        // 输入医生ID并校验属于该科室
        while (1)
        {
            printf("请输入要挂号预约的医生ID(输入0返回): ");
            safe_input(doctor_id, sizeof(doctor_id));

            if (strcmp(doctor_id, "0") == 0)
            {
                clear_screen();
                return;
            }

            int valid_doctor = 0;
            doc_current = doctor_head;
            while (doc_current)
            {
                if (strcmp(doc_current->id, doctor_id) == 0 && strcmp(doc_current->department, department) == 0)
                {
                    valid_doctor = 1;
                    break;
                }
                doc_current = doc_current->next;
            }

            if (!valid_doctor)
            {
                printf("输入错误！医生ID不存在或不属于该科室，请重新输入。\n");
                continue;
            }

            Registration *reg_head = load_registrations_from_file();

            char id[MAX_ID_LEN];
            int next_num = generate_next_registration_id(reg_head);
            snprintf(id, sizeof(id), "R%04d", next_num);

            Registration *new_node = (Registration *)malloc(sizeof(Registration));
            if (!new_node)
            {
                printf("内存分配失败！\n");
                free_registrations(reg_head);
                wait_enter();
                clear_screen();
                return;
            }

            *new_node = create_registration(id, g_session.user_id, doctor_id, time(NULL), 0);
            append_registration(&reg_head, new_node);

            if (save_registrations_to_file(reg_head) != 0)
                printf("保存挂号信息失败！\n");
            else
                printf("挂号预约成功！挂号ID是: %s\n", id);

            free_registrations(reg_head);
            free_doctors(doctor_head);
            wait_enter();
            clear_screen();
            return;
        }
    }
}

/* 查询我的挂号 */
void patient_query_my_registrations()
{
    if (!g_session.logged_in || strcmp(g_session.role, "patient") != 0)
    {
        printf("请先登录患者账号！\n");
        wait_enter();
        clear_screen();
        return;
    }

    Registration *reg_head = load_registrations_from_file();
    if (!reg_head)
    {
        printf("暂无挂号数据！\n");
        wait_enter();
        clear_screen();
        return;
    }

    Patient *p_head = load_patients_from_file();
    Doctor *d_head = load_doctors_from_file();

    clear_screen();
    printf("===== 我的挂号记录（患者ID: %s）=====\n", g_session.user_id);

    Registration *hit = find_registration_by_p_id(reg_head, g_session.user_id);

    if (!hit)
    {
        printf("\n暂无挂号记录！\n");
    }
    else
    {
        int reg_w, p_w, d_w, dept_w, when_w, st_w;
        calc_registration_width(reg_head, p_head, d_head, &reg_w, &p_w, &d_w, &dept_w, &when_w, &st_w);

        print_registration_header(reg_w, p_w, d_w, dept_w, when_w, st_w);

        while (hit)
        {
            print_registration(hit, p_head, d_head, reg_w, p_w, d_w, dept_w, when_w, st_w);
            hit = find_registration_by_p_id(hit->next, g_session.user_id);
        }

        print_registration_line(reg_w, p_w, d_w, dept_w, when_w, st_w);
    }

    free_registrations(reg_head);
    free_patients(p_head);
    free_doctors(d_head);

    printf("\n");
    wait_enter();
    clear_screen();
}

/* 取消挂号预约 */
void patient_cancel_registration()
{
    if (!g_session.logged_in || strcmp(g_session.role, "patient") != 0)
    {
        printf("请先登录患者账号！\n");
        wait_enter();
        clear_screen();
        return;
    }

    Registration *reg_head = load_registrations_from_file();
    if (!reg_head)
    {
        printf("暂无挂号数据！\n");
        wait_enter();
        clear_screen();
        return;
    }

    Patient *p_head = load_patients_from_file();
    Doctor *d_head = load_doctors_from_file();

    int reg_w, p_w, d_w, dept_w, when_w, st_w;
    calc_registration_width(reg_head, p_head, d_head, &reg_w, &p_w, &d_w, &dept_w, &when_w, &st_w);

    while (1)
    {
        clear_screen();
        printf("===== 我的挂号记录（患者ID: %s）=====\n", g_session.user_id);
        print_registration_header(reg_w, p_w, d_w, dept_w, when_w, st_w);

        int mine_count = 0;
        Registration *hit = find_registration_by_p_id(reg_head, g_session.user_id);
        while (hit)
        {
            print_registration(hit, p_head, d_head, reg_w, p_w, d_w, dept_w, when_w, st_w);
            mine_count++;
            hit = find_registration_by_p_id(hit->next, g_session.user_id);
        }
        print_registration_line(reg_w, p_w, d_w, dept_w, when_w, st_w);

        if (mine_count == 0)
        {
            printf("暂无挂号记录！\n");
            free_registrations(reg_head);
            free_patients(p_head);
            free_doctors(d_head);
            wait_enter();
            clear_screen();
            return;
        }

        char reg_id[MAX_ID_LEN];
        printf("请输入要取消的挂号ID(输入0返回): ");
        safe_input(reg_id, sizeof(reg_id));

        if (strcmp(reg_id, "0") == 0)
        {
            free_registrations(reg_head);
            free_patients(p_head);
            free_doctors(d_head);
            clear_screen();
            return;
        }

        Registration *target = find_registration_by_r_id(reg_head, reg_id);

        if (!target || strcmp(target->p_id, g_session.user_id) != 0 || target->status != REG_STATUS_PENDING)
        {
            printf("挂号ID无效或不可取消，请重新输入。\n");
            wait_enter();
            continue;
        }

        char confirm[MAX_INPUT_LEN];
        printf("确认取消挂号 %s 吗？(y/n): ", target->reg_id);
        safe_input(confirm, sizeof(confirm));

        if (strcmp(confirm, "y") == 0 || strcmp(confirm, "Y") == 0)
        {
            target->status = REG_STATUS_CANCELED;

            if (save_registrations_to_file(reg_head) != 0)
                printf("保存挂号信息失败！\n");
            else
                printf("挂号取消成功！\n");

            free_registrations(reg_head);
            free_patients(p_head);
            free_doctors(d_head);
            wait_enter();
            clear_screen();
            return;
        }
        else if (strcmp(confirm, "n") == 0 || strcmp(confirm, "N") == 0)
        {
            printf("已取消本次操作，返回重新输入。\n");
            wait_enter();
            continue;
        }
        else
        {
            printf("输入无效，返回重新输入。\n");
            wait_enter();
            continue;
        }
    }
}

/* 患者查看我的就诊记录 */
void patient_view_my_visits_records()
{
    if (!g_session.logged_in || strcmp(g_session.role, "patient") != 0)
    {
        printf("请先登录患者账号！\n");
        wait_enter();
        clear_screen();
        return;
    }

    Visit *visit_head = load_visits_from_file();
    if (!visit_head)
    {
        printf("暂无就诊数据！\n");
        wait_enter();
        clear_screen();
        return;
    }

    Patient *p_head = load_patients_from_file();
    Doctor *d_head = load_doctors_from_file();
    Registration *reg_head = load_registrations_from_file();

    clear_screen();
    printf("===== 我的就诊记录（患者ID: %s）=====\n", g_session.user_id);

    Visit *hit = find_visit_by_p_id(visit_head, reg_head, g_session.user_id);

    if (!hit)
    {
        printf("\n暂无就诊数据！\n");
    }
    else
    {
        int visit_w, p_w, d_w, dept_w, when_w, st_w, diag_w;
        calc_visit_width(visit_head, reg_head, p_head, d_head, &visit_w, &p_w, &d_w, &dept_w, &when_w, &st_w, &diag_w);

        print_visit_header(visit_w, p_w, d_w, dept_w, when_w, st_w, diag_w);

        while (hit)
        {
            print_visit(hit, reg_head, p_head, d_head, visit_w, p_w, d_w, dept_w, when_w, st_w, diag_w);
            hit = find_visit_by_p_id(hit->next, reg_head, g_session.user_id);
        }

        print_visit_line(visit_w, p_w, d_w, dept_w, when_w, st_w, diag_w);
    }

    free_visits(visit_head);
    free_patients(p_head);
    free_doctors(d_head);
    free_registrations(reg_head);

    printf("\n");
    wait_enter();
    clear_screen();
}

/* 患者查看我的检查记录 */
void patient_view_my_exams_records()
{
    if (!g_session.logged_in || strcmp(g_session.role, "patient") != 0)
    {
        printf("请先登录患者账号！\n");
        wait_enter();
        clear_screen();
        return;
    }

    Exam *exam_head = load_exams_from_file();
    if (!exam_head)
    {
        printf("暂无检查数据！\n");
        wait_enter();
        clear_screen();
        return;
    }

    Patient *p_head = load_patients_from_file();
    Doctor *d_head = load_doctors_from_file();
    Registration *reg_head = load_registrations_from_file();
    Visit *visit_head = load_visits_from_file();

    clear_screen();
    printf("===== 我的检查记录（患者ID: %s）=====\n", g_session.user_id);

    Exam *hit = find_exam_by_p_id(exam_head, visit_head, reg_head, g_session.user_id);

    if (!hit)
    {
        printf("\n暂无检查数据！\n");
    }
    else
    {
        int exam_w, p_w, d_w, dept_w, when_w, type_w, res_w;
        calc_exam_width(exam_head, visit_head, reg_head, p_head, d_head, &exam_w, &p_w, &d_w, &dept_w, &when_w, &type_w, &res_w);

        print_exam_header(exam_w, p_w, d_w, dept_w, when_w, type_w, res_w);

        while (hit)
        {
            print_exam(hit, visit_head, reg_head, p_head, d_head, exam_w, p_w, d_w, dept_w, when_w, type_w, res_w);
            hit = find_exam_by_p_id(hit->next, visit_head, reg_head, g_session.user_id);
        }

        print_exam_line(exam_w, p_w, d_w, dept_w, when_w, type_w, res_w);
    }

    free_exams(exam_head);
    free_patients(p_head);
    free_doctors(d_head);
    free_registrations(reg_head);
    free_visits(visit_head);

    printf("\n");
    wait_enter();
    clear_screen();
}

/* 患者查看我的住院记录 */
void patient_view_my_hospitalization_records()
{
    if (!g_session.logged_in || strcmp(g_session.role, "patient") != 0)
    {
        printf("请先登录患者账号！\n");
        wait_enter();
        clear_screen();
        return;
    }

    Hospitalization *hosp_head = load_hospitalizations_from_file();
    if (!hosp_head)
    {
        printf("暂无住院数据！\n");
        wait_enter();
        clear_screen();
        return;
    }

    Patient *p_head = load_patients_from_file();
    Registration *reg_head = load_registrations_from_file();
    Visit *visit_head = load_visits_from_file();
    Ward *ward_head = load_wards_from_file();
    Bed *bed_head = load_beds_from_file();

    clear_screen();
    printf("===== 我的住院记录（患者ID: %s）=====\n", g_session.user_id);

    Hospitalization *hit = find_hospitalization_by_p_id(hosp_head, g_session.user_id);

    if (!hit)
    {
        printf("\n暂无住院数据！\n");
    }
    else
    {
        int h_w, v_w, p_w, ward_w, bed_w, in_w, out_w, st_w;
        calc_hospitalization_width(hosp_head, visit_head, reg_head, p_head, ward_head, bed_head, &h_w, &v_w, &p_w, &ward_w, &bed_w, &in_w, &out_w, &st_w);

        print_hospitalization_header(h_w, v_w, p_w, ward_w, bed_w, in_w, out_w, st_w);

        while (hit)
        {
            print_hospitalization(hit, visit_head, reg_head, p_head, ward_head, bed_head, h_w, v_w, p_w, ward_w, bed_w, in_w, out_w, st_w);
            hit = find_hospitalization_by_p_id(hit->next, g_session.user_id);
        }

        print_hospitalization_line(h_w, v_w, p_w, ward_w, bed_w, in_w, out_w, st_w);
    }

    free_hospitalizations(hosp_head);
    free_patients(p_head);
    free_registrations(reg_head);
    free_visits(visit_head);
    free_wards(ward_head);
    free_beds(bed_head);

    printf("\n");
    wait_enter();
    clear_screen();
}

/* 患者查看我的处方记录 */
void patient_view_my_prescription_records()
{
    // 1. 权限与登录态校验
    if (!g_session.logged_in || strcmp(g_session.role, "patient") != 0)
    {
        printf("系统错误：请先以患者身份登录！\n");
        wait_enter();
        return;
    }

    // 2. 加载所需的所有基础数据
    Prescription *pr_head = load_prescriptions_from_file();
    if (!pr_head)
    {
        printf("系统提示：暂无任何处方记录！\n");
        wait_enter();
        clear_screen();
        return;
    }

    Patient *p_head = load_patients_from_file();
    Doctor *d_head = load_doctors_from_file();
    Drug *drug_head = load_drugs_from_file();

    // 3. 统计属于当前患者的处方数量
    int total_matches = 0;
    for (Prescription *cur = pr_head; cur; cur = cur->next)
    {
        if (strcmp(cur->p_id, g_session.user_id) == 0)
        {
            total_matches++;
        }
    }

    // 如果没有找到该患者的处方
    if (total_matches == 0)
    {
        printf("您当前没有任何处方记录！\n");
        free_prescriptions(pr_head);
        free_patients(p_head);
        free_doctors(d_head);
        free_drugs(drug_head);
        wait_enter();
        clear_screen();
        return;
    }

    // 4. 将匹配的处方节点指针存入数组，方便后续分页提取
    Prescription **matches = (Prescription **)malloc(total_matches * sizeof(Prescription *));
    int idx = 0;
    for (Prescription *cur = pr_head; cur; cur = cur->next)
    {
        if (strcmp(cur->p_id, g_session.user_id) == 0)
        {
            matches[idx++] = cur;
        }
    }

    // 5. 分页显示逻辑
    int page_size = PAGE_SIZE;
    int total_pages = (total_matches + page_size - 1) / page_size;
    int current_page = 1;

    // 计算表格列宽（基于所有数据计算列宽，保证排版整齐统一）
    int pr_w, visit_w, d_w, p_w, drug_w, dose_w, freq_w;
    calc_prescription_width(pr_head, p_head, d_head, drug_head, &pr_w, &visit_w, &d_w, &p_w, &drug_w, &dose_w, &freq_w);

    while (1)
    {
        clear_screen();
        printf("===== 我的处方记录 (共 %d 条, 第 %d/%d 页) =====\n", total_matches, current_page, total_pages);

        // 打印表头
        print_prescription_header(pr_w, visit_w, d_w, p_w, drug_w, dose_w, freq_w);

        // 计算当前页的起始和结束索引
        int start = (current_page - 1) * page_size;
        int end = start + page_size;
        if (end > total_matches)
            end = total_matches;

        // 打印当前页的数据
        for (int i = start; i < end; i++)
        {
            print_prescription(matches[i], p_head, d_head, drug_head, pr_w, visit_w, d_w, p_w, drug_w, dose_w, freq_w);
        }

        // 打印表尾闭合线
        print_prescription_line(pr_w, visit_w, d_w, p_w, drug_w, dose_w, freq_w);

        // 交互控制
        printf("\n[n]下一页  [p]上一页  [q]退出\n> ");
        char buf[MAX_INPUT_LEN];
        safe_input(buf, sizeof(buf));

        if (strcmp(buf, "n") == 0 || strcmp(buf, "N") == 0)
        {
            if (current_page < total_pages)
                current_page++;
        }
        else if (strcmp(buf, "p") == 0 || strcmp(buf, "P") == 0)
        {
            if (current_page > 1)
                current_page--;
        }
        else if (strcmp(buf, "q") == 0 || strcmp(buf, "Q") == 0)
        {
            break;
        }
        else
        {
            printf("输入无效！\n");
            wait_enter();
        }
    }

    // 6. 清理内存
    free(matches);
    free_prescriptions(pr_head);
    free_patients(p_head);
    free_doctors(d_head);
    free_drugs(drug_head);
    clear_screen();
}
