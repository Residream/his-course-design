/*
 *检查模块
 */
#include "model/exam.h"
#include "core/utils.h"
#include "model/doctor.h"
#include "model/patient.h"
#include "model/registration.h"
#include "model/visit.h"

/*
 * 检查基础功能
 */

/* 从文件中加载检查数据，文件格式: exam_id|visit_id|item|result */
Exam *load_exams_from_file(void)
{
    FILE *fp = fopen(EXAMS_FILE, "r");
    if (!fp)
        return NULL;

    Exam *head = NULL, *tail = NULL;
    char line[MAX_LINE_LEN];

    while (fgets(line, sizeof(line), fp))
    {
        trim_newline(line);
        if (!line[0])
            continue;

        Exam *node = (Exam *)malloc(sizeof(Exam));
        if (!node)
        {
            fclose(fp);
            free_exams(head);
            return NULL;
        }
        memset(node, 0, sizeof(Exam));

        /* 获取exam_id */
        char *token = strtok(line, "|");
        if (!token)
        {
            free(node);
            continue;
        }
        strncpy(node->exam_id, token, sizeof(node->exam_id) - 1);
        node->exam_id[sizeof(node->exam_id) - 1] = '\0';

        /* 获取visit_id */
        token = strtok(NULL, "|");
        if (!token)
        {
            free(node);
            continue;
        }
        strncpy(node->visit_id, token, sizeof(node->visit_id) - 1);
        node->visit_id[sizeof(node->visit_id) - 1] = '\0';

        /* 获取item */
        token = strtok(NULL, "|");
        if (!token)
        {
            free(node);
            continue;
        }
        strncpy(node->item, token, sizeof(node->item) - 1);
        node->item[sizeof(node->item) - 1] = '\0';

        /* 获取result（可选） */
        token = strtok(NULL, "|");
        if (token)
        {
            strncpy(node->result, token, sizeof(node->result) - 1);
            node->result[sizeof(node->result) - 1] = '\0';
        }
        else
        {
            node->result[0] = '\0';
        }

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

/* 将检查数据保存到文件 */
int save_exams_to_file(Exam *head)
{
    char tmp_path[MAX_LINE_LEN];
    FILE *fp = safe_fopen_tmp(EXAMS_FILE, tmp_path, sizeof(tmp_path));
    if (!fp)
        return -1;

    Exam *current = head;
    while (current)
    {
        fprintf(fp, "%s|%s|%s|%s\n", current->exam_id, current->visit_id, current->item, current->result);
        current = current->next;
    }

    return safe_fclose_commit(fp, tmp_path, EXAMS_FILE);
}

/* 释放检查数据内存 */
void free_exams(Exam *head)
{
    Exam *current = head;
    while (current)
    {
        Exam *temp = current;
        current = current->next;
        free(temp);
    }
}

/* 根据检查ID查找检查 */
Exam *find_exam_by_e_id(Exam *exam_head, const char *exam_id)
{
    if (!exam_id)
        return NULL;
    for (Exam *e = exam_head; e; e = e->next)
        if (strcmp(e->exam_id, exam_id) == 0)
            return e;
    return NULL;
}

/* 根据看诊ID查找检查 */
Exam *find_exam_by_v_id(Exam *exam_head, const char *visit_id)
{
    if (!visit_id)
        return NULL;
    for (Exam *e = exam_head; e; e = e->next)
        if (strcmp(e->visit_id, visit_id) == 0)
            return e;
    return NULL;
}

/* 根据患者ID查找检查(需经三表关联: exam->visit->reg->p_id) */
Exam *find_exam_by_p_id(Exam *exam_head, Visit *visit_head, Registration *reg_head, const char *p_id)
{
    if (!p_id)
        return NULL;
    for (Exam *e = exam_head; e; e = e->next)
    {
        Visit *v = find_visit_by_v_id(visit_head, e->visit_id);
        if (!v)
            continue;
        Registration *r = find_registration_by_r_id(reg_head, v->reg_id);
        if (!r)
            continue;
        if (strcmp(r->p_id, p_id) == 0)
            return e;
    }
    return NULL;
}

/* 根据医生ID查找检查 */
Exam *find_exam_by_d_id(Exam *exam_head, Visit *visit_head, Registration *reg_head, const char *d_id)
{
    if (!d_id)
        return NULL;
    for (Exam *e = exam_head; e; e = e->next)
    {
        Visit *v = find_visit_by_v_id(visit_head, e->visit_id);
        if (!v)
            continue;
        Registration *r = find_registration_by_r_id(reg_head, v->reg_id);
        if (!r)
            continue;
        if (strcmp(r->d_id, d_id) == 0)
            return e;
    }
    return NULL;
}

/* 生成下一个检查ID */
int generate_next_exam_id(Exam *head)
{
    int max_id = 0;
    Exam *current = head;
    while (current)
    {
        const char *id = current->exam_id;
        if (id[0] == 'E')
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
                int num = atoi(id + 1);
                if (num > max_id)
                    max_id = num;
            }
        }
        current = current->next;
    }
    return max_id + 1;
}

/* 根据患者ID获取患者姓名 */
static const char *get_patient_name_by_p_id(Patient *head, const char *p_id)
{
    for (Patient *p = head; p; p = p->next)
        if (strcmp(p->id, p_id) == 0)
            return p->name;
    return p_id; // 找不到回退显示ID
}

/* 打印检查信息行（类似 print_registration） */
void print_exam(Exam *e, Visit *v_head, Registration *r_head, Patient *p_head, Doctor *d_head, int exam_w, int visit_w,
                int p_w, int d_w, int dept_w, int item_w, int result_w)
{
    const char *p_name = "未知患者";
    const char *d_name = "未知医生";
    const char *dept = "未知科室";

    Visit *v = find_visit_by_v_id(v_head, e->visit_id);
    if (v)
    {
        Registration *r = find_registration_by_r_id(r_head, v->reg_id);
        if (r)
        {
            p_name = get_patient_name_by_p_id(p_head, r->p_id);

            Doctor *doc = find_doctor_by_d_id(d_head, r->d_id);
            if (doc)
            {
                d_name = doc->name;
                dept = doc->department;
            }
            else if (r->d_id[0] != '\0')
            {
                d_name = r->d_id; // 回退显示ID
            }
        }
    }

    printf("| ");
    print_align(e->exam_id, exam_w);
    printf(" | ");
    print_align(e->visit_id, visit_w);
    printf(" | ");
    print_align(p_name, p_w);
    printf(" | ");
    print_align(d_name, d_w);
    printf(" | ");
    print_align(dept, dept_w);
    printf(" | ");
    print_align(e->item, item_w);
    printf(" | ");
    print_align((e->result[0] ? e->result : "暂无"), result_w);
    printf(" |\n");
}

/* 打印检查表头 */
void print_exam_header(int exam_w, int visit_w, int p_w, int d_w, int dept_w, int item_w, int result_w)
{
    print_exam_line(exam_w, visit_w, p_w, d_w, dept_w, item_w, result_w);

    printf("| ");
    print_align("检查ID", exam_w);
    printf(" | ");
    print_align("看诊ID", visit_w);
    printf(" | ");
    print_align("患者姓名", p_w);
    printf(" | ");
    print_align("医生姓名", d_w);
    printf(" | ");
    print_align("科室", dept_w);
    printf(" | ");
    print_align("检查项目", item_w);
    printf(" | ");
    print_align("检查结果", result_w);
    printf(" |\n");

    print_exam_line(exam_w, visit_w, p_w, d_w, dept_w, item_w, result_w);
}

/* 打印检查分隔线 */
void print_exam_line(int exam_w, int visit_w, int p_w, int d_w, int dept_w, int item_w, int result_w)
{
    printf("+");
    for (int i = 0; i < exam_w + 2; i++)
        printf("-");
    printf("+");
    for (int i = 0; i < visit_w + 2; i++)
        printf("-");
    printf("+");
    for (int i = 0; i < p_w + 2; i++)
        printf("-");
    printf("+");
    for (int i = 0; i < d_w + 2; i++)
        printf("-");
    printf("+");
    for (int i = 0; i < dept_w + 2; i++)
        printf("-");
    printf("+");
    for (int i = 0; i < item_w + 2; i++)
        printf("-");
    printf("+");
    for (int i = 0; i < result_w + 2; i++)
        printf("-");
    printf("+\n");
}

/* 计算检查表格列宽 */
void calc_exam_width(Exam *e_head, Visit *v_head, Registration *r_head, Patient *p_head, Doctor *d_head, int *exam_w,
                     int *visit_w, int *p_w, int *d_w, int *dept_w, int *item_w, int *result_w)
{
    *exam_w = str_width("检查ID");
    *visit_w = str_width("看诊ID");
    *p_w = str_width("患者姓名");
    *d_w = str_width("医生姓名");
    *dept_w = str_width("科室");
    *item_w = str_width("检查项目");
    *result_w = str_width("检查结果");

    for (Exam *e = e_head; e; e = e->next)
    {
        int w;

        const char *p_name = "未知患者";
        const char *d_name = "未知医生";
        const char *dept = "未知科室";

        Visit *v = find_visit_by_v_id(v_head, e->visit_id);
        if (v)
        {
            Registration *r = find_registration_by_r_id(r_head, v->reg_id);
            if (r)
            {
                Patient *p = find_patient_by_p_id(p_head, r->p_id);
                if (p)
                    p_name = p->name;

                Doctor *doc = find_doctor_by_d_id(d_head, r->d_id);
                if (doc)
                {
                    d_name = doc->name;
                    dept = doc->department;
                }
                else if (r->d_id[0] != '\0')
                {
                    d_name = r->d_id;
                }
            }
        }

        w = str_width(e->exam_id);
        if (w > *exam_w)
            *exam_w = w;

        w = str_width(e->visit_id);
        if (w > *visit_w)
            *visit_w = w;

        w = str_width(p_name);
        if (w > *p_w)
            *p_w = w;

        w = str_width(d_name);
        if (w > *d_w)
            *d_w = w;

        w = str_width(dept);
        if (w > *dept_w)
            *dept_w = w;

        w = str_width(e->item);
        if (w > *item_w)
            *item_w = w;

        w = str_width(e->result[0] ? e->result : "暂无");
        if (w > *result_w)
            *result_w = w;
    }
}

/* 统计检查数量 */
int count_exams(Exam *head)
{
    int count = 0;
    for (Exam *e = head; e; e = e->next)
        count++;
    return count;
}

/* 获取第n个检查节点 */
Exam *get_nth_exam(Exam *head, int n)
{
    Exam *e = head;
    for (int i = 0; i < n && e; i++)
        e = e->next;
    return e;
}

/*
 *检查功能函数
 */

/* 创建检查 */
Exam create_exam(const char *exam_id, const char *visit_id, const char *item, const char *result)
{
    Exam exam;
    memset(&exam, 0, sizeof(Exam));

    strncpy(exam.exam_id, exam_id, sizeof(exam.exam_id) - 1);
    exam.exam_id[sizeof(exam.exam_id) - 1] = '\0';

    strncpy(exam.visit_id, visit_id, sizeof(exam.visit_id) - 1);
    exam.visit_id[sizeof(exam.visit_id) - 1] = '\0';

    strncpy(exam.item, item, sizeof(exam.item) - 1);
    exam.item[sizeof(exam.item) - 1] = '\0';

    strncpy(exam.result, result, sizeof(exam.result) - 1);
    exam.result[sizeof(exam.result) - 1] = '\0';

    exam.next = NULL;
    return exam;
}

/* 尾插检查 */
void append_exam(Exam **head, Exam *new_exam)
{
    if (!head || !new_exam)
        return;

    new_exam->next = NULL;

    if (!*head)
    {
        *head = new_exam;
        return;
    }

    Exam *current = *head;
    while (current->next)
        current = current->next;
    current->next = new_exam;
}

/*
 * 检查系统功能(增删改查)
 */

/* 添加检查记录(管理员/医生功能, 需指定看诊ID关联) */
void add_exam_record()
{
    char exam_id[MAX_ID_LEN];
    char visit_id[MAX_ID_LEN];
    char item[MAX_ITEM_LEN];
    char result[MAX_LINE_LEN];
    Exam *e_head = load_exams_from_file();
    Visit *v_head = load_visits_from_file();

    /* 获取看诊ID */
    while (1)
    {
        printf("请输入看诊ID(输入0返回): ");
        safe_input(visit_id, sizeof(visit_id));
        if (strcmp(visit_id, "0") == 0)
            break;

        if (visit_id[0] == '\0')
        {
            printf("输入错误！看诊ID不能为空。\n");
            wait_enter();
            clear_screen();
            break;
        }

        if (!find_visit_by_v_id(v_head, visit_id))
        {
            printf("未找到该看诊ID！\n");
            wait_enter();
            clear_screen();
            break;
        }

        /* 获取检查项目 */
        printf("请输入检查项目(输入0返回): ");
        safe_input(item, sizeof(item));
        if (strcmp(item, "0") == 0)
            break;

        if (item[0] == '\0')
        {
            printf("输入错误！检查项目不能为空。\n");
            wait_enter();
            clear_screen();
            break;
        }

        int next_id_num = generate_next_exam_id(e_head);
        snprintf(exam_id, sizeof(exam_id), "E%04d", next_id_num);

        printf("请输入检查结果(输入0返回): ");
        safe_input(result, sizeof(result));
        if (strcmp(result, "0") == 0)
            break;

        Exam *new_node = (Exam *)malloc(sizeof(Exam));
        if (!new_node)
        {
            printf("内存分配失败！\n");
            free_exams(e_head);
            wait_enter();
            clear_screen();
            break;
        }
        *new_node = create_exam(exam_id, visit_id, item, result);
        new_node->next = NULL;
        append_exam(&e_head, new_node);
        if (save_exams_to_file(e_head) != 0)
            printf("保存检查信息失败！\n");
        else
            printf("检查记录添加成功！检查ID: %s\n", exam_id);
        wait_enter();
        clear_screen();
        break;
    }

    free_exams(e_head);
    free_visits(v_head);
}

/* 删除检查记录 */
void delete_exam_record()
{
    char exam_id[MAX_ID_LEN];
    Exam *e_head = load_exams_from_file();
    Visit *v_head = load_visits_from_file();
    Registration *r_head = load_registrations_from_file();
    Patient *p_head = load_patients_from_file();
    Doctor *d_head = load_doctors_from_file();

    while (1)
    {
        printf("请输入要删除的检查ID(输入0返回): ");
        safe_input(exam_id, sizeof(exam_id));
        if (strcmp(exam_id, "0") == 0)
            break;

        if (exam_id[0] == '\0')
        {
            printf("输入错误！检查ID不能为空。\n");
            wait_enter();
            clear_screen();
            break;
        }

        Exam *current = e_head, *prev = NULL;
        while (current)
        {
            if (strcmp(current->exam_id, exam_id) == 0)
            {
                int exam_w, visit_w, p_w, d_w, dept_w, item_w, result_w;
                calc_exam_width(e_head, v_head, r_head, p_head, d_head, &exam_w, &visit_w, &p_w, &d_w, &dept_w, &item_w,
                                &result_w);

                print_exam_header(exam_w, visit_w, p_w, d_w, dept_w, item_w, result_w);
                print_exam(current, v_head, r_head, p_head, d_head, exam_w, visit_w, p_w, d_w, dept_w, item_w,
                           result_w);
                print_exam_line(exam_w, visit_w, p_w, d_w, dept_w, item_w, result_w);

                printf("确定要删除该检查记录吗？(y/n): ");
                char choice[4];
                safe_input(choice, sizeof(choice));
                if (strcmp(choice, "y") != 0 && strcmp(choice, "Y") != 0)
                {
                    printf("已取消删除。\n");
                    free_visits(v_head);
                    free_registrations(r_head);
                    free_patients(p_head);
                    free_doctors(d_head);
                    free_exams(e_head);
                    wait_enter();
                    clear_screen();
                    return;
                }
                if (prev)
                    prev->next = current->next;
                else
                    e_head = current->next;

                free(current);

                if (save_exams_to_file(e_head) != 0)
                    printf("保存检查信息失败！\n");
                else
                    printf("删除成功！\n");
                wait_enter();
                clear_screen();

                free_exams(e_head);
                free_visits(v_head);
                free_registrations(r_head);
                free_patients(p_head);
                free_doctors(d_head);
                return;
            }
            prev = current;
            current = current->next;
        }
        printf("未找到指定ID的检查记录！\n");
        wait_enter();
        clear_screen();
        break;
    }
    free_exams(e_head);
    free_visits(v_head);
    free_registrations(r_head);
    free_patients(p_head);
    free_doctors(d_head);
}

/* 更新检查记录 */
void update_exam_record()
{
    char exam_id[MAX_ID_LEN];
    char buf[MAX_INPUT_LEN];
    int select;

    Exam *e_head = load_exams_from_file();
    if (!e_head)
    {
        printf("暂无检查记录可更新！\n");
        wait_enter();
        clear_screen();
        return;
    }

    printf("请输入要更新的检查ID(输入0返回): ");
    safe_input(exam_id, sizeof(exam_id));

    if (strcmp(exam_id, "0") == 0)
    {
        clear_screen();
        free_exams(e_head);
        return;
    }

    if (exam_id[0] == '\0')
    {
        printf("输入错误！检查ID不能为空。\n");
        wait_enter();
        clear_screen();
        free_exams(e_head);
        return;
    }

    Exam *current = NULL;
    for (current = e_head; current; current = current->next)
    {
        if (strcmp(current->exam_id, exam_id) != 0)
            continue;

        Visit *v_head = load_visits_from_file();
        Registration *r_head = load_registrations_from_file();
        Patient *p_head = load_patients_from_file();
        Doctor *d_head = load_doctors_from_file();

        int done = 0;
        while (!done)
        {
            int exam_w, visit_w, p_w, d_w, dept_w, item_w, result_w;
            clear_screen();
            calc_exam_width(e_head, v_head, r_head, p_head, d_head, &exam_w, &visit_w, &p_w, &d_w, &dept_w, &item_w,
                            &result_w);

            printf("找到检查记录，信息如下：\n");
            print_exam_header(exam_w, visit_w, p_w, d_w, dept_w, item_w, result_w);
            print_exam(current, v_head, r_head, p_head, d_head, exam_w, visit_w, p_w, d_w, dept_w, item_w, result_w);
            print_exam_line(exam_w, visit_w, p_w, d_w, dept_w, item_w, result_w);

            printf("\n请选择要更新的内容:\n");
            printf("1. 更新检查项目 (当前: %s)\n", current->item);
            printf("2. 更新检查结果 (当前: %s)\n", current->result[0] ? current->result : "暂无");
            printf("0. 返回\n");
            printf("请输入您的选择: ");
            safe_input(buf, sizeof(buf));

            if (!validate_choice(buf, 2))
            {
                printf("输入有误，请重新选择！\n");
                wait_enter();
                continue;
            }

            select = atoi(buf);

            if (select == 0)
            {
                done = 1;
                continue;
            }

            if (select == 1)
            {
                char item[MAX_ITEM_LEN];
                printf("请输入新的检查项目(输入0取消本次修改): ");
                safe_input(item, sizeof(item));

                if (strcmp(item, "0") != 0 && item[0] != '\0')
                {
                    strncpy(current->item, item, sizeof(current->item) - 1);
                    current->item[sizeof(current->item) - 1] = '\0';

                    if (save_exams_to_file(e_head) != 0)
                        printf("保存检查信息失败！\n");
                    else
                        printf("更新成功！\n");
                }
                wait_enter();
            }
            else if (select == 2)
            {
                printf("请输入新的检查结果(输入0取消本次修改): ");
                safe_input(buf, sizeof(buf));

                if (strcmp(buf, "0") != 0)
                {
                    strncpy(current->result, buf, sizeof(current->result) - 1);
                    current->result[sizeof(current->result) - 1] = '\0';

                    if (save_exams_to_file(e_head) != 0)
                        printf("保存检查信息失败！\n");
                    else
                        printf("更新成功！\n");
                }
                wait_enter();
            }
        }

        free_exams(e_head);
        free_visits(v_head);
        free_registrations(r_head);
        free_patients(p_head);
        free_doctors(d_head);
        clear_screen();
        return;
    }

    printf("未找到指定ID的检查记录！\n");
    wait_enter();
    clear_screen();
    free_exams(e_head);
}

/* 查询检查记录 */
void query_exam_record()
{
    Exam *e_head = load_exams_from_file();
    Visit *v_head = load_visits_from_file();
    Registration *r_head = load_registrations_from_file();
    Patient *p_head = load_patients_from_file();
    Doctor *d_head = load_doctors_from_file();
    char buf[MAX_INPUT_LEN];
    int select;

    while (1)
    {
        clear_screen();
        printf("===== 查询检查记录 =====\n");
        printf("请选择查询方式：\n");
        printf("1. 按检查ID查询\n");
        printf("2. 按看诊ID查询\n");
        printf("3. 按患者ID查询\n");
        printf("4. 按医生ID查询\n");
        printf("0. 返回\n");

        printf("请输入您的选择: ");
        safe_input(buf, sizeof(buf));

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
        case 1: {
            char exam_id[MAX_ID_LEN];

            printf("请输入检查ID(输入0返回): ");
            safe_input(exam_id, sizeof(exam_id));

            if (strcmp(exam_id, "0") == 0)
            {
                clear_screen();
                continue;
            }
            if (exam_id[0] == '\0')
            {
                printf("输入错误！检查ID不能为空。\n");
                wait_enter();
                clear_screen();
                continue;
            }

            Exam *e = find_exam_by_e_id(e_head, exam_id);
            if (e)
            {
                int exam_w, visit_w, p_w, d_w, dept_w, item_w, result_w;
                calc_exam_width(e_head, v_head, r_head, p_head, d_head, &exam_w, &visit_w, &p_w, &d_w, &dept_w, &item_w,
                                &result_w);

                print_exam_header(exam_w, visit_w, p_w, d_w, dept_w, item_w, result_w);
                print_exam(e, v_head, r_head, p_head, d_head, exam_w, visit_w, p_w, d_w, dept_w, item_w, result_w);
                print_exam_line(exam_w, visit_w, p_w, d_w, dept_w, item_w, result_w);
            }
            else
            {
                printf("未找到指定ID的检查记录！\n");
            }
            wait_enter();
            clear_screen();
            break;
        }
        case 2: {
            char visit_id[MAX_ID_LEN];

            printf("请输入看诊ID(输入0返回): ");
            safe_input(visit_id, sizeof(visit_id));

            if (strcmp(visit_id, "0") == 0)
            {
                clear_screen();
                continue;
            }

            int found = 0;
            int exam_w, visit_w, p_w, d_w, dept_w, item_w, result_w;
            calc_exam_width(e_head, v_head, r_head, p_head, d_head, &exam_w, &visit_w, &p_w, &d_w, &dept_w, &item_w,
                            &result_w);

            Exam *cur = e_head;
            Exam *e = NULL;

            while ((e = find_exam_by_v_id(cur, visit_id)) != NULL)
            {
                if (!found)
                {
                    print_exam_header(exam_w, visit_w, p_w, d_w, dept_w, item_w, result_w);
                    found = 1;
                }

                print_exam(e, v_head, r_head, p_head, d_head, exam_w, visit_w, p_w, d_w, dept_w, item_w, result_w);

                cur = e->next;
            }
            if (!found)
            {
                printf("未找到指定ID的检查记录！\n");
            }
            else
            {
                print_exam_line(exam_w, visit_w, p_w, d_w, dept_w, item_w, result_w);
            }

            wait_enter();
            clear_screen();
            break;
        }
        case 3: {
            char p_id[MAX_ID_LEN];
            printf("请输入患者ID(输入0返回): ");
            safe_input(p_id, sizeof(p_id));
            if (strcmp(p_id, "0") == 0)
            {
                clear_screen();
                continue;
            }

            int found = 0;
            int exam_w, visit_w, p_w, d_w, dept_w, item_w, result_w;
            calc_exam_width(e_head, v_head, r_head, p_head, d_head, &exam_w, &visit_w, &p_w, &d_w, &dept_w, &item_w,
                            &result_w);

            Exam *cur = e_head;
            Exam *e = NULL;

            while ((e = find_exam_by_p_id(cur, v_head, r_head, p_id)) != NULL)
            {
                if (!found)
                {
                    print_exam_header(exam_w, visit_w, p_w, d_w, dept_w, item_w, result_w);
                    found = 1;
                }

                print_exam(e, v_head, r_head, p_head, d_head, exam_w, visit_w, p_w, d_w, dept_w, item_w, result_w);

                cur = e->next;
            }

            if (!found)
            {
                printf("未找到指定ID的检查记录！\n");
            }
            else
            {
                print_exam_line(exam_w, visit_w, p_w, d_w, dept_w, item_w, result_w);
            }

            wait_enter();
            clear_screen();
            break;
        }
        case 4: {
            char d_id[MAX_ID_LEN];
            printf("请输入医生ID(输入0返回): ");
            safe_input(d_id, sizeof(d_id));
            if (strcmp(d_id, "0") == 0)
            {
                clear_screen();
                continue;
            }

            int found = 0;
            int exam_w, visit_w, p_w, d_w, dept_w, item_w, result_w;
            calc_exam_width(e_head, v_head, r_head, p_head, d_head, &exam_w, &visit_w, &p_w, &d_w, &dept_w, &item_w,
                            &result_w);

            Exam *cur = e_head;
            Exam *e = NULL;

            while ((e = find_exam_by_d_id(cur, v_head, r_head, d_id)) != NULL)
            {
                if (!found)
                {
                    print_exam_header(exam_w, visit_w, p_w, d_w, dept_w, item_w, result_w);
                    found = 1;
                }

                print_exam(e, v_head, r_head, p_head, d_head, exam_w, visit_w, p_w, d_w, dept_w, item_w, result_w);

                cur = e->next;
            }

            if (!found)
            {
                printf("未找到指定ID的检查记录！\n");
            }
            else
            {
                print_exam_line(exam_w, visit_w, p_w, d_w, dept_w, item_w, result_w);
            }

            wait_enter();
            clear_screen();
            break;
        }
        case 0:
            free_exams(e_head);
            free_visits(v_head);
            free_registrations(r_head);
            free_patients(p_head);
            free_doctors(d_head);
            clear_screen();
            return;
        default:
            printf("未知错误: %d\n", select);
            break;
        }
    }
}

/* 显示所有检查记录 */
void show_all_exam_records()
{
    Exam *e_head = load_exams_from_file();
    if (!e_head)
    {
        printf("没有检查记录可显示！\n");
        wait_enter();
        clear_screen();
        return;
    }
    Visit *v_head = load_visits_from_file();
    if (!v_head)
    {
        printf("没有看诊记录可显示！\n");
        free_exams(e_head);
        wait_enter();
        clear_screen();
        return;
    }
    Registration *r_head = load_registrations_from_file();
    if (!r_head)
    {
        printf("没有挂号记录可显示！\n");
        free_exams(e_head);
        free_visits(v_head);
        wait_enter();
        clear_screen();
        return;
    }
    Patient *p_head = load_patients_from_file();
    if (!p_head)
    {
        printf("没有患者记录可显示！\n");
        free_exams(e_head);
        free_visits(v_head);
        free_registrations(r_head);
        wait_enter();
        clear_screen();
        return;
    }
    Doctor *d_head = load_doctors_from_file();
    if (!d_head)
    {
        printf("没有医生记录可显示！\n");
        free_exams(e_head);
        free_visits(v_head);
        free_registrations(r_head);
        free_patients(p_head);
        wait_enter();
        clear_screen();
        return;
    }

    int page_size = PAGE_SIZE;
    int total = count_exams(e_head);
    int total_pages = (total + page_size - 1) / page_size;

    int current_page = 1;

    int exam_w, visit_w, p_w, d_w, dept_w, item_w, result_w;
    calc_exam_width(e_head, v_head, r_head, p_head, d_head, &exam_w, &visit_w, &p_w, &d_w, &dept_w, &item_w, &result_w);

    while (1)
    {
        clear_screen();

        printf("===== 所有检查记录 (第 %d/%d 页) =====\n", current_page, total_pages);

        print_exam_header(exam_w, visit_w, p_w, d_w, dept_w, item_w, result_w);

        int start = (current_page - 1) * page_size;
        Exam *e_cur = get_nth_exam(e_head, start);

        for (int i = 0; i < page_size && e_cur; i++)
        {
            print_exam(e_cur, v_head, r_head, p_head, d_head, exam_w, visit_w, p_w, d_w, dept_w, item_w, result_w);
            e_cur = e_cur->next;
        }

        print_exam_line(exam_w, visit_w, p_w, d_w, dept_w, item_w, result_w);

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

    free_exams(e_head);
    free_visits(v_head);
    free_registrations(r_head);
    free_patients(p_head);
    free_doctors(d_head);
}