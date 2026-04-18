/*
 * 处方功能模块
 */

#include "model/prescription.h"
#include "core/utils.h"
#include "model/doctor.h"
#include "model/drug.h"
#include "model/patient.h"
#include "model/registration.h"
#include "model/visit.h"

/*
 * 处方基础操作
 */

/* 从文件中加载处方数据，文件格式: pr_id|visit_id|d_id|p_id|drug_id|dose|frequency|dispensed
 * 向后兼容: 若缺少 dispensed 字段，默认视为 0（未发药） */
Prescription *load_prescriptions_from_file(void)
{
    FILE *fp = fopen(PRESCRIPTIONS_FILE, "r");
    if (!fp)
        return NULL;

    Prescription *head = NULL, *tail = NULL;
    char line[MAX_LINE_LEN];

    while (fgets(line, sizeof(line), fp))
    {
        trim_newline(line);
        if (!line[0])
            continue;

        Prescription *node = (Prescription *)malloc(sizeof(Prescription));
        if (!node)
        {
            fclose(fp);
            free_prescriptions(head);
            return NULL;
        }
        memset(node, 0, sizeof(Prescription));

        /* 获取pr_id */
        char *token = strtok(line, "|");
        if (!token)
        {
            free(node);
            continue;
        }
        strncpy(node->pr_id, token, sizeof(node->pr_id) - 1);

        /* 获取visit_id */
        token = strtok(NULL, "|");
        if (!token)
        {
            free(node);
            continue;
        }
        strncpy(node->visit_id, token, sizeof(node->visit_id) - 1);

        /* 获取d_id */
        token = strtok(NULL, "|");
        if (!token)
        {
            free(node);
            continue;
        }
        strncpy(node->d_id, token, sizeof(node->d_id) - 1);

        /* 获取p_id */
        token = strtok(NULL, "|");
        if (!token)
        {
            free(node);
            continue;
        }
        strncpy(node->p_id, token, sizeof(node->p_id) - 1);

        /* 获取drug_id */
        token = strtok(NULL, "|");
        if (!token)
        {
            free(node);
            continue;
        }
        strncpy(node->drug_id, token, sizeof(node->drug_id) - 1);

        /* 获取dose */
        token = strtok(NULL, "|");
        if (!token)
        {
            free(node);
            continue;
        }
        strncpy(node->dose, token, sizeof(node->dose) - 1);

        /* 获取frequency */
        token = strtok(NULL, "|");
        if (!token)
        {
            free(node);
            continue;
        }
        strncpy(node->frequency, token, sizeof(node->frequency) - 1);

        /* 获取 dispensed（可选字段，不存在时默认为 0） */
        token = strtok(NULL, "|");
        if (token)
            node->dispensed = atoi(token);
        else
            node->dispensed = PR_STATUS_UNDISPENSED;

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

/* 将处方数据保存到文件 */
int save_prescriptions_to_file(Prescription *head)
{
    char tmp_path[MAX_LINE_LEN];
    FILE *fp = safe_fopen_tmp(PRESCRIPTIONS_FILE, tmp_path, sizeof(tmp_path));
    if (!fp)
        return -1;

    for (Prescription *cur = head; cur; cur = cur->next)
        fprintf(fp, "%s|%s|%s|%s|%s|%s|%s|%d\n", cur->pr_id, cur->visit_id, cur->d_id, cur->p_id, cur->drug_id,
                cur->dose, cur->frequency, cur->dispensed);

    return safe_fclose_commit(fp, tmp_path, PRESCRIPTIONS_FILE);
}

/* 释放处方数据内存 */
void free_prescriptions(Prescription *head)
{
    Prescription *current = head;
    while (current)
    {
        Prescription *temp = current;
        current = current->next;
        free(temp);
    }
}

/* 根据处方ID查找处方 */
Prescription *find_prescription_by_pr_id(Prescription *head, const char *pr_id)
{
    for (Prescription *cur = head; cur; cur = cur->next)
        if (strcmp(cur->pr_id, pr_id) == 0)
            return cur;
    return NULL;
}

/* 根据看诊ID查找处方 */
Prescription *find_prescription_by_visit_id(Prescription *head, const char *visit_id)
{
    for (Prescription *cur = head; cur; cur = cur->next)
        if (strcmp(cur->visit_id, visit_id) == 0)
            return cur;
    return NULL;
}

/* 根据患者ID查找处方 */
Prescription *find_prescription_by_p_id(Prescription *head, const char *p_id)
{
    for (Prescription *cur = head; cur; cur = cur->next)
        if (strcmp(cur->p_id, p_id) == 0)
            return cur;
    return NULL;
}

/* 根据医生ID查找处方 */
Prescription *find_prescription_by_d_id(Prescription *head, const char *d_id)
{
    for (Prescription *cur = head; cur; cur = cur->next)
        if (strcmp(cur->d_id, d_id) == 0)
            return cur;
    return NULL;
}

/* 生成自增处方ID */
int generate_next_prescription_id(Prescription *head)
{
    int max_id = 0;
    for (Prescription *cur = head; cur; cur = cur->next)
    {
        const char *id = cur->pr_id;
        if (strncmp(id, "PR", 2) == 0)
        {
            int valid = 1;
            for (int i = 2; id[i] != '\0'; i++)
            {
                if (id[i] < '0' || id[i] > '9')
                {
                    valid = 0;
                    break;
                }
            }
            if (valid && id[2] != '\0')
            {
                int id_num = atoi(id + 2);
                if (id_num > max_id)
                    max_id = id_num;
            }
        }
    }
    return max_id + 1;
}

/* 打印处方信息行 */
void print_prescription(Prescription *pr, Patient *p_head, Doctor *d_head, Drug *drug_head, int pr_w, int visit_w,
                        int d_w, int p_w, int drug_w, int dose_w, int freq_w)
{
    const char *p_name = get_patient_name_by_p_id(p_head, pr->p_id);
    Doctor *doc = find_doctor_by_d_id(d_head, pr->d_id);
    const char *d_name = doc ? doc->name : pr->d_id;
    Drug *drug = find_drug_by_id(drug_head, pr->drug_id);
    const char *drug_name = drug ? drug->trade_name : pr->drug_id;

    printf("| ");
    print_align(pr->pr_id, pr_w);
    printf(" | ");
    print_align(pr->visit_id, visit_w);
    printf(" | ");
    print_align(d_name, d_w);
    printf(" | ");
    print_align(p_name, p_w);
    printf(" | ");
    print_align(drug_name, drug_w);
    printf(" | ");
    print_align(pr->dose, dose_w);
    printf(" | ");
    print_align(pr->frequency, freq_w);
    printf(" |\n");
}

/* 打印处方分隔线 */
void print_prescription_line(int pr_w, int visit_w, int d_w, int p_w, int drug_w, int dose_w, int freq_w)
{
    printf("+");
    for (int i = 0; i < pr_w + 2; i++)
        printf("-");
    printf("+");
    for (int i = 0; i < visit_w + 2; i++)
        printf("-");
    printf("+");
    for (int i = 0; i < d_w + 2; i++)
        printf("-");
    printf("+");
    for (int i = 0; i < p_w + 2; i++)
        printf("-");
    printf("+");
    for (int i = 0; i < drug_w + 2; i++)
        printf("-");
    printf("+");
    for (int i = 0; i < dose_w + 2; i++)
        printf("-");
    printf("+");
    for (int i = 0; i < freq_w + 2; i++)
        printf("-");
    printf("+\n");
}

/* 打印处方表头 */
void print_prescription_header(int pr_w, int visit_w, int d_w, int p_w, int drug_w, int dose_w, int freq_w)
{
    print_prescription_line(pr_w, visit_w, d_w, p_w, drug_w, dose_w, freq_w);

    printf("| ");
    print_align("处方ID", pr_w);
    printf(" | ");
    print_align("看诊ID", visit_w);
    printf(" | ");
    print_align("医生姓名", d_w);
    printf(" | ");
    print_align("患者姓名", p_w);
    printf(" | ");
    print_align("药品名称", drug_w);
    printf(" | ");
    print_align("剂量", dose_w);
    printf(" | ");
    print_align("频次", freq_w);
    printf(" |\n");

    print_prescription_line(pr_w, visit_w, d_w, p_w, drug_w, dose_w, freq_w);
}

/* 计算处方表格列宽 */
void calc_prescription_width(Prescription *head, Patient *p_head, Doctor *d_head, Drug *drug_head, int *pr_w,
                             int *visit_w, int *d_w, int *p_w, int *drug_w, int *dose_w, int *freq_w)
{
    *pr_w = str_width("处方ID");
    *visit_w = str_width("看诊ID");
    *d_w = str_width("医生姓名");
    *p_w = str_width("患者姓名");
    *drug_w = str_width("药品名称");
    *dose_w = str_width("剂量");
    *freq_w = str_width("频次");

    for (Prescription *cur = head; cur; cur = cur->next)
    {
        int len;

        len = str_width(cur->pr_id);
        if (len > *pr_w)
            *pr_w = len;

        len = str_width(cur->visit_id);
        if (len > *visit_w)
            *visit_w = len;

        Doctor *doc = find_doctor_by_d_id(d_head, cur->d_id);
        const char *d_name = doc ? doc->name : cur->d_id;
        len = str_width(d_name);
        if (len > *d_w)
            *d_w = len;

        Patient *p = find_patient_by_p_id(p_head, cur->p_id);
        const char *p_name = p ? p->name : cur->p_id;
        len = str_width(p_name);
        if (len > *p_w)
            *p_w = len;

        Drug *drug = find_drug_by_id(drug_head, cur->drug_id);
        const char *drug_name = drug ? drug->trade_name : cur->drug_id;
        len = str_width(drug_name);
        if (len > *drug_w)
            *drug_w = len;

        len = str_width(cur->dose);
        if (len > *dose_w)
            *dose_w = len;

        len = str_width(cur->frequency);
        if (len > *freq_w)
            *freq_w = len;
    }
}

/* 统计处方数量 */
int count_prescriptions(Prescription *head)
{
    int count = 0;
    for (Prescription *cur = head; cur; cur = cur->next)
        count++;
    return count;
}

/* 获取第n个处方节点 */
Prescription *get_nth_prescription(Prescription *head, int n)
{
    Prescription *pr = head;
    for (int i = 0; i < n && pr; i++)
        pr = pr->next;
    return pr;
}

/* 统计医生名下的处方数量 */
int count_prescriptions_for_doctor(Prescription *pr_head, const char *d_id)
{
    int count = 0;
    for (Prescription *cur = pr_head; cur; cur = cur->next)
    {
        if (strcmp(cur->d_id, d_id) == 0)
        {
            count++;
        }
    }
    return count;
}

/* 获取医生名下的第n个处方节点 */
Prescription *get_nth_prescription_for_doctor(Prescription *pr_head, const char *d_id, int n)
{
    int count = 0;
    for (Prescription *cur = pr_head; cur; cur = cur->next)
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
 * 处方功能函数
 */

/* 创建处方 */
Prescription create_prescription(const char *pr_id, const char *visit_id, const char *p_id, const char *d_id,
                                 const char *drug_id, const char *dose, const char *frequency)
{
    Prescription pr;
    memset(&pr, 0, sizeof(Prescription));

    strncpy(pr.pr_id, pr_id, sizeof(pr.pr_id) - 1);
    strncpy(pr.visit_id, visit_id, sizeof(pr.visit_id) - 1);
    strncpy(pr.p_id, p_id, sizeof(pr.p_id) - 1);
    strncpy(pr.d_id, d_id, sizeof(pr.d_id) - 1);
    strncpy(pr.drug_id, drug_id, sizeof(pr.drug_id) - 1);
    strncpy(pr.dose, dose, sizeof(pr.dose) - 1);
    strncpy(pr.frequency, frequency, sizeof(pr.frequency) - 1);

    pr.next = NULL;
    return pr;
}

/* 尾插处方 */
void append_prescription(Prescription **head, Prescription *new_pr)
{
    if (!head || !new_pr)
        return;
    new_pr->next = NULL;
    if (!*head)
    {
        *head = new_pr;
        return;
    }

    Prescription *cur = *head;
    while (cur->next)
        cur = cur->next;
    cur->next = new_pr;
}

/* 删除处方 */
void prescription_remove(Prescription **head, Prescription *target)
{
    if (!head || !*head || !target)
        return;

    if (*head == target)
    {
        *head = target->next;
        free(target);
        return;
    }

    Prescription *prev = *head;
    while (prev->next && prev->next != target)
        prev = prev->next;

    if (prev->next == target)
    {
        prev->next = target->next;
        free(target);
    }
    /* 若未找到 target 说明它不在链表中, 不处理 */
}
