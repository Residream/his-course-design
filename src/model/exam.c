/*
 * 检查模块
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

/* 统计医生名下的检查数量 */
int count_exams_for_doctor(Exam *e_head, Visit *v_head, Registration *reg_head, const char *d_id)
{
    int count = 0;
    for (Exam *e = e_head; e; e = e->next)
    {
        Visit *v = find_visit_by_v_id(v_head, e->visit_id);
        if (!v)
            continue;

        const char *e_d_id = get_doctor_id_by_reg_id(reg_head, v->reg_id);
        if (e_d_id && strcmp(e_d_id, d_id) == 0)
        {
            count++;
        }
    }
    return count;
}

/* 获取医生名下的第n个检查节点 */
Exam *get_nth_exam_for_doctor(Exam *e_head, Visit *v_head, Registration *reg_head, const char *d_id, int n)
{
    int count = 0;
    for (Exam *e = e_head; e; e = e->next)
    {
        Visit *v = find_visit_by_v_id(v_head, e->visit_id);
        if (!v)
            continue;

        const char *e_d_id = get_doctor_id_by_reg_id(reg_head, v->reg_id);
        if (e_d_id && strcmp(e_d_id, d_id) == 0)
        {
            if (count == n)
                return e;
            count++;
        }
    }
    return NULL;
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

/* 删除检查 */
void exam_remove(Exam **head, Exam *target)
{
    if (!head || !*head || !target)
        return;

    if (*head == target)
    {
        *head = target->next;
        free(target);
        return;
    }

    Exam *prev = *head;
    while (prev->next && prev->next != target)
        prev = prev->next;

    if (prev->next == target)
    {
        prev->next = target->next;
        free(target);
    }
    /* 若未找到 target 说明它不在链表中, 不处理 */
}
