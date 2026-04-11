/*
 * 看诊模块
 */
#include "model/visit.h"
#include "core/utils.h"
#include "model/doctor.h"
#include "model/drug.h"
#include "model/exam.h"
#include "model/hospitalization.h"
#include "model/patient.h"
#include "model/prescription.h"
#include "model/registration.h"

/*
 * 看诊基础功能
 */

/* 从文件中加载看诊数据 */
Visit *load_visits_from_file(void)
{
    FILE *fp = fopen(VISITS_FILE, "r");
    if (!fp)
        return NULL;

    Visit *head = NULL, *tail = NULL;
    char line[MAX_LINE_LEN];

    while (fgets(line, sizeof(line), fp))
    {
        trim_newline(line);
        if (!line[0])
            continue;

        Visit *node = (Visit *)malloc(sizeof(Visit));
        if (!node)
        {
            fclose(fp);
            free_visits(head);
            return NULL;
        }
        memset(node, 0, sizeof(Visit));

        char *token = strtok(line, "|"); // visit_id
        if (!token)
        {
            free(node);
            continue;
        }
        strncpy(node->visit_id, token, sizeof(node->visit_id) - 1);
        node->visit_id[sizeof(node->visit_id) - 1] = '\0';

        token = strtok(NULL, "|"); // reg_id
        if (!token)
        {
            free(node);
            continue;
        }
        strncpy(node->reg_id, token, sizeof(node->reg_id) - 1);
        node->reg_id[sizeof(node->reg_id) - 1] = '\0';

        token = strtok(NULL, "|"); // when
        if (!token)
        {
            free(node);
            continue;
        }
        char *endptr = NULL;
        long long when_val = strtoll(token, &endptr, 10);
        if (token[0] == '\0' || *endptr != '\0' || when_val < 0)
        {
            free(node);
            continue;
        }
        node->when = (time_t)when_val;

        token = strtok(NULL, "|"); // status
        if (!token)
        {
            free(node);
            continue;
        }
        endptr = NULL;
        long status_val = strtol(token, &endptr, 10);
        if (token[0] == '\0' || *endptr != '\0' || status_val < 0 || status_val >= VISIT_STATUS_COUNT)
        {
            free(node);
            continue;
        }
        node->status = (int)status_val;

        token = strtok(NULL, "|"); // diagnosis (可选，未来可能添加)
        if (token)
        {
            strncpy(node->diagnosis, token, sizeof(node->diagnosis) - 1);
            node->diagnosis[sizeof(node->diagnosis) - 1] = '\0';
        }
        else
        {
            node->diagnosis[0] = '\0';
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

/* 将看诊数据保存到文件 */
int save_visits_to_file(Visit *head)
{
    char tmp_path[MAX_LINE_LEN];
    FILE *fp = safe_fopen_tmp(VISITS_FILE, tmp_path, sizeof(tmp_path));
    if (!fp)
        return -1;

    Visit *current = head;
    while (current)
    {
        fprintf(fp, "%s|%s|%lld|%d|%s\n", current->visit_id, current->reg_id, (long long)current->when, current->status,
                current->diagnosis);
        current = current->next;
    }

    return safe_fclose_commit(fp, tmp_path, VISITS_FILE);
}

/* 释放看诊数据内存 */
void free_visits(Visit *head)
{
    Visit *current = head;
    while (current)
    {
        Visit *temp = current;
        current = current->next;
        free(temp);
    }
}

static const char *get_patient_id_by_reg_id(Registration *reg_head, const char *reg_id)
{
    if (!reg_id)
        return "未知患者";
    for (Registration *r = reg_head; r; r = r->next)
        if (strcmp(r->reg_id, reg_id) == 0)
            return r->p_id;
    return "未知患者";
}

static const char *get_doctor_id_by_reg_id(Registration *reg_head, const char *reg_id)
{
    if (!reg_id)
        return "未知医生";
    for (Registration *r = reg_head; r; r = r->next)
        if (strcmp(r->reg_id, reg_id) == 0)
            return r->d_id;
    return "未知医生";
}

static const char *get_patient_name_by_patient_id(Patient *p_head, const char *p_id)
{
    if (!p_id)
        return "未知患者";
    for (Patient *p = p_head; p; p = p->next)
        if (strcmp(p->id, p_id) == 0)
            return p->name;
    return "未知患者";
}

/* 根据看诊ID查找看诊 */
Visit *find_visit_by_v_id(Visit *v_head, const char *v_id)
{
    if (!v_id)
        return NULL;
    for (Visit *cur = v_head; cur; cur = cur->next)
        if (strcmp(cur->visit_id, v_id) == 0)
            return cur;
    return NULL;
}

/* 根据患者ID查找看诊 */
Visit *find_visit_by_p_id(Visit *v_head, Registration *reg_head, const char *p_id)
{
    if (!p_id)
        return NULL;
    for (Visit *cur = v_head; cur; cur = cur->next)
    {
        const char *reg_p_id = get_patient_id_by_reg_id(reg_head, cur->reg_id);
        if (strcmp(reg_p_id, p_id) == 0)
            return cur;
    }
    return NULL;
}

/* 根据医生ID查找看诊 */
Visit *find_visit_by_d_id(Visit *v_head, Registration *reg_head, const char *d_id)
{
    if (!d_id)
        return NULL;
    for (Visit *cur = v_head; cur; cur = cur->next)
    {
        const char *reg_d_id = get_doctor_id_by_reg_id(reg_head, cur->reg_id);
        if (strcmp(reg_d_id, d_id) == 0)
            return cur;
    }
    return NULL;
}

/* 生成下一个看诊ID(Vxxxx) */
int generate_next_visit_id(Visit *head)
{
    int max_id = 0;
    Visit *current = head;
    while (current)
    {
        const char *id = current->visit_id;
        if (id[0] == 'V')
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

/* 打印看诊信息行 */
void print_visit(Visit *v, Registration *reg_head, Patient *p_head, Doctor *d_head, int v_w, int p_w, int d_w,
                 int dept_w, int when_w, int st_w, int diag_w)
{
    char when_buf[32];
    char st_buf[16];

    const char *p_id = get_patient_id_by_reg_id(reg_head, v->reg_id);
    const char *p_name = get_patient_name_by_patient_id(p_head, p_id);
    const char *d_id = get_doctor_id_by_reg_id(reg_head, v->reg_id);
    Doctor *doc = find_doctor_by_d_id(d_head, d_id);
    const char *d_name = doc ? doc->name : "未知医生";
    const char *dept = doc ? doc->department : "未知科室";

    if (format_beijing_time(v->when, when_buf, sizeof(when_buf)) == 0)
        strncpy(when_buf, "时间无效", sizeof(when_buf) - 1);
    when_buf[sizeof(when_buf) - 1] = '\0';

    if (v->status >= 0 && v->status < VISIT_STATUS_COUNT)
        strncpy(st_buf, VISIT_STATUS_TEXT[v->status], sizeof(st_buf) - 1);
    else
        strncpy(st_buf, "未知", sizeof(st_buf) - 1);
    st_buf[sizeof(st_buf) - 1] = '\0';

    printf("| ");
    print_align(v->visit_id, v_w);
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
    printf(" | ");
    print_align((v->diagnosis[0] ? v->diagnosis : "暂无"), diag_w);
    printf(" |\n");
}

/* 打印看诊表头 */
void print_visit_header(int v_w, int p_w, int d_w, int dept_w, int when_w, int st_w, int diag_w)
{
    print_visit_line(v_w, p_w, d_w, dept_w, when_w, st_w, diag_w);
    printf("| ");
    print_align("看诊ID", v_w);
    printf(" | ");
    print_align("患者姓名", p_w);
    printf(" | ");
    print_align("医生姓名", d_w);
    printf(" | ");
    print_align("科室", dept_w);
    printf(" | ");
    print_align("看诊时间", when_w);
    printf(" | ");
    print_align("状态", st_w);
    printf(" | ");
    print_align("诊断", diag_w);
    printf(" |\n");
    print_visit_line(v_w, p_w, d_w, dept_w, when_w, st_w, diag_w);
}

/* 打印看诊分隔线 */
void print_visit_line(int v_w, int p_w, int d_w, int dept_w, int when_w, int st_w, int diag_w)
{
    printf("+");
    for (int i = 0; i < v_w + 2; i++)
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
    printf("+");
    for (int i = 0; i < diag_w + 2; i++)
        printf("-");
    printf("+\n");
}

/* 计算看诊表格列宽 */
void calc_visit_width(Visit *v_head, Registration *reg_head, Patient *p_head, Doctor *d_head, int *v_w, int *p_w,
                      int *d_w, int *dept_w, int *when_w, int *st_w, int *diag_w)
{
    *v_w = str_width("看诊ID");
    *p_w = str_width("患者姓名");
    *d_w = str_width("医生姓名");
    *dept_w = str_width("科室");
    *when_w = str_width("看诊时间");
    *st_w = str_width("状态");
    *diag_w = str_width("诊断");

    for (Visit *v = v_head; v; v = v->next)
    {
        int w;
        char when_buf[32];
        const char *st = "未知";

        const char *p_id = get_patient_id_by_reg_id(reg_head, v->reg_id);
        const char *p_name = get_patient_name_by_patient_id(p_head, p_id);
        const char *d_id = get_doctor_id_by_reg_id(reg_head, v->reg_id);
        Doctor *doc = find_doctor_by_d_id(d_head, d_id);
        const char *d_name = doc ? doc->name : "未知医生";
        const char *dept = doc ? doc->department : "未知科室";

        w = str_width(v->visit_id);
        if (w > *v_w)
            *v_w = w;
        w = str_width(p_name);
        if (w > *p_w)
            *p_w = w;
        w = str_width(d_name);
        if (w > *d_w)
            *d_w = w;
        w = str_width(dept);
        if (w > *dept_w)
            *dept_w = w;
        if (format_beijing_time(v->when, when_buf, sizeof(when_buf)) == 0)
            strncpy(when_buf, "时间无效", sizeof(when_buf) - 1);
        w = str_width(when_buf);
        if (w > *when_w)
            *when_w = w;

        if (v->status >= 0 && v->status < VISIT_STATUS_COUNT)
            st = VISIT_STATUS_TEXT[v->status];
        w = str_width(st);
        if (w > *st_w)
            *st_w = w;

        w = str_width(v->diagnosis);
        if (w > *diag_w)
            *diag_w = w;
    }
}

/*
 * 看诊功能函数
 */

/* 创建看诊 */
Visit create_visit(const char *visit_id, const char *reg_id, time_t when, int status, const char *diagnosis)
{
    Visit visit;
    memset(&visit, 0, sizeof(Visit));

    strncpy(visit.visit_id, visit_id, sizeof(visit.visit_id) - 1);
    visit.visit_id[sizeof(visit.visit_id) - 1] = '\0';

    strncpy(visit.reg_id, reg_id, sizeof(visit.reg_id) - 1);
    visit.reg_id[sizeof(visit.reg_id) - 1] = '\0';

    visit.when = when;
    visit.status = status;

    if (diagnosis)
    {
        strncpy(visit.diagnosis, diagnosis, sizeof(visit.diagnosis) - 1);
        visit.diagnosis[sizeof(visit.diagnosis) - 1] = '\0';
    }
    else
    {
        visit.diagnosis[0] = '\0';
    }

    return visit;
}

/* 尾插看诊 */
void append_visit(Visit **head, Visit *new_visit)
{
    if (!head || !new_visit)
        return;

    new_visit->next = NULL;

    if (!*head)
    {
        *head = new_visit;
        return;
    }

    Visit *current = *head;
    while (current->next)
        current = current->next;
    current->next = new_visit;
}

/* 更新诊断结果 */
void update_diagnosis(Visit *visit, const char *diagnosis)
{
    if (!visit)
        return;

    if (diagnosis == NULL)
        diagnosis = "";

    visit->diagnosis[0] = '\0';
    strncat(visit->diagnosis, diagnosis, sizeof(visit->diagnosis) - 1);
}

/*
 *看诊系统功能
 */

/* 医生看诊患者 */
void doctor_visit_patient(Visit *v_head, const char *v_id, Exam **e_head, Registration *r_head, Patient *p_head,
                          Doctor *d_head)
{
    char buf[MAX_INPUT_LEN];
    int select;

    while (1)
    {
        clear_screen();
        printf("===== 看诊中（%s） =====\n", v_id);
        printf("1.开诊断\n");
        printf("2.开检查\n");
        printf("3.开处方\n");
        printf("4.办理住院\n");
        printf("5.办理出院\n");
        printf("0.结束看诊\n");

        printf("请输入您的选择: ");
        safe_input(buf, MAX_INPUT_LEN);

        if (!validate_choice(buf, 5))
        {
            printf("输入有误，请重新选择！\n");
            wait_enter();
            continue;
        }

        select = atoi(buf);

        switch (select)
        {
        case 1:
        {
            char diagnosis[MAX_LINE_LEN];
            while (1)
            {
                Visit *visit = find_visit_by_v_id(v_head, v_id);
                if (!visit)
                {
                    printf("未找到看诊记录！\n");
                    wait_enter();
                    break;
                }
                strncpy(diagnosis, visit->diagnosis, sizeof(diagnosis) - 1);
                diagnosis[sizeof(diagnosis) - 1] = '\0';
                if (strlen(diagnosis) > 0)
                {
                    printf("当前诊断结果: %s\n", diagnosis);
                    printf("请修改诊断结果(输入0返回): ");
                }
                else
                {
                    printf("当前诊断结果: 暂无\n");
                    printf("请填写诊断结果(输入0返回): ");
                }
                safe_input(diagnosis, sizeof(diagnosis));
                if (strcmp(diagnosis, "0") == 0)
                    break;
                update_diagnosis(visit, diagnosis);
                if (save_visits_to_file(v_head))
                {
                    printf("保存诊断结果失败！\n");
                }
                else
                {
                    printf("诊断结果已保存！\n");
                }
                wait_enter();
                clear_screen();
                break;
            }
            break;
        }
        case 2:
        {
            while (1)
            {
                clear_screen();
                printf("===== 检查管理（看诊ID: %s）=====\n", v_id);

                int has_exam = 0;

                for (Exam *e = *e_head; e; e = e->next)
                {
                    if (strcmp(e->visit_id, v_id) == 0)
                    {
                        has_exam = 1;
                        break;
                    }
                }

                if (!has_exam)
                {
                    printf("暂无已开检查记录。\n");
                }
                else
                {
                    int exam_w, visit_w, p_w, d_w, dept_w, item_w, result_w;
                    calc_exam_width(*e_head, v_head, r_head, p_head, d_head, &exam_w, &visit_w, &p_w, &d_w, &dept_w,
                                    &item_w, &result_w);

                    print_exam_header(exam_w, visit_w, p_w, d_w, dept_w, item_w, result_w);

                    for (Exam *e = *e_head; e; e = e->next)
                    {
                        if (strcmp(e->visit_id, v_id) == 0)
                        {
                            print_exam(e, v_head, r_head, p_head, d_head, exam_w, visit_w, p_w, d_w, dept_w, item_w,
                                       result_w);
                        }
                    }

                    print_exam_line(exam_w, visit_w, p_w, d_w, dept_w, item_w, result_w);
                }

                printf("\n1. 新增检查项目\n");
                printf("2. 填写/修改检查结果\n");
                printf("0. 返回上一级\n");
                printf("请输入您的选择: ");

                char op_buf[MAX_INPUT_LEN];
                safe_input(op_buf, sizeof(op_buf));
                if (!validate_choice(op_buf, 2))
                {
                    printf("输入有误，请重新选择！\n");
                    wait_enter();
                    continue;
                }

                int op = atoi(op_buf);

                if (op == 0)
                    break;

                if (op == 1)
                {
                    char item[MAX_ITEM_LEN];
                    printf("请输入检查项目(输入0返回): ");
                    safe_input(item, sizeof(item));

                    if (strcmp(item, "0") == 0)
                        continue;

                    if (item[0] == '\0')
                    {
                        printf("输入错误！检查项目不能为空。\n");
                        wait_enter();
                        continue;
                    }

                    char exam_id[MAX_ID_LEN];
                    int next_num = generate_next_exam_id(*e_head);
                    snprintf(exam_id, sizeof(exam_id), "E%04d", next_num);

                    Exam *new_node = (Exam *)malloc(sizeof(Exam));
                    if (!new_node)
                    {
                        printf("内存分配失败！\n");
                        wait_enter();
                        continue;
                    }

                    *new_node = create_exam(exam_id, v_id, item, "");
                    new_node->next = NULL;
                    append_exam(e_head, new_node);

                    if (save_exams_to_file(*e_head) != 0)
                        printf("保存检查信息失败！\n");
                    else
                        printf("检查已开立！检查ID: %s\n", exam_id);

                    wait_enter();
                    continue;
                }

                if (op == 2)
                {
                    char exam_id[MAX_ID_LEN];
                    printf("请输入要填写结果的检查ID(输入0返回): ");
                    safe_input(exam_id, sizeof(exam_id));

                    if (strcmp(exam_id, "0") == 0)
                        continue;

                    Exam *target = NULL;
                    for (Exam *e = *e_head; e; e = e->next)
                    {
                        if (strcmp(e->exam_id, exam_id) == 0 && strcmp(e->visit_id, v_id) == 0)
                        {
                            target = e;
                            break;
                        }
                    }

                    if (!target)
                    {
                        printf("未找到该检查ID，或该检查不属于当前看诊。\n");
                        wait_enter();
                        continue;
                    }

                    printf("当前检查项目: %s\n", target->item);
                    printf("当前检查结果: %s\n", target->result[0] ? target->result : "暂无");

                    char result[MAX_LINE_LEN];
                    printf("请输入检查结果(输入0返回): ");
                    safe_input(result, sizeof(result));

                    if (strcmp(result, "0") == 0)
                        continue;

                    if (result[0] == '\0')
                    {
                        printf("输入错误！检查结果不能为空。\n");
                        wait_enter();
                        continue;
                    }

                    strncpy(target->result, result, sizeof(target->result) - 1);
                    target->result[sizeof(target->result) - 1] = '\0';

                    if (save_exams_to_file(*e_head) != 0)
                        printf("保存检查结果失败！\n");
                    else
                        printf("检查结果已保存！\n");

                    wait_enter();
                    continue;
                }
            }
            break;
        }
        case 3:
        {
            Prescription *pr_head = load_prescriptions_from_file();
            Drug *drug_head = load_drugs_from_file();

            Visit *curr_v = find_visit_by_v_id(v_head, v_id);
            Registration *curr_r = curr_v ? find_registration_by_r_id(r_head, curr_v->reg_id) : NULL;

            if (!curr_v || !curr_r)
            {
                printf("数据异常：无法关联看诊或挂号记录！\n");
                wait_enter();
                if (pr_head)
                    free_prescriptions(pr_head);
                if (drug_head)
                    free_drugs(drug_head);
                break;
            }

            while (1)
            {
                clear_screen();
                printf("===== 处方管理（看诊ID: %s）=====\n", v_id);

                int has_pr = 0;
                for (Prescription *pr = pr_head; pr; pr = pr->next)
                {
                    if (strcmp(pr->visit_id, v_id) == 0)
                    {
                        has_pr = 1;
                        break;
                    }
                }

                if (!has_pr)
                {
                    printf("暂无已开药品处方记录。\n");
                }
                else
                {
                    int pr_w, visit_w, d_w, p_w, drug_w, dose_w, freq_w;
                    calc_prescription_width(pr_head, p_head, d_head, drug_head, &pr_w, &visit_w, &d_w, &p_w, &drug_w,
                                            &dose_w, &freq_w);

                    print_prescription_header(pr_w, visit_w, d_w, p_w, drug_w, dose_w, freq_w);
                    for (Prescription *pr = pr_head; pr; pr = pr->next)
                    {
                        if (strcmp(pr->visit_id, v_id) == 0)
                        {
                            print_prescription(pr, p_head, d_head, drug_head, pr_w, visit_w, d_w, p_w, drug_w, dose_w,
                                               freq_w);
                        }
                    }
                    print_prescription_line(pr_w, visit_w, d_w, p_w, drug_w, dose_w, freq_w);
                }

                printf("\n1. 新增处方药品\n");
                printf("2. 删除处方药品\n");
                printf("0. 返回上一级\n");
                printf("请输入您的选择: ");

                char op_buf[MAX_INPUT_LEN];
                safe_input(op_buf, sizeof(op_buf));
                if (!validate_choice(op_buf, 2))
                {
                    printf("输入有误，请重新选择！\n");
                    wait_enter();
                    continue;
                }

                int op = atoi(op_buf);
                if (op == 0)
                    break;

                if (op == 1)
                {
                    char drug_id[MAX_ID_LEN], dose[MAX_DOSE_LEN], freq[MAX_FREQ_LEN];

                    printf("请输入药品ID(输入0返回): ");
                    safe_input(drug_id, sizeof(drug_id));
                    if (strcmp(drug_id, "0") == 0)
                        continue;

                    Drug *drug = find_drug_by_id(drug_head, drug_id);
                    if (!drug)
                    {
                        printf("未找到该药品ID，请核对！\n");
                        wait_enter();
                        continue;
                    }

                    printf("已选中药品 [%s - %s]\n", drug->id, drug->generic_name);

                    printf("请输入剂量(如 1粒, 10ml, 输入0返回): ");
                    safe_input(dose, sizeof(dose));
                    if (strcmp(dose, "0") == 0)
                        continue;
                    if (dose[0] == '\0')
                    {
                        printf("剂量不能为空！\n");
                        wait_enter();
                        continue;
                    }

                    printf("请输入频次(如 1日3次, 输入0返回): ");
                    safe_input(freq, sizeof(freq));
                    if (strcmp(freq, "0") == 0)
                        continue;
                    if (freq[0] == '\0')
                    {
                        printf("频次不能为空！\n");
                        wait_enter();
                        continue;
                    }

                    char new_pr_id[MAX_ID_LEN];
                    int next_num = generate_next_prescription_id(pr_head);
                    snprintf(new_pr_id, sizeof(new_pr_id), "PR%04d", next_num);

                    Prescription *new_pr = (Prescription *)malloc(sizeof(Prescription));
                    if (!new_pr)
                    {
                        printf("内存分配失败！\n");
                        wait_enter();
                        continue;
                    }

                    *new_pr = create_prescription(new_pr_id, v_id, curr_r->p_id, curr_r->d_id, drug_id, dose, freq);
                    new_pr->next = NULL;

                    append_prescription(&pr_head, new_pr);

                    if (save_prescriptions_to_file(pr_head) != 0)
                        printf("保存处方失败！\n");
                    else
                        printf("处方开立成功！处方ID: %s\n", new_pr_id);

                    wait_enter();
                    continue;
                }

                if (op == 2)
                {
                    char del_id[MAX_ID_LEN];
                    printf("请输入要删除的处方ID(输入0返回): ");
                    safe_input(del_id, sizeof(del_id));
                    if (strcmp(del_id, "0") == 0)
                        continue;

                    Prescription *prev = NULL, *cur = pr_head;
                    int found = 0;
                    while (cur)
                    {
                        if (strcmp(cur->pr_id, del_id) == 0 && strcmp(cur->visit_id, v_id) == 0)
                        {
                            if (prev)
                                prev->next = cur->next;
                            else
                                pr_head = cur->next;
                            free(cur);
                            found = 1;
                            break;
                        }
                        prev = cur;
                        cur = cur->next;
                    }

                    if (found)
                    {
                        if (save_prescriptions_to_file(pr_head) != 0)
                            printf("保存处方信息失败！\n");
                        else
                            printf("删除成功！\n");
                    }
                    else
                    {
                        printf("未找到该处方记录，或该处方不属于当前看诊！\n");
                    }
                    wait_enter();
                    continue;
                }
            }

            free_prescriptions(pr_head);
            free_drugs(drug_head);
            break;
        }
        case 4:
            doctor_admit_patient_hospitalization(v_id);
            break;
        case 5:
            doctor_discharge_patient_hospitalization(v_id);
            break;
        case 0:
            printf("结束看诊！\n");
            wait_enter();
            clear_screen();
            return;
        default:
            printf("未知错误: %d\n", select);
            break;
        }
    }
}
