/*
 * 医生功能模块
 */
#include "model/doctor.h"
#include "core/auth.h"
#include "core/session.h"
#include "core/utils.h"
#include "model/department.h"
#include "model/patient.h"
#include "model/registration.h"
#include "model/visit.h"

/*
 * 医生基础操作
 */

/* 从文件中加载医生数据 */
Doctor *load_doctors_from_file(void)
{
    FILE *fp = fopen(DOCTORS_FILE, "r");
    if (!fp)
        return NULL;

    Doctor *head = NULL, *tail = NULL;
    char line[MAX_LINE_LEN];

    while (fgets(line, sizeof(line), fp))
    {
        trim_newline(line);
        if (!line[0])
            continue;

        Doctor *node = (Doctor *)malloc(sizeof(Doctor));
        if (!node)
        {
            fclose(fp);
            free_doctors(head);
            return NULL;
        }
        memset(node, 0, sizeof(Doctor));

        char *token = strtok(line, "|"); // id
        if (!token)
        {
            free(node);
            continue;
        }
        strncpy(node->id, token, sizeof(node->id) - 1);
        node->id[sizeof(node->id) - 1] = '\0';

        token = strtok(NULL, "|"); // name
        if (!token)
        {
            free(node);
            continue;
        }
        strncpy(node->name, token, sizeof(node->name) - 1);
        node->name[sizeof(node->name) - 1] = '\0';

        token = strtok(NULL, "|"); // gender
        if (!token)
        {
            free(node);
            continue;
        }
        strncpy(node->gender, token, sizeof(node->gender) - 1);
        node->gender[sizeof(node->gender) - 1] = '\0';

        token = strtok(NULL, "|"); // department
        if (!token)
        {
            free(node);
            continue;
        }
        strncpy(node->department, token, sizeof(node->department) - 1);
        node->department[sizeof(node->department) - 1] = '\0';

        token = strtok(NULL, "|"); // pwd_hash
        if (!token)
        {
            free(node);
            continue;
        }
        strncpy(node->pwd_hash, token, sizeof(node->pwd_hash) - 1);
        node->pwd_hash[sizeof(node->pwd_hash) - 1] = '\0';

        token = strtok(NULL, "|"); // salt
        if (!token)
        {
            free(node);
            continue;
        }
        strncpy(node->salt, token, sizeof(node->salt) - 1);
        node->salt[sizeof(node->salt) - 1] = '\0';

        node->next = NULL;
        if (!head)
            head = tail = node;
        else
        {
            tail->next = node;
            tail = node;
        }
    }

    fclose(fp);
    return head;
}

/* 将医生数据保存到文件 */
int save_doctors_to_file(Doctor *head)
{
    char tmp_path[MAX_LINE_LEN];
    FILE *fp = safe_fopen_tmp(DOCTORS_FILE, tmp_path, sizeof(tmp_path));
    if (!fp)
        return -1;

    Doctor *current = head;
    while (current)
    {
        fprintf(fp, "%s|%s|%s|%s|%s|%s\n", current->id, current->name, current->gender, current->department,
                current->pwd_hash, current->salt);
        current = current->next;
    }

    return safe_fclose_commit(fp, tmp_path, DOCTORS_FILE);
}

/* 释放医生数据内存 */
void free_doctors(Doctor *head)
{
    Doctor *current = head;
    while (current)
    {
        Doctor *temp = current;
        current = current->next;
        free(temp);
    }
}

/* 根据ID查找医生 */
Doctor *find_doctor_by_d_id(Doctor *head, const char *id)
{
    Doctor *current = head;
    while (current)
    {
        if (strcmp(current->id, id) == 0)
            return current;
        current = current->next;
    }
    return NULL;
}

/* 打印医生信息并对齐 */
void print_doctor_align(const char *s, int width)
{
    int w = str_width(s);
    printf("%s", s);

    for (int i = w; i < width; i++)
        printf(" ");
}

/* 打印医生表格分隔线 */
void print_doctor_line(int id_w, int name_w, int gen_w, int dept_w)
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
    for (int i = 0; i < dept_w + 2; i++)
        printf("-");
    printf("+\n");
}

/* 打印医生表头 */
void print_doctor_header(int id_w, int name_w, int gen_w, int dept_w)
{
    print_doctor_line(id_w, name_w, gen_w, dept_w);

    printf("| ");
    print_doctor_align("医生ID", id_w);
    printf(" | ");
    print_doctor_align("姓名", name_w);
    printf(" | ");
    print_doctor_align("性别", gen_w);
    printf(" | ");
    print_doctor_align("科室", dept_w);
    printf(" |\n");

    print_doctor_line(id_w, name_w, gen_w, dept_w);
}

/* 生成下一个医生ID(Dxxxx) */
int generate_next_doctor_id(Doctor *head)
{
    int max_id = 0;
    Doctor *current = head;
    while (current)
    {
        const char *id = current->id;
        if (id[0] == 'D')
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

/* 打印医生信息行 */
void print_doctor(Doctor *d, int id_w, int name_w, int gen_w, int dept_w)
{
    printf("| ");
    print_doctor_align(d->id, id_w);
    printf(" | ");
    print_doctor_align(d->name, name_w);
    printf(" | ");
    print_doctor_align(d->gender, gen_w);
    printf(" | ");
    print_doctor_align(d->department, dept_w);
    printf(" |\n");
}

/* 计算医生信息表格列宽 */
void calc_doctor_width(Doctor *head, int *id_w, int *name_w, int *gen_w, int *dept_w)
{
    *id_w = str_width("医生ID");
    *name_w = str_width("姓名");
    *gen_w = str_width("性别");
    *dept_w = str_width("科室");

    for (Doctor *d = head; d; d = d->next)
    {
        int w;
        w = str_width(d->id);
        if (w > *id_w)
            *id_w = w;

        w = str_width(d->name);
        if (w > *name_w)
            *name_w = w;

        w = str_width(d->gender);
        if (w > *gen_w)
            *gen_w = w;

        w = str_width(d->department);
        if (w > *dept_w)
            *dept_w = w;
    }
}

/* 统计医生数量 */
int count_doctors(Doctor *head)
{
    int count = 0;
    while (head)
    {
        count++;
        head = head->next;
    }
    return count;
}

/* 获取第n个医生节点 */
Doctor *get_nth_doctor(Doctor *head, int n)
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
 * 医生功能函数
 */

/* 创建医生 */
Doctor create_doctor(const char *id, const char *name, const char *gender, const char *department, const char *pwd_hash)
{
    Doctor doctor;
    memset(&doctor, 0, sizeof(Doctor));

    strncpy(doctor.id, id, sizeof(doctor.id) - 1);
    doctor.id[sizeof(doctor.id) - 1] = '\0';

    strncpy(doctor.name, name, sizeof(doctor.name) - 1);
    doctor.name[sizeof(doctor.name) - 1] = '\0';

    strncpy(doctor.gender, gender, sizeof(doctor.gender) - 1);
    doctor.gender[sizeof(doctor.gender) - 1] = '\0';

    strncpy(doctor.department, department, sizeof(doctor.department) - 1);
    doctor.department[sizeof(doctor.department) - 1] = '\0';

    strncpy(doctor.pwd_hash, pwd_hash, sizeof(doctor.pwd_hash) - 1);
    doctor.pwd_hash[sizeof(doctor.pwd_hash) - 1] = '\0';

    doctor.next = NULL;
    return doctor;
}

/* 尾插医生 */
void append_doctor(Doctor **head, Doctor *new_doctor)
{
    if (!head || !new_doctor)
        return;

    new_doctor->next = NULL;

    if (!*head)
    {
        *head = new_doctor;
        return;
    }

    Doctor *current = *head;
    while (current->next)
        current = current->next;
    current->next = new_doctor;
}

/*
 * 管理员系统功能（医生管理）
 */

/* 添加医生 */
void add_doctor()
{
    char name[MAX_NAME_LEN];
    char gender[MAX_GENDER_LEN];
    char department[MAX_INPUT_LEN];
    char password[MAX_INPUT_LEN];

    /* 姓名 */
    while (1)
    {
        printf("请输入医生姓名(汉字 | 输入0返回): ");
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
        printf("请输入医生性别(男/女 | 输入0返回): ");
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

    /* 科室 */
    while (1)
    {
        print_department_hint();
        printf("请输入医生科室(输入0返回): ");
        safe_input(department, sizeof(department));

        if (strcmp(department, "0") == 0)
            return;

        if (!is_valid_department(department))
        {
            printf("输入错误！科室不在可选范围，请重新输入。\n");
            continue;
        }
        break;
    }

    /* 密码 */
    while (1)
    {
        printf("请输入医生密码(输入0返回): ");
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

    Doctor *doctor_head = load_doctors_from_file();

    char id[MAX_ID_LEN];
    int next_num = generate_next_doctor_id(doctor_head);
    snprintf(id, sizeof(id), "D%04d", next_num);

    /* 生成盐与哈希 */
    unsigned char salt[SALT_RAW_LEN];
    char salt_hex[SALT_HEX_LEN];
    char pwd_hash[MAX_PWD_HASH];

    generate_salt(salt, SALT_RAW_LEN);
    bytes_to_hex(salt, SALT_RAW_LEN, salt_hex);
    build_pass_hash(salt, password, pwd_hash);

    Doctor *new_node = (Doctor *)malloc(sizeof(Doctor));
    if (!new_node)
    {
        printf("内存分配失败！\n");
        free_doctors(doctor_head);
        wait_enter();
        return;
    }

    *new_node = create_doctor(id, name, gender, department, pwd_hash);
    strncpy(new_node->salt, salt_hex, sizeof(new_node->salt) - 1);
    new_node->salt[sizeof(new_node->salt) - 1] = '\0';
    new_node->next = NULL;

    append_doctor(&doctor_head, new_node);

    if (save_doctors_to_file(doctor_head) != 0)
        printf("保存医生信息失败！\n");
    else
        printf("添加成功！添加的医生ID是: %s\n", id);

    free_doctors(doctor_head);
    wait_enter();
    clear_screen();
}

/* 删除医生 */
void delete_doctor()
{
    char id[MAX_ID_LEN];
    printf("请输入要删除的医生ID(输入0返回): ");
    safe_input(id, sizeof(id));
    if (strcmp(id, "0") == 0)
    {
        clear_screen();
        return;
    }

    Doctor *doctor_head = load_doctors_from_file();
    if (!doctor_head)
    {
        printf("没有医生数据！\n");
        wait_enter();
        clear_screen();
        return;
    }

    Doctor *current = doctor_head, *prev = NULL;
    while (current)
    {
        if (strcmp(current->id, id) == 0)
        {
            printf("找到医生: %s (%s, %s)，确定要删除吗？(y/n): ", current->name, current->id, current->department);

            char confirm[MAX_INPUT_LEN];
            safe_input(confirm, sizeof(confirm));

            if (strcmp(confirm, "n") == 0 || strcmp(confirm, "N") == 0)
            {
                printf("已取消删除。\n");
                free_doctors(doctor_head);
                wait_enter();
                clear_screen();
                return;
            }
            else if (strcmp(confirm, "y") != 0 && strcmp(confirm, "Y") != 0)
            {
                printf("输入无效，已取消删除。\n");
                free_doctors(doctor_head);
                wait_enter();
                clear_screen();
                return;
            }

            // 检查是否有关联的未完成挂号记录
            Registration *reg_head = load_registrations_from_file();
            int has_reg = 0;
            for (Registration *r = reg_head; r; r = r->next)
            {
                if (strcmp(r->d_id, id) == 0 && r->status == REG_STATUS_PENDING)
                {
                    has_reg = 1;
                    break;
                }
            }
            if (has_reg)
            {
                printf("无法删除！该医生仍有未完成的挂号记录，请先处理相关挂号。\n");
                free_registrations(reg_head);
                free_doctors(doctor_head);
                wait_enter();
                clear_screen();
                return;
            }
            free_registrations(reg_head);

            if (prev)
                prev->next = current->next;
            else
                doctor_head = current->next;

            free(current);

            if (save_doctors_to_file(doctor_head) != 0)
                printf("保存医生信息失败！\n");
            else
                printf("删除成功！\n");

            free_doctors(doctor_head);
            wait_enter();
            clear_screen();
            return;
        }

        prev = current;
        current = current->next;
    }

    printf("未找到指定ID的医生！\n");
    free_doctors(doctor_head);
    wait_enter();
    clear_screen();
}

/* 更新医生信息 */
void update_doctor()
{
    char id[MAX_ID_LEN];
    printf("请输入要更新的医生ID(输入0返回): ");
    safe_input(id, sizeof(id));
    if (strcmp(id, "0") == 0)
    {
        clear_screen();
        return;
    }

    Doctor *doctor_head = load_doctors_from_file();
    if (!doctor_head)
    {
        printf("没有医生数据！\n");
        wait_enter();
        clear_screen();
        return;
    }

    Doctor *doctor = find_doctor_by_d_id(doctor_head, id);
    if (!doctor)
    {
        printf("未找到指定ID的医生！\n");
        free_doctors(doctor_head);
        wait_enter();
        clear_screen();
        return;
    }

    int id_w, name_w, gen_w, dept_w;
    calc_doctor_width(doctor_head, &id_w, &name_w, &gen_w, &dept_w);

    char buf[MAX_INPUT_LEN];
    int select;

    while (1)
    {
        clear_screen();
        printf("找到医生: %s (%s)\n", doctor->name, doctor->id);
        print_doctor_header(id_w, name_w, gen_w, dept_w);
        print_doctor(doctor, id_w, name_w, gen_w, dept_w);
        print_doctor_line(id_w, name_w, gen_w, dept_w);

        printf("请选择要更新的信息:\n");
        printf("1. 姓名\n");
        printf("2. 性别\n");
        printf("3. 科室\n");
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
                printf("请输入医生姓名(汉字 | 输入0返回): ");
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

                strncpy(doctor->name, name, sizeof(doctor->name) - 1);
                doctor->name[sizeof(doctor->name) - 1] = '\0';

                if (save_doctors_to_file(doctor_head) != 0)
                    printf("保存医生信息失败！\n");
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
                printf("请输入医生性别(男/女 | 输入0返回): ");
                safe_input(gender, sizeof(gender));

                if (strcmp(gender, "0") == 0)
                    break;

                if (strcmp(gender, "男") != 0 && strcmp(gender, "女") != 0)
                {
                    printf("输入错误！性别只能为男或女。\n");
                    continue;
                }

                strncpy(doctor->gender, gender, sizeof(doctor->gender) - 1);
                doctor->gender[sizeof(doctor->gender) - 1] = '\0';

                if (save_doctors_to_file(doctor_head) != 0)
                    printf("保存医生信息失败！\n");
                else
                    printf("性别更新成功！\n");

                wait_enter();
                break;
            }
            break;
        }
        /* 更新科室 */
        case 3:
        {
            char department[MAX_INPUT_LEN];
            while (1)
            {
                print_department_hint();
                printf("请输入医生科室(输入0返回): ");
                safe_input(department, sizeof(department));

                if (strcmp(department, "0") == 0)
                    break;

                if (!is_valid_department(department))
                {
                    printf("输入错误！科室不在可选范围，请重新输入。\n");
                    continue;
                }

                strncpy(doctor->department, department, sizeof(doctor->department) - 1);
                doctor->department[sizeof(doctor->department) - 1] = '\0';

                if (save_doctors_to_file(doctor_head) != 0)
                    printf("保存医生信息失败！\n");
                else
                    printf("科室更新成功！\n");

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

                printf("请输入医生新密码(输入0返回): ");
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

                strncpy(doctor->salt, new_salt_hex, sizeof(doctor->salt) - 1);
                doctor->salt[sizeof(doctor->salt) - 1] = '\0';
                strncpy(doctor->pwd_hash, new_pwd_hash, sizeof(doctor->pwd_hash) - 1);
                doctor->pwd_hash[sizeof(doctor->pwd_hash) - 1] = '\0';

                if (save_doctors_to_file(doctor_head) != 0)
                    printf("保存医生信息失败！\n");
                else
                    printf("密码更新成功！\n");

                wait_enter();
                break;
            }
            break;
        }
        case 0:
            printf("欢迎下次使用\n");
            free_doctors(doctor_head);
            wait_enter();
            clear_screen();
            return;
        default:
            printf("未知错误: %d\n", select);
            break;
        }
    }
}

/* 查询医生信息（支持ID精确查询、姓名和科室模糊查询） */
void query_doctor()
{
    Doctor *doctor_head = load_doctors_from_file();
    if (!doctor_head)
    {
        printf("没有医生数据！\n");
        wait_enter();
        clear_screen();
        return;
    }

    char buf[MAX_INPUT_LEN];
    while (1)
    {
        clear_screen();
        printf("===== 查询医生信息 =====\n");
        printf("1. 按医生ID精确查询\n");
        printf("2. 按姓名模糊查询\n");
        printf("3. 按科室模糊查询\n");
        printf("0. 返回\n");
        printf("请输入您的选择: ");
        safe_input(buf, sizeof(buf));

        if (!validate_choice(buf, 3))
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

        int id_w, name_w, gen_w, dept_w;
        calc_doctor_width(doctor_head, &id_w, &name_w, &gen_w, &dept_w);

        int found = 0;
        for (Doctor *cur = doctor_head; cur; cur = cur->next)
        {
            int match = 0;
            if (select == 1 && strcmp(cur->id, query) == 0)
                match = 1;
            else if (select == 2 && strstr(cur->name, query) != NULL)
                match = 1;
            else if (select == 3 && strstr(cur->department, query) != NULL)
                match = 1;

            if (match)
            {
                if (!found)
                {
                    printf("查询结果:\n");
                    print_doctor_header(id_w, name_w, gen_w, dept_w);
                    found = 1;
                }
                print_doctor(cur, id_w, name_w, gen_w, dept_w);
            }
        }

        if (found)
            print_doctor_line(id_w, name_w, gen_w, dept_w);
        else
            printf("未找到匹配的医生！\n");

        wait_enter();
    }

    free_doctors(doctor_head);
    clear_screen();
}

/* 显示所有医生信息 */
void show_all_doctors()
{
    Doctor *doctor_head = load_doctors_from_file();
    if (!doctor_head)
    {
        printf("暂无医生数据\n");
        wait_enter();
        clear_screen();
        return;
    }

    int page_size = PAGE_SIZE;
    int total = count_doctors(doctor_head);
    int total_pages = (total + page_size - 1) / page_size;

    int current_page = 1;

    int id_w, name_w, gen_w, dept_w;
    calc_doctor_width(doctor_head, &id_w, &name_w, &gen_w, &dept_w);

    while (1)
    {
        clear_screen();

        printf("===== 医生列表（第 %d/%d 页）=====\n", current_page, total_pages);

        print_doctor_header(id_w, name_w, gen_w, dept_w);

        int start = (current_page - 1) * page_size;
        Doctor *cur = get_nth_doctor(doctor_head, start);

        for (int i = 0; i < page_size && cur; i++)
        {
            print_doctor(cur, id_w, name_w, gen_w, dept_w);
            cur = cur->next;
        }

        print_doctor_line(id_w, name_w, gen_w, dept_w);

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

    free_doctors(doctor_head);
    clear_screen();
}

/* 医生查看个人信息 */
void doctor_view_my_info()
{
    if (!g_session.logged_in || strcmp(g_session.role, "doctor") != 0)
    {
        printf("请先登录医生账号！\n");
        wait_enter();
        clear_screen();
        return;
    }

    Doctor *doctor_head = load_doctors_from_file();
    if (!doctor_head)
    {
        printf("没有医生数据！\n");
        wait_enter();
        clear_screen();
        return;
    }

    Doctor *me = find_doctor_by_d_id(doctor_head, g_session.user_id);
    if (!me)
    {
        printf("未找到当前登录医生信息(ID: %s)\n", g_session.user_id);
        free_doctors(doctor_head);
        wait_enter();
        clear_screen();
        return;
    }

    int id_w, name_w, gen_w, dept_w;
    calc_doctor_width(doctor_head, &id_w, &name_w, &gen_w, &dept_w);

    printf("我的信息：\n");
    print_doctor_header(id_w, name_w, gen_w, dept_w);
    print_doctor(me, id_w, name_w, gen_w, dept_w);
    print_doctor_line(id_w, name_w, gen_w, dept_w);

    free_doctors(doctor_head);
    wait_enter();
    clear_screen();
}

/* 医生修改个人信息 */
void doctor_update_my_info()
{
    if (!g_session.logged_in || strcmp(g_session.role, "doctor") != 0)
    {
        printf("请先登录医生账号！\n");
        wait_enter();
        clear_screen();
        return;
    }

    Doctor *doctor_head = load_doctors_from_file();
    if (!doctor_head)
    {
        printf("没有医生数据！\n");
        wait_enter();
        clear_screen();
        return;
    }

    Doctor *me = find_doctor_by_d_id(doctor_head, g_session.user_id);
    if (!me)
    {
        printf("未找到当前登录医生信息(ID: %s)\n", g_session.user_id);
        free_doctors(doctor_head);
        wait_enter();
        clear_screen();
        return;
    }

    int id_w, name_w, gen_w, dept_w;
    calc_doctor_width(doctor_head, &id_w, &name_w, &gen_w, &dept_w);

    char buf[MAX_INPUT_LEN];
    int select;

    while (1)
    {
        clear_screen();
        printf("我的信息：\n");
        print_doctor_header(id_w, name_w, gen_w, dept_w);
        print_doctor(me, id_w, name_w, gen_w, dept_w);
        print_doctor_line(id_w, name_w, gen_w, dept_w);

        printf("请选择要更新的信息:\n");
        printf("1. 姓名\n");
        printf("2. 性别\n");
        printf("3. 科室\n");
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
                printf("请输入医生姓名(汉字 | 输入0返回): ");
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

                if (save_doctors_to_file(doctor_head) != 0)
                    printf("保存医生信息失败！\n");
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
                printf("请输入医生性别(男/女 | 输入0返回): ");
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

                if (save_doctors_to_file(doctor_head) != 0)
                    printf("保存医生信息失败！\n");
                else
                    printf("性别更新成功！\n");

                wait_enter();
                break;
            }
            break;
        }
        /* 更新科室 */
        case 3:
        {
            char department[MAX_INPUT_LEN];
            while (1)
            {
                print_department_hint();
                printf("请输入医生科室(输入0返回): ");
                safe_input(department, sizeof(department));

                if (strcmp(department, "0") == 0)
                    break;

                if (!is_valid_department(department))
                {
                    printf("输入错误！科室不在可选范围，请重新输入。\n");
                    continue;
                }

                strncpy(me->department, department, sizeof(me->department) - 1);
                me->department[sizeof(me->department) - 1] = '\0';

                if (save_doctors_to_file(doctor_head) != 0)
                    printf("保存医生信息失败！\n");
                else
                    printf("科室更新成功！\n");

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

                printf("请输入医生旧密码(输入0返回): ");
                safe_input(old_password, sizeof(old_password));

                if (strcmp(old_password, "0") == 0)
                    break;

                if (!verify_password(old_password, me->salt, me->pwd_hash))
                {
                    printf("输入错误！旧密码不正确，请重新输入。\n");
                    continue;
                }

                int cancel_new_pwd = 0;
                while (1)
                {
                    printf("请输入医生新密码(输入0返回): ");
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

                if (save_doctors_to_file(doctor_head) != 0)
                    printf("保存医生信息失败！\n");
                else
                    printf("密码更新成功！\n");

                wait_enter();
                break;
            }
            break;
        }
        case 0:
            printf("欢迎下次使用\n");
            free_doctors(doctor_head);
            wait_enter();
            clear_screen();
            return;
        default:
            printf("未知错误: %d\n", select);
            break;
        }
    }
}

/* 医生查看挂号记录 */
void doctor_view_patient_registrations()
{
    if (!g_session.logged_in || strcmp(g_session.role, "doctor") != 0)
    {
        printf("请先登录医生账号！\n");
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

    int page_size = PAGE_SIZE;
    int total = count_registrations_for_doctor(reg_head, g_session.user_id);
    if (total == 0)
    {
        printf("暂无您的挂号记录！\n");
        free_registrations(reg_head);
        free_patients(p_head);
        free_doctors(d_head);
        wait_enter();
        clear_screen();
        return;
    }

    int total_pages = (total + page_size - 1) / page_size;
    int current_page = 1;

    int reg_w, p_w, d_w, dept_w, when_w, st_w;
    calc_registration_width(reg_head, p_head, d_head, &reg_w, &p_w, &d_w, &dept_w, &when_w, &st_w);

    while (1)
    {
        clear_screen();
        printf("===== 挂号列表（第 %d/%d 页）=====\n", current_page, total_pages);
        print_registration_header(reg_w, p_w, d_w, dept_w, when_w, st_w);

        int start = (current_page - 1) * page_size;
        Registration *cur = get_nth_registration_for_doctor(reg_head, g_session.user_id, start);

        int shown = 0;
        while (cur && shown < page_size)
        {
            print_registration(cur, p_head, d_head, reg_w, p_w, d_w, dept_w, when_w, st_w);
            shown++;
            cur = find_registration_by_d_id(cur->next, g_session.user_id);
        }

        print_registration_line(reg_w, p_w, d_w, dept_w, when_w, st_w);
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

    free_registrations(reg_head);
    free_patients(p_head);
    free_doctors(d_head);
    wait_enter();
    clear_screen();
}
