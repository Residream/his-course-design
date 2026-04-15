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

/* 从文件中加载处方数据，文件格式: pr_id|visit_id|p_id|d_id|drug_id|dose|frequency */
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
        fprintf(fp, "%s|%s|%s|%s|%s|%s|%s\n", cur->pr_id, cur->visit_id, cur->d_id, cur->p_id, cur->drug_id, cur->dose,
                cur->frequency);

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

static const char *get_patient_name_by_p_id(Patient *p_head, const char *p_id)
{
    Patient *p = find_patient_by_p_id(p_head, p_id);
    return p ? p->name : p_id;
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
Prescription *get_prescription_by_index(Prescription *head, int index)
{
    Prescription *pr = head;
    for (int i = 0; i < index && pr; i++)
        pr = pr->next;
    return pr;
}

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

/*
 * 处方系统功能(增删改查)
 */

/* 添加处方记录(需指定看诊ID, 自动关联患者和医生) */
void add_prescription_record()
{
    Prescription *pr_head = load_prescriptions_from_file();
    Registration *r_head = load_registrations_from_file();
    Visit *v_head = load_visits_from_file();
    Patient *p_head = load_patients_from_file();
    Doctor *d_head = load_doctors_from_file();
    Drug *drug_head = load_drugs_from_file();

    char pr_id[MAX_ID_LEN];
    char visit_id[MAX_ID_LEN];
    char p_id[MAX_ID_LEN];
    char d_id[MAX_ID_LEN];
    char drug_id[MAX_ID_LEN];
    char dose[MAX_DOSE_LEN];
    char frequency[MAX_FREQ_LEN];

    /* 自动分配新处方ID */
    int id_num = generate_next_prescription_id(pr_head);
    snprintf(pr_id, sizeof(pr_id), "PR%04d", id_num);

    Visit *visit = NULL;
    Registration *reg = NULL;

    /* 获取并验证看诊ID */
    while (1)
    {
        printf("请输入看诊ID(输入0返回): ");
        safe_input(visit_id, sizeof(visit_id));
        if (strcmp(visit_id, "0") == 0)
            goto cleanup;

        if (visit_id[0] == '\0')
        {
            printf("输入错误！看诊ID不能为空。\n");
            continue;
        }

        visit = find_visit_by_v_id(v_head, visit_id);
        if (!visit)
        {
            printf("未找到该看诊ID！\n");
            continue;
        }

        reg = find_registration_by_r_id(r_head, visit->reg_id);
        if (!reg)
        {
            printf("该看诊记录关联的挂号记录丢失，数据异常！\n");
            goto cleanup;
        }
        break;
    }

    /* 获取并验证患者ID */
    while (1)
    {
        printf("请输入患者ID(输入0返回): ");
        safe_input(p_id, sizeof(p_id));
        if (strcmp(p_id, "0") == 0)
            goto cleanup;

        if (p_id[0] == '\0')
            continue;

        if (!find_patient_by_p_id(p_head, p_id))
        {
            printf("未找到该患者ID！\n");
            continue;
        }
        if (strcmp(reg->p_id, p_id) != 0)
        {
            printf("错误：患者ID与原看诊记录的患者不匹配 (该看诊属患者: %s)！\n", reg->p_id);
            continue;
        }
        break;
    }

    /* 获取并验证医生ID */
    while (1)
    {
        printf("请输入医生ID(输入0返回): ");
        safe_input(d_id, sizeof(d_id));
        if (strcmp(d_id, "0") == 0)
            goto cleanup;

        if (d_id[0] == '\0')
            continue;

        if (!find_doctor_by_d_id(d_head, d_id))
        {
            printf("未找到该医生ID！\n");
            continue;
        }
        if (strcmp(reg->d_id, d_id) != 0)
        {
            printf("错误：医生ID与原看诊记录的医生不匹配 (原看诊医生: %s)！\n", reg->d_id);
            continue;
        }
        break;
    }

    /* 获取并验证药品ID */
    while (1)
    {
        printf("请输入药品ID(输入0返回): ");
        safe_input(drug_id, sizeof(drug_id));
        if (strcmp(drug_id, "0") == 0)
            goto cleanup;

        if (drug_id[0] == '\0')
            continue;

        if (!find_drug_by_id(drug_head, drug_id))
        {
            printf("未找到该药品ID！\n");
            continue;
        }
        break;
    }

    /* 剂量 */
    while (1)
    {
        printf("请输入剂量(如 1粒, 10ml, 输入0返回): ");
        safe_input(dose, sizeof(dose));
        if (strcmp(dose, "0") == 0)
            goto cleanup;
        if (dose[0] != '\0')
            break;
    }

    /* 频次 */
    while (1)
    {
        printf("请输入频次(如 1日3次, 输入0返回): ");
        safe_input(frequency, sizeof(frequency));
        if (strcmp(frequency, "0") == 0)
            goto cleanup;
        if (frequency[0] != '\0')
            break;
    }

    /* 构建并插入新处方 */
    Prescription *new_node = (Prescription *)malloc(sizeof(Prescription));
    if (!new_node)
    {
        printf("内存分配失败！\n");
        goto cleanup;
    }

    *new_node = create_prescription(pr_id, visit_id, p_id, d_id, drug_id, dose, frequency);
    append_prescription(&pr_head, new_node);

    if (save_prescriptions_to_file(pr_head) != 0)
        printf("保存处方信息失败！\n");
    else
        printf("处方记录添加成功！分配的处方ID: %s\n", pr_id);

    wait_enter();

cleanup:
    free_prescriptions(pr_head);
    free_registrations(r_head);
    free_visits(v_head);
    free_patients(p_head);
    free_doctors(d_head);
    free_drugs(drug_head);
    clear_screen();
}

/* 删除处方记录 */
void delete_prescription_record()
{
    char pr_id[MAX_ID_LEN];
    Prescription *pr_head = load_prescriptions_from_file();
    Patient *p_head = load_patients_from_file();
    Doctor *d_head = load_doctors_from_file();
    Drug *drug_head = load_drugs_from_file();

    while (1)
    {
        printf("请输入要删除的处方ID(输入0返回): ");
        safe_input(pr_id, sizeof(pr_id));
        if (strcmp(pr_id, "0") == 0)
            break;

        if (pr_id[0] == '\0')
        {
            printf("输入错误！处方ID不能为空。\n");
            continue;
        }

        Prescription *current = pr_head, *prev = NULL;
        int found = 0;

        while (current)
        {
            if (strcmp(current->pr_id, pr_id) == 0)
            {
                found = 1;
                int pr_w, visit_w, d_w, p_w, drug_w, dose_w, freq_w;
                calc_prescription_width(current, p_head, d_head, drug_head, &pr_w, &visit_w, &d_w, &p_w, &drug_w,
                                        &dose_w, &freq_w);

                clear_screen();
                printf("找到处方:\n");
                print_prescription_header(pr_w, visit_w, d_w, p_w, drug_w, dose_w, freq_w);
                print_prescription(current, p_head, d_head, drug_head, pr_w, visit_w, d_w, p_w, drug_w, dose_w, freq_w);
                print_prescription_line(pr_w, visit_w, d_w, p_w, drug_w, dose_w, freq_w);

                printf("确认要删除处方记录 %s 吗？(y/n): ", current->pr_id);
                char choice[16];
                safe_input(choice, sizeof(choice));
                if (strcmp(choice, "y") != 0 && strcmp(choice, "Y") != 0)
                {
                    printf("已取消删除。\n");
                    break;
                }

                if (prev)
                    prev->next = current->next;
                else
                    pr_head = current->next;

                free(current);

                if (save_prescriptions_to_file(pr_head) != 0)
                    printf("保存处方信息失败！\n");
                else
                    printf("删除成功！\n");
                break;
            }
            prev = current;
            current = current->next;
        }

        if (!found)
            printf("未找到指定ID的处方记录！\n");

        wait_enter();
        clear_screen();
        break;
    }

    free_prescriptions(pr_head);
    free_patients(p_head);
    free_doctors(d_head);
    free_drugs(drug_head);
}

/* 更新处方记录 */
void update_prescription_record()
{
    char pr_id[MAX_ID_LEN];
    char buf[MAX_INPUT_LEN];

    Prescription *pr_head = load_prescriptions_from_file();
    if (!pr_head)
    {
        printf("暂无处方记录可更新！\n");
        wait_enter();
        clear_screen();
        return;
    }

    printf("请输入要修改的处方ID(输入0返回): ");
    safe_input(pr_id, sizeof(pr_id));

    if (strcmp(pr_id, "0") == 0)
    {
        clear_screen();
        free_prescriptions(pr_head);
        return;
    }

    if (pr_id[0] == '\0')
    {
        printf("输入错误！处方ID不能为空。\n");
        wait_enter();
        clear_screen();
        free_prescriptions(pr_head);
        return;
    }

    Prescription *current = NULL;
    for (current = pr_head; current; current = current->next)
    {
        if (strcmp(current->pr_id, pr_id) != 0)
            continue;

        Patient *p_head = load_patients_from_file();
        Doctor *d_head = load_doctors_from_file();
        Drug *drug_head = load_drugs_from_file();

        int done = 0;
        while (!done)
        {
            clear_screen();
            int pr_w, visit_w, d_w, p_w, drug_w, dose_w, freq_w;
            calc_prescription_width(pr_head, p_head, d_head, drug_head, &pr_w, &visit_w, &d_w, &p_w, &drug_w, &dose_w,
                                    &freq_w);

            printf("找到处方记录，信息如下：\n");
            print_prescription_header(pr_w, visit_w, d_w, p_w, drug_w, dose_w, freq_w);
            print_prescription(current, p_head, d_head, drug_head, pr_w, visit_w, d_w, p_w, drug_w, dose_w, freq_w);
            print_prescription_line(pr_w, visit_w, d_w, p_w, drug_w, dose_w, freq_w);

            printf("\n请选择要修改的字段:\n");
            printf("1. 更新剂量 (当前: %s)\n", current->dose);
            printf("2. 更新频次 (当前: %s)\n", current->frequency);
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
            {
                done = 1;
                continue;
            }

            if (select == 1)
            {
                printf("请输入新的剂量(输入0取消): ");
                safe_input(buf, sizeof(buf));
                if (strcmp(buf, "0") != 0 && buf[0] != '\0')
                {
                    strncpy(current->dose, buf, sizeof(current->dose) - 1);
                    current->dose[sizeof(current->dose) - 1] = '\0';

                    if (save_prescriptions_to_file(pr_head) != 0)
                        printf("保存处方信息失败！\n");
                    else
                        printf("更新成功！\n");
                }
                wait_enter();
            }
            else if (select == 2)
            {
                printf("请输入新的频次(输入0取消): ");
                safe_input(buf, sizeof(buf));
                if (strcmp(buf, "0") != 0 && buf[0] != '\0')
                {
                    strncpy(current->frequency, buf, sizeof(current->frequency) - 1);
                    current->frequency[sizeof(current->frequency) - 1] = '\0';

                    if (save_prescriptions_to_file(pr_head) != 0)
                        printf("保存处方信息失败！\n");
                    else
                        printf("更新成功！\n");
                }
                wait_enter();
            }
        }

        free_prescriptions(pr_head);
        free_patients(p_head);
        free_doctors(d_head);
        free_drugs(drug_head);
        clear_screen();
        return;
    }

    printf("未找到指定ID的处方记录！\n");
    wait_enter();
    clear_screen();
    free_prescriptions(pr_head);
}

/* 查询处方记录 */
void query_prescription_record()
{
    Prescription *pr_head = load_prescriptions_from_file();
    Patient *p_head = load_patients_from_file();
    Doctor *d_head = load_doctors_from_file();
    Drug *drug_head = load_drugs_from_file();
    char buf[MAX_INPUT_LEN];

    while (1)
    {
        clear_screen();
        printf("===== 查询处方记录 =====\n");
        printf("1. 按处方ID查询\n");
        printf("2. 按看诊ID查询\n");
        printf("3. 按患者ID查询\n");
        printf("4. 按医生ID查询\n");
        printf("5. 按药品名称模糊查询\n");
        printf("0. 返回\n");
        printf("请输入您的选择: ");
        safe_input(buf, sizeof(buf));

        if (!validate_choice(buf, 5))
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

        int found = 0;
        int pr_w, visit_w, d_w, p_w, drug_w, dose_w, freq_w;
        calc_prescription_width(pr_head, p_head, d_head, drug_head, &pr_w, &visit_w, &d_w, &p_w, &drug_w, &dose_w,
                                &freq_w);

        for (Prescription *cur = pr_head; cur; cur = cur->next)
        {
            int match = 0;
            if (select == 1 && strcmp(cur->pr_id, query) == 0)
                match = 1;
            else if (select == 2 && strcmp(cur->visit_id, query) == 0)
                match = 1;
            else if (select == 3 && strcmp(cur->p_id, query) == 0)
                match = 1;
            else if (select == 4 && strcmp(cur->d_id, query) == 0)
                match = 1;
            else if (select == 5) // 按药品名称模糊匹配（通用名/商品名/别名）
            {
                Drug *drug = find_drug_by_id(drug_head, cur->drug_id);
                if (drug && (strstr(drug->generic_name, query) != NULL || strstr(drug->trade_name, query) != NULL ||
                             strstr(drug->alias, query) != NULL))
                    match = 1;
            }

            if (match)
            {
                if (!found)
                {
                    print_prescription_header(pr_w, visit_w, d_w, p_w, drug_w, dose_w, freq_w);
                    found = 1;
                }
                print_prescription(cur, p_head, d_head, drug_head, pr_w, visit_w, d_w, p_w, drug_w, dose_w, freq_w);
            }
        }

        if (!found)
            printf("未找到匹配的处方记录！\n");
        else
            print_prescription_line(pr_w, visit_w, d_w, p_w, drug_w, dose_w, freq_w);

        wait_enter();
    }

    free_prescriptions(pr_head);
    free_patients(p_head);
    free_doctors(d_head);
    free_drugs(drug_head);
    clear_screen();
}

/* 显示所有处方记录 */
void show_all_prescription_records()
{
    Prescription *pr_head = load_prescriptions_from_file();
    if (!pr_head)
    {
        printf("暂无处方记录！\n");
        wait_enter();
        clear_screen();
        return;
    }

    Patient *p_head = load_patients_from_file();
    Doctor *d_head = load_doctors_from_file();
    Drug *drug_head = load_drugs_from_file();

    int page_size = PAGE_SIZE;
    int total = count_prescriptions(pr_head);
    int total_pages = (total + page_size - 1) / page_size;
    int current_page = 1;

    int pr_w, visit_w, d_w, p_w, drug_w, dose_w, freq_w;
    calc_prescription_width(pr_head, p_head, d_head, drug_head, &pr_w, &visit_w, &d_w, &p_w, &drug_w, &dose_w, &freq_w);

    while (1)
    {
        clear_screen();
        printf("===== 处方列表 (第 %d/%d 页) =====\n", current_page, total_pages);
        print_prescription_header(pr_w, visit_w, d_w, p_w, drug_w, dose_w, freq_w);

        int start = (current_page - 1) * page_size;
        Prescription *pr_cur = get_prescription_by_index(pr_head, start);

        for (int i = 0; i < page_size && pr_cur; i++)
        {
            print_prescription(pr_cur, p_head, d_head, drug_head, pr_w, visit_w, d_w, p_w, drug_w, dose_w, freq_w);
            pr_cur = pr_cur->next;
        }
        print_prescription_line(pr_w, visit_w, d_w, p_w, drug_w, dose_w, freq_w);

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
            break;
        else
        {
            printf("输入无效！\n");
            wait_enter();
        }
    }

    free_prescriptions(pr_head);
    free_patients(p_head);
    free_doctors(d_head);
    free_drugs(drug_head);
    clear_screen();
}