/*
 * 住院模块
 */
#include "model/hospitalization.h"
#include "core/session.h"
#include "core/utils.h"
#include "model/bed.h"
#include "model/doctor.h"
#include "model/patient.h"
#include "model/registration.h"
#include "model/visit.h"
#include "model/ward.h"

/*
 * 住院基础功能
 */

/* 从文件中加载住院数据，文件格式: hosp_id|visit_id|p_id|ward_id|bed_id|admit_date|discharge_date|status */
Hospitalization *load_hospitalizations_from_file(void)
{
    FILE *fp = fopen(HOSPITALIZATIONS_FILE, "r");
    if (!fp)
        return NULL;

    Hospitalization *head = NULL, *tail = NULL;
    char line[MAX_LINE_LEN];

    while (fgets(line, sizeof(line), fp))
    {
        trim_newline(line);
        if (!line[0])
            continue;

        Hospitalization *node = (Hospitalization *)malloc(sizeof(Hospitalization));
        if (!node)
        {
            fclose(fp);
            free_hospitalizations(head);
            return NULL;
        }
        memset(node, 0, sizeof(Hospitalization));

        /* 获取hosp_id */
        char *token = strtok(line, "|");
        if (!token)
        {
            free(node);
            continue;
        }
        strncpy(node->hosp_id, token, sizeof(node->hosp_id) - 1);

        /* 获取visit_id */
        token = strtok(NULL, "|");
        if (!token)
        {
            free(node);
            continue;
        }
        strncpy(node->visit_id, token, sizeof(node->visit_id) - 1);

        /* 获取p_id */
        token = strtok(NULL, "|");
        if (!token)
        {
            free(node);
            continue;
        }
        strncpy(node->p_id, token, sizeof(node->p_id) - 1);

        /* 获取ward_id */
        token = strtok(NULL, "|");
        if (!token)
        {
            free(node);
            continue;
        }
        strncpy(node->ward_id, token, sizeof(node->ward_id) - 1);

        /* 获取bed_id */
        token = strtok(NULL, "|");
        if (!token)
        {
            free(node);
            continue;
        }
        strncpy(node->bed_id, token, sizeof(node->bed_id) - 1);

        /* 获取admit_date */
        token = strtok(NULL, "|");
        if (!token)
        {
            free(node);
            continue;
        }
        char *endptr = NULL;
        long long admit_val = strtoll(token, &endptr, 10);
        if (token[0] == '\0' || *endptr != '\0' || admit_val < 0)
        {
            free(node);
            continue;
        }
        node->admit_date = (time_t)admit_val;

        /* 获取discharge_date */
        token = strtok(NULL, "|");
        if (!token)
        {
            free(node);
            continue;
        }
        endptr = NULL;
        long long discharge_val = strtoll(token, &endptr, 10);
        if (token[0] == '\0' || *endptr != '\0' || discharge_val < 0)
        {
            free(node);
            continue;
        }
        node->discharge_date = (time_t)discharge_val;

        /* 获取status */
        token = strtok(NULL, "|");
        if (!token)
        {
            free(node);
            continue;
        }
        endptr = NULL;
        long status_val = strtol(token, &endptr, 10);
        if (token[0] == '\0' || *endptr != '\0' || status_val < 0 || status_val >= HOSP_STATUS_COUNT)
        {
            free(node);
            continue;
        }
        node->status = (int)status_val;

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

/* 将住院数据保存到文件 */
int save_hospitalizations_to_file(Hospitalization *head)
{
    char tmp_path[MAX_LINE_LEN];
    FILE *fp = safe_fopen_tmp(HOSPITALIZATIONS_FILE, tmp_path, sizeof(tmp_path));
    if (!fp)
        return -1;

    for (Hospitalization *cur = head; cur; cur = cur->next)
        fprintf(fp, "%s|%s|%s|%s|%s|%lld|%lld|%d\n", cur->hosp_id, cur->visit_id, cur->p_id, cur->ward_id, cur->bed_id,
                (long long)cur->admit_date, (long long)cur->discharge_date, cur->status);

    return safe_fclose_commit(fp, tmp_path, HOSPITALIZATIONS_FILE);
}

/* 释放住院链表 */
void free_hospitalizations(Hospitalization *head)
{
    while (head)
    {
        Hospitalization *tmp = head;
        head = head->next;
        free(tmp);
    }
}

/* 按ID查找住院记录 */
Hospitalization *find_hospitalization_by_h_id(Hospitalization *head, const char *hosp_id)
{
    for (Hospitalization *cur = head; cur; cur = cur->next)
        if (strcmp(cur->hosp_id, hosp_id) == 0)
            return cur;
    return NULL;
}

/* 按看诊ID查找住院记录 */
Hospitalization *find_hospitalization_by_v_id(Hospitalization *head, const char *visit_id)
{
    for (Hospitalization *cur = head; cur; cur = cur->next)
        if (strcmp(cur->visit_id, visit_id) == 0)
            return cur;
    return NULL;
}

/* 根据患者ID查找住院记录 */
Hospitalization *find_hospitalization_by_p_id(Hospitalization *head, const char *p_id)
{
    for (Hospitalization *cur = head; cur; cur = cur->next)
        if (strcmp(cur->p_id, p_id) == 0)
            return cur;
    return NULL;
}

/* 查找患者住院中记录 */
Hospitalization *find_ongoing_hospitalization_by_p_id(Hospitalization *head, const char *p_id)
{
    for (Hospitalization *cur = head; cur; cur = cur->next)
        if (strcmp(cur->p_id, p_id) == 0 && cur->status == HOSP_STATUS_ONGOING)
            return cur;
    return NULL;
}

/* 生成下一个住院ID(Hxxxx) */
int generate_next_hospitalization_id(Hospitalization *head)
{
    int max_id = 0;
    for (Hospitalization *cur = head; cur; cur = cur->next)
    {
        const char *id = cur->hosp_id;
        if (id[0] == 'H')
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
                int n = atoi(id + 1);
                if (n > max_id)
                    max_id = n;
            }
        }
    }
    return max_id + 1;
}

/* 打印住院信息行 */
void print_hospitalization(Hospitalization *h, Visit *v_head, Registration *r_head, Patient *p_head, Ward *w_head,
                           Bed *b_head, int h_w, int v_w, int p_w, int ward_w, int bed_w, int in_w, int out_w, int st_w)
{
    (void)v_head;
    (void)r_head;
    (void)b_head;

    char in_buf[32], out_buf[32];
    if (!format_beijing_time(h->admit_date, in_buf, sizeof(in_buf)))
        strncpy(in_buf, "时间无效", sizeof(in_buf) - 1);
    in_buf[sizeof(in_buf) - 1] = '\0';

    if (h->discharge_date == 0)
        strncpy(out_buf, "未出院", sizeof(out_buf) - 1);
    else if (!format_beijing_time(h->discharge_date, out_buf, sizeof(out_buf)))
        strncpy(out_buf, "时间无效", sizeof(out_buf) - 1);
    out_buf[sizeof(out_buf) - 1] = '\0';

    Patient *p = find_patient_by_p_id(p_head, h->p_id);
    Ward *w = find_ward_by_w_id(w_head, h->ward_id);

    const char *p_name = p ? p->name : h->p_id;
    const char *ward_name = w ? w->name : h->ward_id;

    printf("| ");
    print_align(h->hosp_id, h_w);
    printf(" | ");
    print_align(h->visit_id, v_w);
    printf(" | ");
    print_align(p_name, p_w);
    printf(" | ");
    print_align(ward_name, ward_w);
    printf(" | ");
    print_align(h->bed_id, bed_w);
    printf(" | ");
    print_align(in_buf, in_w);
    printf(" | ");
    print_align(out_buf, out_w);
    printf(" | ");
    print_align(h->status == HOSP_STATUS_ONGOING ? "住院中" : "已出院", st_w);
    printf(" |\n");
}

/* 打印住院表头 */
void print_hospitalization_header(int h_w, int v_w, int p_w, int ward_w, int bed_w, int in_w, int out_w, int st_w)
{
    print_hospitalization_line(h_w, v_w, p_w, ward_w, bed_w, in_w, out_w, st_w);
    printf("| ");
    print_align("住院ID", h_w);
    printf(" | ");
    print_align("看诊ID", v_w);
    printf(" | ");
    print_align("患者姓名", p_w);
    printf(" | ");
    print_align("病房", ward_w);
    printf(" | ");
    print_align("床位", bed_w);
    printf(" | ");
    print_align("入院时间", in_w);
    printf(" | ");
    print_align("出院时间", out_w);
    printf(" | ");
    print_align("状态", st_w);
    printf(" |\n");
    print_hospitalization_line(h_w, v_w, p_w, ward_w, bed_w, in_w, out_w, st_w);
}

/* 打印住院分隔线 */
void print_hospitalization_line(int h_w, int v_w, int p_w, int ward_w, int bed_w, int in_w, int out_w, int st_w)
{
    printf("+");
    for (int i = 0; i < h_w + 2; i++)
        printf("-");
    printf("+");
    for (int i = 0; i < v_w + 2; i++)
        printf("-");
    printf("+");
    for (int i = 0; i < p_w + 2; i++)
        printf("-");
    printf("+");
    for (int i = 0; i < ward_w + 2; i++)
        printf("-");
    printf("+");
    for (int i = 0; i < bed_w + 2; i++)
        printf("-");
    printf("+");
    for (int i = 0; i < in_w + 2; i++)
        printf("-");
    printf("+");
    for (int i = 0; i < out_w + 2; i++)
        printf("-");
    printf("+");
    for (int i = 0; i < st_w + 2; i++)
        printf("-");
    printf("+\n");
}

/* 计算住院表格列宽 */
void calc_hospitalization_width(Hospitalization *h_head, Visit *v_head, Registration *r_head, Patient *p_head,
                                Ward *w_head, Bed *b_head, int *h_w, int *v_w, int *p_w, int *ward_w, int *bed_w,
                                int *in_w, int *out_w, int *st_w)
{
    (void)v_head;
    (void)r_head;
    (void)b_head;

    *h_w = str_width("住院ID");
    *v_w = str_width("看诊ID");
    *p_w = str_width("患者姓名");
    *ward_w = str_width("病房");
    *bed_w = str_width("床位");
    *in_w = str_width("入院时间");
    *out_w = str_width("出院时间");
    *st_w = str_width("状态");

    for (Hospitalization *h = h_head; h; h = h->next)
    {
        int width;
        char in_buf[32], out_buf[32];

        Patient *p = find_patient_by_p_id(p_head, h->p_id);
        Ward *w = find_ward_by_w_id(w_head, h->ward_id);
        const char *p_name = p ? p->name : h->p_id;
        const char *ward_name = w ? w->name : h->ward_id;

        format_beijing_time(h->admit_date, in_buf, sizeof(in_buf));
        if (h->discharge_date == 0)
        {
            strncpy(out_buf, "未出院", sizeof(out_buf) - 1);
            out_buf[sizeof(out_buf) - 1] = '\0';
        }
        else
            format_beijing_time(h->discharge_date, out_buf, sizeof(out_buf));

        width = str_width(h->hosp_id);
        if (width > *h_w)
            *h_w = width;
        width = str_width(h->visit_id);
        if (width > *v_w)
            *v_w = width;
        width = str_width(p_name);
        if (width > *p_w)
            *p_w = width;
        width = str_width(ward_name);
        if (width > *ward_w)
            *ward_w = width;
        width = str_width(h->bed_id);
        if (width > *bed_w)
            *bed_w = width;
        width = str_width(in_buf);
        if (width > *in_w)
            *in_w = width;
        width = str_width(out_buf);
        if (width > *out_w)
            *out_w = width;
        width = str_width(h->status == HOSP_STATUS_ONGOING ? "住院中" : "已出院");
        if (width > *st_w)
            *st_w = width;
    }
}

/* 统计住院记录数量 */
int count_hospitalizations(Hospitalization *head)
{
    int c = 0;
    while (head)
    {
        c++;
        head = head->next;
    }
    return c;
}

/* 获取第n个住院节点 */
Hospitalization *get_nth_hospitalization(Hospitalization *head, int n)
{
    for (int i = 0; i < n && head; i++)
    {
        head = head->next;
    }
    return head;
}

/* 统计医生名下的住院数量 */
int count_hospitalizations_for_doctor(Hospitalization *h_head, Visit *v_head, Registration *reg_head, const char *d_id)
{
    int count = 0;
    for (Hospitalization *h = h_head; h; h = h->next)
    {
        Visit *v = find_visit_by_v_id(v_head, h->visit_id);
        if (!v)
            continue;

        const char *h_d_id = get_doctor_id_by_reg_id(reg_head, v->reg_id);
        if (h_d_id && strcmp(h_d_id, d_id) == 0)
        {
            count++;
        }
    }
    return count;
}

/* 获取医生名下的第n个住院节点 */
Hospitalization *get_nth_hospitalization_for_doctor(Hospitalization *h_head, Visit *v_head, Registration *reg_head,
                                                    const char *d_id, int n)
{
    for (Hospitalization *h = h_head; h; h = h->next)
    {
        Visit *v = find_visit_by_v_id(v_head, h->visit_id);
        if (!v)
            continue;

        const char *h_d_id = get_doctor_id_by_reg_id(reg_head, v->reg_id);
        if (h_d_id && strcmp(h_d_id, d_id) == 0)
        {
            if (n == 0)
                return h;
            n--;
        }
    }
    return NULL;
}

/* 删除住院 */
void hospitalization_remove(Hospitalization **head, Hospitalization *target)
{
    if (!head || !*head || !target)
        return;

    if (*head == target)
    {
        *head = target->next;
        free(target);
        return;
    }

    Hospitalization *prev = *head;
    while (prev->next && prev->next != target)
        prev = prev->next;

    if (prev->next == target)
    {
        prev->next = target->next;
        free(target);
    }
    /* 若未找到 target 说明它不在链表中, 不处理 */
}

/*
 * 住院功能函数
 */

/* 创建住院记录 */
Hospitalization create_hospitalization(const char *hosp_id, const char *visit_id, const char *p_id, const char *ward_id,
                                       const char *bed_id, time_t admit_date, time_t discharge_date, int status)
{
    Hospitalization h;
    memset(&h, 0, sizeof(Hospitalization));
    strncpy(h.hosp_id, hosp_id, sizeof(h.hosp_id) - 1);
    strncpy(h.visit_id, visit_id, sizeof(h.visit_id) - 1);
    strncpy(h.p_id, p_id, sizeof(h.p_id) - 1);
    strncpy(h.ward_id, ward_id, sizeof(h.ward_id) - 1);
    strncpy(h.bed_id, bed_id, sizeof(h.bed_id) - 1);
    h.admit_date = admit_date;
    h.discharge_date = discharge_date;
    h.status = status;
    h.next = NULL;
    return h;
}

/* 尾插住院记录 */
void append_hospitalization(Hospitalization **head, Hospitalization *new_hosp)
{
    if (!head || !new_hosp)
        return;
    new_hosp->next = NULL;
    if (!*head)
    {
        *head = new_hosp;
        return;
    }

    Hospitalization *cur = *head;
    while (cur->next)
        cur = cur->next;
    cur->next = new_hosp;
}

/*
 * 入院操作（纯内存修改，不涉及文件IO）
 * 流程: 查重 → 优先在指定病房找空床 → 全院找空床 → 分配床位/更新占用数
 * 调用方负责将修改后的链表持久化到文件
 */
int admit_patient(Hospitalization **h_head, Ward *w_head, Bed *b_head, const char *visit_id, const char *p_id,
                  const char *preferred_ward_id)
{
    if (!h_head || !visit_id || !p_id)
        return 0;
    if (find_ongoing_hospitalization_by_p_id(*h_head, p_id))
        return 0; // 已住院中，不重复入院

    /* 优先在指定病房查找空床，找不到则全院范围查找 */
    Bed *bed = NULL;
    if (preferred_ward_id && preferred_ward_id[0] != '\0')
        bed = find_first_free_bed_in_ward(b_head, preferred_ward_id);
    if (!bed)
        bed = find_first_free_bed(b_head);
    if (!bed)
        return 0; // 无空床

    Ward *ward = find_ward_by_w_id(w_head, bed->ward_id);
    if (!ward)
        return 0;

    char hosp_id[MAX_ID_LEN];
    snprintf(hosp_id, sizeof(hosp_id), "H%04d", generate_next_hospitalization_id(*h_head));

    Hospitalization *node = (Hospitalization *)malloc(sizeof(Hospitalization));
    if (!node)
        return 0;

    *node = create_hospitalization(hosp_id, visit_id, p_id, bed->ward_id, bed->bed_id, time(NULL), 0,
                                   HOSP_STATUS_ONGOING);
    append_hospitalization(h_head, node);

    /* 同步更新床位状态和病房占用数（内存级，尚未持久化） */
    bed->status = BED_STATUS_OCCUPIED;
    ward->occupied += 1;

    return 1;
}

/*
 * 出院操作（纯内存修改，不涉及文件IO）
 * 流程: 释放床位 → 减少病房占用数 → 标记出院时间
 * 调用方负责将修改后的链表持久化到文件
 */
int discharge_patient(Hospitalization *h_head, Ward *w_head, Bed *b_head, const char *hosp_id)
{
    Hospitalization *h = find_hospitalization_by_h_id(h_head, hosp_id);
    if (!h || h->status != HOSP_STATUS_ONGOING)
        return 0;

    Bed *b = find_bed_by_b_id(b_head, h->bed_id);
    Ward *w = find_ward_by_w_id(w_head, h->ward_id);

    /* 释放关联的床位和病房占用计数 */
    if (b)
        b->status = BED_STATUS_FREE;
    if (w && w->occupied > 0)
        w->occupied -= 1;

    /* 标记出院状态和时间 */
    h->status = HOSP_STATUS_DISCHARGED;
    h->discharge_date = time(NULL);
    return 1;
}

/*
 * 住院业务函数
 * 以下函数涉及住院/出院/床位三张表的联动修改,
 * 采用"快照-修改-顺序保存-失败则恢复快照并重写"的回滚策略
 */

/*
 * 医生办理住院(看诊中调用)
 * 涉及三张表联动: hospitalizations + beds + wards
 * 保存策略: 三张表顺序写入, 任一失败则全部回滚
 */
void doctor_admit_patient_hospitalization(const char *v_id)
{
    Hospitalization *h_head = NULL;
    Ward *w_head = NULL;
    Bed *b_head = NULL;
    Visit *v_head = NULL;
    Registration *r_head = NULL;
    Patient *p_head = NULL;
    Doctor *d_head = NULL;

    int ok = 1;

    if (!g_session.logged_in || strcmp(g_session.role, "doctor") != 0)
    {
        printf("请先登录医生账号！\n");
        ok = 0;
    }

    if (ok && (!v_id || v_id[0] == '\0'))
    {
        printf("看诊ID无效！\n");
        ok = 0;
    }

    if (ok)
    {
        h_head = load_hospitalizations_from_file();
        w_head = load_wards_from_file();
        b_head = load_beds_from_file();
        v_head = load_visits_from_file();
        r_head = load_registrations_from_file();
        p_head = load_patients_from_file();
        d_head = load_doctors_from_file();

        if (!v_head || !r_head || !w_head || !b_head || !p_head || !d_head)
        {
            printf("住院办理所需数据不完整！\n");
            ok = 0;
        }
    }

    if (ok)
    {
        Visit *v = find_visit_by_v_id(v_head, v_id);
        if (!v)
        {
            printf("看诊ID不存在！\n");
            ok = 0;
        }
        else
        {
            Registration *r = find_registration_by_r_id(r_head, v->reg_id);
            if (!r)
            {
                printf("该看诊关联挂号不存在，数据异常！\n");
                ok = 0;
            }
            else if (strcmp(r->d_id, g_session.user_id) != 0)
            {
                printf("无权限办理：该看诊不属于当前医生！\n");
                ok = 0;
            }
            else if (v->status != VISIT_STATUS_ONGOING && v->status != VISIT_STATUS_DONE)
            {
                printf("当前看诊状态不允许办理住院！\n");
                ok = 0;
            }
            else if (find_hospitalization_by_v_id(h_head, v->visit_id))
            {
                printf("该看诊已存在住院记录，不能重复办理！\n");
                ok = 0;
            }
            else
            {
                int v_w, p_w, d_w, dept_w, when_w, st_w, diag_w;
                calc_visit_width(v_head, r_head, p_head, d_head, &v_w, &p_w, &d_w, &dept_w, &when_w, &st_w, &diag_w);

                Doctor *me = find_doctor_by_d_id(d_head, g_session.user_id);
                const char *my_dept = me ? me->department : "未知科室";

                clear_screen();
                printf("===== 办理住院 =====\n");
                print_visit_header(v_w, p_w, d_w, dept_w, when_w, st_w, diag_w);
                print_visit(v, r_head, p_head, d_head, v_w, p_w, d_w, dept_w, when_w, st_w, diag_w);
                print_visit_line(v_w, p_w, d_w, dept_w, when_w, st_w, diag_w);

                printf("当前医生科室：%s\n", my_dept);

                int w_id_w, w_name_w, w_type_w, w_dept_w, w_cap_w, w_occ_w;
                calc_ward_width(w_head, &w_id_w, &w_name_w, &w_type_w, &w_dept_w, &w_cap_w, &w_occ_w);
                print_ward_header(w_id_w, w_name_w, w_type_w, w_dept_w, w_cap_w, w_occ_w);
                for (Ward *w = w_head; w; w = w->next)
                    print_ward(w, w_id_w, w_name_w, w_type_w, w_dept_w, w_cap_w, w_occ_w);
                print_ward_line(w_id_w, w_name_w, w_type_w, w_dept_w, w_cap_w, w_occ_w);

                char preferred_ward_id[MAX_ID_LEN];
                preferred_ward_id[0] = '\0';
                printf("请输入优先病房ID(回车=自动分配，输入0取消): ");
                safe_input(preferred_ward_id, sizeof(preferred_ward_id));

                if (preferred_ward_id[0] == '0' && preferred_ward_id[1] == '\0')
                {
                    printf("已取消办理住院。\n");
                }
                else
                {
                    if (preferred_ward_id[0] != '\0')
                    {
                        Ward *preferred_ward = find_ward_by_w_id(w_head, preferred_ward_id);
                        if (!preferred_ward)
                        {
                            printf("病房ID不存在，系统将自动分配病房。\n");
                            preferred_ward_id[0] = '\0';
                        }
                        else if (preferred_ward->occupied >= preferred_ward->capacity)
                        {
                            printf("该病房已满，系统将自动分配病房。\n");
                            preferred_ward_id[0] = '\0';
                        }
                    }

                    char confirm[MAX_INPUT_LEN];
                    printf("确认办理住院？(y/n): ");
                    safe_input(confirm, sizeof(confirm));

                    if (!(confirm[0] == 'y' || confirm[0] == 'Y'))
                    {
                        printf("已取消办理住院。\n");
                    }
                    else
                    {
                        const char *auto_pref_ward_id = NULL;

                        /* 按科室字段匹配病房 */
                        if (preferred_ward_id[0] == '\0')
                        {
                            if (me && me->department[0] != '\0')
                            {
                                for (Ward *w = w_head; w; w = w->next)
                                {
                                    if (strcmp(w->department, me->department) == 0 && w->occupied < w->capacity)
                                    {
                                        auto_pref_ward_id = w->ward_id;
                                        break;
                                    }
                                }
                            }
                        }

                        /* 最终传给 admit_patient 的优先病房参数 */
                        const char *final_pref_ward_id = NULL;
                        if (preferred_ward_id[0] != '\0')
                            final_pref_ward_id = preferred_ward_id; // 手动输入优先级最高
                        else if (auto_pref_ward_id)
                            final_pref_ward_id = auto_pref_ward_id; // 自动按科室优先
                        else
                            final_pref_ward_id = NULL; // 最后全院自动

                        /* 提示 */
                        if (preferred_ward_id[0] == '\0')
                        {
                            if (auto_pref_ward_id)
                            {
                                Ward *auto_ward = find_ward_by_w_id(w_head, auto_pref_ward_id);
                                printf("系统自动优先分配当前科室病房：%s(%s)\n", auto_ward ? auto_ward->name : "未知",
                                       auto_pref_ward_id);
                            }
                            else
                                printf("当前科室无可用病房，系统将全院自动分配。\n");
                        }

                        /* 执行入院(内存修改: 新增住院记录+占用床位+病房计数) */
                        if (admit_patient(&h_head, w_head, b_head, v->visit_id, r->p_id, final_pref_ward_id))
                        {
                            /* 定位刚创建的记录和关联的床位/病房, 用于失败时回滚 */
                            Hospitalization *new_h = find_hospitalization_by_v_id(h_head, v->visit_id);
                            Bed *new_bed = new_h ? find_bed_by_b_id(b_head, new_h->bed_id) : NULL;
                            Ward *new_ward = new_h ? find_ward_by_w_id(w_head, new_h->ward_id) : NULL;

                            /* 顺序保存三张表 */
                            int s1 = (save_hospitalizations_to_file(h_head) == 0);
                            int s2 = (save_beds_to_file(b_head) == 0);
                            int s3 = (save_wards_to_file(w_head) == 0);

                            if (s1 && s2 && s3)
                                printf("患者已成功办理住院！\n");
                            else
                            {
                                /* 回滚: 从链表中摘除刚插入的住院节点 */
                                if (new_h)
                                {
                                    Hospitalization *cur = h_head, *prev = NULL;
                                    while (cur)
                                    {
                                        if (cur == new_h)
                                        {
                                            if (prev)
                                                prev->next = cur->next;
                                            else
                                                h_head = cur->next;
                                            free(cur);
                                            break;
                                        }
                                        prev = cur;
                                        cur = cur->next;
                                    }
                                }
                                /* 回滚: 恢复床位和病房占用计数 */
                                if (new_bed)
                                    new_bed->status = BED_STATUS_FREE;
                                if (new_ward && new_ward->occupied > 0)
                                    new_ward->occupied -= 1;

                                /* 将恢复后的状态重新写入全部文件 */
                                int rb1 = (save_hospitalizations_to_file(h_head) == 0);
                                int rb2 = (save_beds_to_file(b_head) == 0);
                                int rb3 = (save_wards_to_file(w_head) == 0);

                                printf("办理住院失败：保存文件失败。");
                                if (rb1 && rb2 && rb3)
                                    printf("已回滚。\n");
                                else
                                    printf("且回滚失败，请立即检查数据文件。\n");
                            }
                        }
                        else
                        {
                            printf("办理住院失败！可能原因：患者已住院中或无空床位。\n");
                        }
                    }
                }
            }
        }
    }

    wait_enter();
    clear_screen();

    free_hospitalizations(h_head);
    free_wards(w_head);
    free_beds(b_head);
    free_visits(v_head);
    free_registrations(r_head);
    free_patients(p_head);
    free_doctors(d_head);
}

/*
 * 医生办理出院(看诊中调用)
 * 涉及三张表联动: hospitalizations + beds + wards
 * 保存策略: 三张表顺序写入, 任一失败则全部回滚
 */
void doctor_discharge_patient_hospitalization(const char *v_id)
{
    Hospitalization *h_head = NULL;
    Ward *w_head = NULL;
    Bed *b_head = NULL;
    Visit *v_head = NULL;
    Registration *r_head = NULL;
    Patient *p_head = NULL;
    Doctor *d_head = NULL;

    int ok = 1;

    if (!g_session.logged_in || strcmp(g_session.role, "doctor") != 0)
    {
        printf("请先登录医生账号！\n");
        ok = 0;
    }

    if (ok && (!v_id || v_id[0] == '\0'))
    {
        printf("看诊ID无效！\n");
        ok = 0;
    }

    if (ok)
    {
        h_head = load_hospitalizations_from_file();
        w_head = load_wards_from_file();
        b_head = load_beds_from_file();
        v_head = load_visits_from_file();
        r_head = load_registrations_from_file();
        p_head = load_patients_from_file();
        d_head = load_doctors_from_file();

        if (!h_head || !v_head || !r_head || !w_head || !b_head || !p_head || !d_head)
        {
            printf("出院办理所需数据不完整！\n");
            ok = 0;
        }
    }

    if (ok)
    {
        Visit *v = find_visit_by_v_id(v_head, v_id);
        if (!v)
        {
            printf("看诊ID不存在！\n");
            ok = 0;
        }
        else
        {
            Registration *r = find_registration_by_r_id(r_head, v->reg_id);
            if (!r)
            {
                printf("该看诊关联挂号不存在，数据异常！\n");
                ok = 0;
            }
            else if (strcmp(r->d_id, g_session.user_id) != 0)
            {
                printf("无权限办理：该看诊不属于当前医生！\n");
                ok = 0;
            }
            else
            {
                Hospitalization *h = find_hospitalization_by_v_id(h_head, v_id);
                if (!h)
                {
                    printf("该看诊没有住院记录！\n");
                    ok = 0;
                }
                else if (h->status != HOSP_STATUS_ONGOING)
                {
                    printf("该住院记录已出院，无需重复办理！\n");
                    ok = 0;
                }
                else
                {
                    int h_w, vv_w, p_w, ward_w, bed_w, in_w, out_w, st_w;
                    calc_hospitalization_width(h_head, v_head, r_head, p_head, w_head, b_head, &h_w, &vv_w, &p_w,
                                               &ward_w, &bed_w, &in_w, &out_w, &st_w);

                    clear_screen();
                    printf("===== 办理出院 =====\n");
                    print_hospitalization_header(h_w, vv_w, p_w, ward_w, bed_w, in_w, out_w, st_w);
                    print_hospitalization(h, v_head, r_head, p_head, w_head, b_head, h_w, vv_w, p_w, ward_w, bed_w,
                                          in_w, out_w, st_w);
                    print_hospitalization_line(h_w, vv_w, p_w, ward_w, bed_w, in_w, out_w, st_w);

                    char confirm[MAX_INPUT_LEN];
                    printf("确认办理出院？(y/n): ");
                    safe_input(confirm, sizeof(confirm));

                    if (!(confirm[0] == 'y' || confirm[0] == 'Y'))
                    {
                        printf("已取消办理出院。\n");
                    }
                    else
                    {
                        /* 第一步: 快照出院前的床位/病房/住院状态 */
                        Bed *old_bed = find_bed_by_b_id(b_head, h->bed_id);
                        Ward *old_ward = find_ward_by_w_id(w_head, h->ward_id);
                        int old_bed_status = old_bed ? old_bed->status : -1;
                        int old_ward_occupied = old_ward ? old_ward->occupied : -1;
                        int old_h_status = h->status;
                        time_t old_discharge_date = h->discharge_date;

                        /* 第二步: 执行出院(内存修改) */
                        if (discharge_patient(h_head, w_head, b_head, h->hosp_id))
                        {
                            /* 第三步: 顺序保存三张表 */
                            int s1 = (save_hospitalizations_to_file(h_head) == 0);
                            int s2 = (save_beds_to_file(b_head) == 0);
                            int s3 = (save_wards_to_file(w_head) == 0);

                            if (s1 && s2 && s3)
                                printf("患者已成功办理出院！\n");
                            else
                            {
                                /* 第四步: 任一保存失败, 从快照恢复内存状态 */
                                if (old_bed)
                                    old_bed->status = old_bed_status;
                                if (old_ward)
                                    old_ward->occupied = old_ward_occupied;
                                h->status = old_h_status;
                                h->discharge_date = old_discharge_date;

                                /* 第五步: 将恢复后的状态重新写入全部文件 */
                                int rb1 = (save_hospitalizations_to_file(h_head) == 0);
                                int rb2 = (save_beds_to_file(b_head) == 0);
                                int rb3 = (save_wards_to_file(w_head) == 0);

                                printf("办理出院失败：保存文件失败。");
                                if (rb1 && rb2 && rb3)
                                    printf("已回滚。\n");
                                else
                                    printf("且回滚失败，请立即检查数据文件。\n");
                            }
                        }
                        else
                        {
                            printf("办理出院失败！\n");
                        }
                    }
                }
            }
        }
    }

    wait_enter();
    clear_screen();

    free_hospitalizations(h_head);
    free_wards(w_head);
    free_beds(b_head);
    free_visits(v_head);
    free_registrations(r_head);
    free_patients(p_head);
    free_doctors(d_head);
}
