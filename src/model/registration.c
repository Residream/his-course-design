/*
 * 挂号预约模块
 */
#include "model/registration.h"
#include "core/utils.h"
#include "model/doctor.h"
#include "model/patient.h"

/*
 * 挂号基础操作
 */

/* 从文件中加载挂号数据 */
Registration *load_registrations_from_file(void)
{
    FILE *fp = fopen(REGISTRATIONS_FILE, "r");
    if (!fp)
        return NULL;

    Registration *head = NULL, *tail = NULL;
    char line[MAX_LINE_LEN];

    while (fgets(line, sizeof(line), fp))
    {
        trim_newline(line);
        if (!line[0])
            continue;

        Registration *node = (Registration *)malloc(sizeof(Registration));
        if (!node)
        {
            fclose(fp);
            free_registrations(head);
            return NULL;
        }
        memset(node, 0, sizeof(Registration));

        char *token = strtok(line, "|"); // reg_id
        if (!token)
        {
            free(node);
            continue;
        }
        strncpy(node->reg_id, token, sizeof(node->reg_id) - 1);
        node->reg_id[sizeof(node->reg_id) - 1] = '\0';

        token = strtok(NULL, "|"); // p_id
        if (!token)
        {
            free(node);
            continue;
        }
        strncpy(node->p_id, token, sizeof(node->p_id) - 1);
        node->p_id[sizeof(node->p_id) - 1] = '\0';

        token = strtok(NULL, "|"); // d_id
        if (!token)
        {
            free(node);
            continue;
        }
        strncpy(node->d_id, token, sizeof(node->d_id) - 1);
        node->d_id[sizeof(node->d_id) - 1] = '\0';

        token = strtok(NULL, "|"); // when
        if (!token)
        {
            free(node);
            continue;
        }
        node->when = (time_t)atoll(token);

        token = strtok(NULL, "|"); // status
        if (!token)
        {
            free(node);
            continue;
        }
        node->status = atoi(token);

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

/* 将挂号数据保存到文件 */
int save_registrations_to_file(Registration *head)
{
    char tmp_path[MAX_LINE_LEN];
    FILE *fp = safe_fopen_tmp(REGISTRATIONS_FILE, tmp_path, sizeof(tmp_path));
    if (!fp)
        return -1;

    Registration *current = head;
    while (current)
    {
        fprintf(fp, "%s|%s|%s|%lld|%d\n", current->reg_id, current->p_id, current->d_id, (long long)current->when,
                current->status);
        current = current->next;
    }

    return safe_fclose_commit(fp, tmp_path, REGISTRATIONS_FILE);
}

/* 释放挂号数据内存 */
void free_registrations(Registration *head)
{
    Registration *current = head;
    while (current)
    {
        Registration *temp = current;
        current = current->next;
        free(temp);
    }
}

/* 根据挂号ID查找挂号 */
Registration *find_registration_by_r_id(Registration *head, const char *reg_id)
{
    if (!reg_id)
        return NULL;
    for (Registration *cur = head; cur; cur = cur->next)
    {
        if (strcmp(cur->reg_id, reg_id) == 0)
            return cur;
    }
    return NULL;
}

/* 根据患者ID查找挂号 */
Registration *find_registration_by_p_id(Registration *head, const char *p_id)
{
    if (!p_id)
        return NULL;
    for (Registration *cur = head; cur; cur = cur->next)
    {
        if (strcmp(cur->p_id, p_id) == 0)
            return cur;
    }
    return NULL;
}

/* 根据医生ID查找挂号 */
Registration *find_registration_by_d_id(Registration *head, const char *d_id)
{
    if (!d_id)
        return NULL;
    for (Registration *cur = head; cur; cur = cur->next)
    {
        if (strcmp(cur->d_id, d_id) == 0)
            return cur;
    }
    return NULL;
}

/* 生成下一个挂号ID(Rxxxx) */
int generate_next_registration_id(Registration *head)
{
    int max_id = 0;
    Registration *current = head;
    while (current)
    {
        if (strncmp(current->reg_id, "R", 1) == 0)
        {
            int num = atoi(current->reg_id + 1);
            if (num > max_id)
                max_id = num;
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

/* 打印挂号信息行 */
void print_registration(Registration *r, Patient *p_head, Doctor *d_head, int reg_w, int p_w, int d_w, int dept_w,
                        int when_w, int st_w)
{
    char when_buf[32];
    char st_buf[16];

    const char *p_name = get_patient_name_by_p_id(p_head, r->p_id);
    Doctor *doc = find_doctor_by_d_id(d_head, r->d_id);
    const char *d_name = doc ? doc->name : r->d_id;
    const char *dept = doc ? doc->department : "未知科室";

    if (format_beijing_time(r->when, when_buf, sizeof(when_buf)) == 0)
        strncpy(when_buf, "时间无效", sizeof(when_buf) - 1);
    when_buf[sizeof(when_buf) - 1] = '\0';

    if (r->status >= 0 && r->status < REG_STATUS_COUNT)
        strncpy(st_buf, REG_STATUS_TEXT[r->status], sizeof(st_buf) - 1);
    else
        strncpy(st_buf, "未知", sizeof(st_buf) - 1);
    st_buf[sizeof(st_buf) - 1] = '\0';

    printf("| ");
    print_align(r->reg_id, reg_w);
    printf(" | ");
    print_align(p_name, p_w);
    printf(" | ");
    print_align(d_name, d_w);
    printf(" | ");
    print_align(dept, dept_w);
    printf(" | ");
    print_align(when_buf, when_w);
    printf(" | ");
    print_align(st_buf, st_w);
    printf(" |\n");
}

/* 打印挂号表头 */
void print_registration_header(int reg_w, int p_w, int d_w, int dept_w, int when_w, int st_w)
{
    print_registration_line(reg_w, p_w, d_w, dept_w, when_w, st_w);
    printf("| ");
    print_align("挂号ID", reg_w);
    printf(" | ");
    print_align("患者姓名", p_w);
    printf(" | ");
    print_align("医生姓名", d_w);
    printf(" | ");
    print_align("科室", dept_w);
    printf(" | ");
    print_align("挂号时间", when_w);
    printf(" | ");
    print_align("状态", st_w);
    printf(" |\n");
    print_registration_line(reg_w, p_w, d_w, dept_w, when_w, st_w);
}

/* 打印挂号分隔线 */
void print_registration_line(int reg_w, int p_w, int d_w, int dept_w, int when_w, int st_w)
{
    printf("+");
    for (int i = 0; i < reg_w + 2; i++)
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
    for (int i = 0; i < when_w + 2; i++)
        printf("-");
    printf("+");
    for (int i = 0; i < st_w + 2; i++)
        printf("-");
    printf("+\n");
}

/* 计算挂号表格列宽 */
void calc_registration_width(Registration *r_head, Patient *p_head, Doctor *d_head, int *reg_w, int *p_w, int *d_w,
                             int *dept_w, int *when_w, int *st_w)
{
    *reg_w = str_width("挂号ID");
    *p_w = str_width("患者姓名");
    *d_w = str_width("医生姓名");
    *dept_w = str_width("科室");
    *when_w = str_width("挂号时间");
    *st_w = str_width("状态");

    for (Registration *r = r_head; r; r = r->next)
    {
        int w;
        char when_buf[32];
        const char *st = "未知";

        const char *p_name = get_patient_name_by_p_id(p_head, r->p_id);
        Doctor *doc = find_doctor_by_d_id(d_head, r->d_id);
        const char *d_name = doc ? doc->name : r->d_id;
        const char *dept = doc ? doc->department : "未知科室";

        w = str_width(r->reg_id);
        if (w > *reg_w)
            *reg_w = w;
        w = str_width(p_name);
        if (w > *p_w)
            *p_w = w;
        w = str_width(d_name);
        if (w > *d_w)
            *d_w = w;
        w = str_width(dept);
        if (w > *dept_w)
            *dept_w = w;

        if (format_beijing_time(r->when, when_buf, sizeof(when_buf)) == 0)
            strncpy(when_buf, "时间无效", sizeof(when_buf) - 1);
        when_buf[sizeof(when_buf) - 1] = '\0';
        w = str_width(when_buf);
        if (w > *when_w)
            *when_w = w;

        if (r->status >= 0 && r->status < REG_STATUS_COUNT)
            st = REG_STATUS_TEXT[r->status];
        w = str_width(st);
        if (w > *st_w)
            *st_w = w;
    }
}

/* 统计医生的挂号数量 */
int count_registrations_for_doctor(Registration *head, const char *d_id)
{
    int count = 0;
    for (Registration *cur = head; cur; cur = cur->next)
    {
        if (strcmp(cur->d_id, d_id) == 0)
            count++;
    }
    return count;
}

/* 获取医生的第n个挂号节点 */
Registration *get_nth_registration_for_doctor(Registration *head, const char *d_id, int n)
{
    int count = 0;
    for (Registration *cur = head; cur; cur = cur->next)
    {
        if (strcmp(cur->d_id, d_id) == 0)
        {
            if (count == n)
                return cur;
            count++;
        }
    }
    return NULL;
}

/*
 * 挂号功能函数
 */

/* 创建挂号 */
Registration create_registration(const char *reg_id, const char *p_id, const char *d_id, time_t when, int status)
{
    Registration reg;
    memset(&reg, 0, sizeof(Registration));

    strncpy(reg.reg_id, reg_id, sizeof(reg.reg_id) - 1);
    reg.reg_id[sizeof(reg.reg_id) - 1] = '\0';

    strncpy(reg.p_id, p_id, sizeof(reg.p_id) - 1);
    reg.p_id[sizeof(reg.p_id) - 1] = '\0';

    strncpy(reg.d_id, d_id, sizeof(reg.d_id) - 1);
    reg.d_id[sizeof(reg.d_id) - 1] = '\0';

    reg.when = when;
    reg.status = status;
    reg.next = NULL;
    return reg;
}

/* 尾插挂号 */
void append_registration(Registration **head, Registration *new_registration)
{
    if (!head || !new_registration)
        return;

    new_registration->next = NULL;

    if (!*head)
    {
        *head = new_registration;
        return;
    }

    Registration *current = *head;
    while (current->next)
        current = current->next;
    current->next = new_registration;
}
