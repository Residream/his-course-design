/*
 * 科室功能模块
 */
#include "model/department.h"
#include "core/utils.h"
#include "model/doctor.h"

/*
 * 科室基础操作
 */

/* 从文件中加载科室数据，文件格式: name(每行一个科室名) */
Department *load_departments_from_file(void)
{
    FILE *fp = fopen(DEPARTMENTS_FILE, "r");
    if (!fp)
        return NULL;

    Department *head = NULL, *tail = NULL;
    char line[MAX_LINE_LEN];

    while (fgets(line, sizeof(line), fp))
    {
        trim_newline(line);
        if (!line[0])
            continue;

        Department *node = (Department *)malloc(sizeof(Department));
        if (!node)
        {
            fclose(fp);
            free_departments(head);
            return NULL;
        }
        memset(node, 0, sizeof(Department));

        strncpy(node->name, line, sizeof(node->name) - 1);
        node->name[sizeof(node->name) - 1] = '\0';
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

/* 将科室数据保存到文件 */
int save_departments_to_file(Department *head)
{
    char tmp_path[MAX_LINE_LEN];
    FILE *fp = safe_fopen_tmp(DEPARTMENTS_FILE, tmp_path, sizeof(tmp_path));
    if (!fp)
        return -1;

    Department *current = head;
    while (current)
    {
        fprintf(fp, "%s\n", current->name);
        current = current->next;
    }

    return safe_fclose_commit(fp, tmp_path, DEPARTMENTS_FILE);
}

/* 释放科室数据内存 */
void free_departments(Department *head)
{
    Department *current = head;
    while (current)
    {
        Department *temp = current;
        current = current->next;
        free(temp);
    }
}

/* 根据名称查找科室 */
Department *find_department_by_name(Department *head, const char *name)
{
    Department *current = head;
    while (current)
    {
        if (strcmp(current->name, name) == 0)
            return current;
        current = current->next;
    }
    return NULL;
}

/* 校验科室是否合法 */
int is_valid_department(const char *dept)
{
    if (!dept || dept[0] == '\0')
        return 0;
    Department *dept_head = load_departments_from_file();
    if (!dept_head)
        return 0;
    Department *current = dept_head;
    int valid = 0;
    while (current)
    {
        if (strcmp(dept, current->name) == 0)
        {
            valid = 1;
            break;
        }
        current = current->next;
    }
    free_departments(dept_head);
    return valid;
}

/*
 * 科室系统功能
 */
void add_department()
{
    char name[MAX_NAME_LEN];

    while (1)
    {
        printf("请输入要添加的科室名称(输入0返回): ");
        safe_input(name, sizeof(name));

        if (strcmp(name, "0") == 0)
        {
            clear_screen();
            return;
        }

        if (name[0] == '\0')
        {
            printf("输入错误！科室名称不能为空，请重新输入。\n");
            continue;
        }

        Department *dept_head = load_departments_from_file();

        if (find_department_by_name(dept_head, name))
        {
            printf("输入错误！科室已存在，请重新输入。\n");
            free_departments(dept_head);
            continue;
        }

        Department new_dept;
        memset(&new_dept, 0, sizeof(Department));
        strncpy(new_dept.name, name, sizeof(new_dept.name) - 1);
        new_dept.name[sizeof(new_dept.name) - 1] = '\0';

        if (!dept_head)
        {
            dept_head = (Department *)malloc(sizeof(Department));
            if (!dept_head)
            {
                printf("内存分配失败！\n");
                return;
            }
            *dept_head = new_dept;
        }
        else
        {
            Department *new_node = (Department *)malloc(sizeof(Department));
            if (!new_node)
            {
                printf("内存分配失败！\n");
                free_departments(dept_head);
                return;
            }
            *new_node = new_dept;
            new_node->next = dept_head;
            dept_head = new_node;
        }

        if (save_departments_to_file(dept_head) != 0)
            printf("保存科室信息失败！\n");
        else
            printf("添加成功！\n");

        free_departments(dept_head);
        wait_enter();
        clear_screen();
    }
}

/*
 * 删除科室(管理员功能)
 * 安全检查: 拒绝删除仍有医生挂靠的科室
 * 需先将科室下所有医生转到其他科室才允许删除
 */
void delete_department()
{
    char name[MAX_NAME_LEN];
    printf("请输入要删除的科室名称(输入0返回): ");
    safe_input(name, sizeof(name));
    if (strcmp(name, "0") == 0)
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

    Department *current = dept_head, *prev = NULL;
    while (current)
    {
        if (strcmp(current->name, name) == 0)
        {
            printf("找到科室: %s，确定要删除吗？(y/n): ", current->name);
            char confirm[MAX_INPUT_LEN];
            safe_input(confirm, sizeof(confirm));
            if (strcmp(confirm, "n") == 0 || strcmp(confirm, "N") == 0) // 用户输入n/N取消删除
            {
                printf("已取消删除。\n");
                free_departments(dept_head);
                wait_enter();
                clear_screen();
                return;
            }
            else if (strcmp(confirm, "y") != 0 &&
                     strcmp(confirm, "Y") != 0) // 用户输入非y/Y/n/N的其他内容，视为无效输入并取消删除
            {
                printf("输入无效，已取消删除。\n");
                free_departments(dept_head);
                wait_enter();
                clear_screen();
                return;
            }
            else // 用户输入y/Y确认删除
            {
                /* 检查是否有医生属于该科室 */
                Doctor *doc_head = load_doctors_from_file();
                int has_doctor = 0;
                for (Doctor *doc = doc_head; doc; doc = doc->next)
                {
                    if (strcmp(doc->department, name) == 0)
                    {
                        has_doctor = 1;
                        break;
                    }
                }
                if (has_doctor)
                {
                    printf("无法删除！该科室下仍有关联的医生记录，请先处理相关医生。\n");
                    free_doctors(doc_head);
                    free_departments(dept_head);
                    wait_enter();
                    clear_screen();
                    return;
                }
                free_doctors(doc_head);

                if (prev)
                    prev->next = current->next;
                else
                    dept_head = current->next;

                free(current);

                if (save_departments_to_file(dept_head) != 0)
                    printf("保存科室信息失败！\n");
                else
                    printf("删除成功！\n");

                free_departments(dept_head);
                wait_enter();
                clear_screen();
                return;
            }
        }
        prev = current;
        current = current->next;
    }
    printf("未找到指定名称的科室！\n");
    free_departments(dept_head);
    wait_enter();
    clear_screen();
}

/* 显示科室医生 */
void show_department_doctors()
{
    char name[MAX_NAME_LEN];
    print_department_hint();
    printf("请输入科室名称(输入0返回): ");
    safe_input(name, sizeof(name));
    if (strcmp(name, "0") == 0)
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

    Department *current = dept_head;
    while (current)
    {
        if (strcmp(current->name, name) == 0)
        {
            printf("科室: %s\n", current->name);
            Doctor *doctor_head = load_doctors_from_file();
            if (!doctor_head)
            {
                printf("没有医生数据！\n");
                free_departments(dept_head);
                wait_enter();
                clear_screen();
                return;
            }
            int id_w, name_w, gen_w, dept_w;
            calc_doctor_width(doctor_head, &id_w, &name_w, &gen_w, &dept_w);
            print_doctor_header(id_w, name_w, gen_w, dept_w);
            Doctor *doc_current = doctor_head;
            int found = 0;
            while (doc_current)
            {
                if (strcmp(doc_current->department, current->name) == 0)
                {
                    print_doctor(doc_current, id_w, name_w, gen_w, dept_w);
                    found = 1;
                }
                doc_current = doc_current->next;
            }
            if (!found)
                printf("该科室暂无医生！\n");
            print_doctor_line(id_w, name_w, gen_w, dept_w);
            free_doctors(doctor_head);
            break;
        }
        current = current->next;
    }
    free_departments(dept_head);
    wait_enter();
    clear_screen();
}

/* 显示所有科室 */
void show_all_departments()
{
    Department *dept_head = load_departments_from_file();
    if (!dept_head)
    {
        printf("暂无科室数据\n");
        wait_enter();
        clear_screen();
        return;
    }

    printf("===== 科室列表 =====\n");
    Department *current = dept_head;
    while (current)
    {
        printf("- %s\n", current->name);
        current = current->next;
    }
    free_departments(dept_head);
    wait_enter();
    clear_screen();
}

/* 显示所有科室可选项 */
void print_department_hint()
{
    Department *dept_head = load_departments_from_file();
    if (!dept_head)
    {
        printf("没有科室数据！\n");
        return;
    }
    printf("可选科室: ");
    Department *current = dept_head;
    while (current)
    {
        printf("%s", current->name);
        if (current->next)
            printf(" / ");
        current = current->next;
    }
    printf("\n");
    free_departments(dept_head);
}