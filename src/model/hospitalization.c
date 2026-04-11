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

/* 从文件中加载住院数据 */
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

        char *token = strtok(line, "|"); // hosp_id
        if (!token)
        {
            free(node);
            continue;
        }
        strncpy(node->hosp_id, token, sizeof(node->hosp_id) - 1);

        token = strtok(NULL, "|"); // visit_id
        if (!token)
        {
            free(node);
            continue;
        }
        strncpy(node->visit_id, token, sizeof(node->visit_id) - 1);

        token = strtok(NULL, "|"); // p_id
        if (!token)
        {
            free(node);
            continue;
        }
        strncpy(node->p_id, token, sizeof(node->p_id) - 1);

        token = strtok(NULL, "|"); // ward_id
        if (!token)
        {
            free(node);
            continue;
        }
        strncpy(node->ward_id, token, sizeof(node->ward_id) - 1);

        token = strtok(NULL, "|"); // bed_id
        if (!token)
        {
            free(node);
            continue;
        }
        strncpy(node->bed_id, token, sizeof(node->bed_id) - 1);

        token = strtok(NULL, "|"); // admit_date
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

        token = strtok(NULL, "|"); // discharge_date
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

        token = strtok(NULL, "|"); // status
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
        if (strcmp(cur->p_id, p_id) == 0 && cur->status == 0)
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
    print_align(h->status == 0 ? "住院中" : "已出院", st_w);
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
    print_align("患者", p_w);
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
    *p_w = str_width("患者");
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
        width = str_width(h->status == 0 ? "住院中" : "已出院");
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

/* 入院 */
int admit_patient(Hospitalization **h_head, Ward *w_head, Bed *b_head, const char *visit_id, const char *p_id,
                  const char *preferred_ward_id)
{
    if (!h_head || !visit_id || !p_id)
        return 0;
    if (find_ongoing_hospitalization_by_p_id(*h_head, p_id))
        return 0; // 已住院中，不重复入院

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

    *node = create_hospitalization(hosp_id, visit_id, p_id, bed->ward_id, bed->bed_id, time(NULL), 0, 0);
    append_hospitalization(h_head, node);

    bed->status = 1;
    ward->occupied += 1;

    return 1;
}

/* 出院 */
int discharge_patient(Hospitalization *h_head, Ward *w_head, Bed *b_head, const char *hosp_id)
{
    Hospitalization *h = find_hospitalization_by_h_id(h_head, hosp_id);
    if (!h || h->status != 0)
        return 0;

    Bed *b = find_bed_by_b_id(b_head, h->bed_id);
    Ward *w = find_ward_by_w_id(w_head, h->ward_id);

    if (b)
        b->status = 0;
    if (w && w->occupied > 0)
        w->occupied -= 1;

    h->status = 1;
    h->discharge_date = time(NULL);
    return 1;
}

/*
 *住院业务函数
 */

/* 医生办理住院 */
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
                            final_pref_ward_id = preferred_ward_id; /* 手动输入优先级最高 */
                        else if (auto_pref_ward_id)
                            final_pref_ward_id = auto_pref_ward_id; /* 自动按科室优先 */
                        else
                            final_pref_ward_id = NULL; /* 最后全院自动 */

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

                        if (admit_patient(&h_head, w_head, b_head, v->visit_id, r->p_id, final_pref_ward_id))
                        {
                            Hospitalization *new_h = find_hospitalization_by_v_id(h_head, v->visit_id);
                            Bed *new_bed = new_h ? find_bed_by_b_id(b_head, new_h->bed_id) : NULL;
                            Ward *new_ward = new_h ? find_ward_by_w_id(w_head, new_h->ward_id) : NULL;

                            int s1 = (save_hospitalizations_to_file(h_head) == 0);
                            int s2 = (save_beds_to_file(b_head) == 0);
                            int s3 = (save_wards_to_file(w_head) == 0);

                            if (s1 && s2 && s3)
                                printf("患者已成功办理住院！\n");
                            else
                            {
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
                                if (new_bed)
                                    new_bed->status = BED_STATUS_FREE;
                                if (new_ward && new_ward->occupied > 0)
                                    new_ward->occupied -= 1;

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

/* 医生办理出院 */
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
                else if (h->status != 0)
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
                        Bed *old_bed = find_bed_by_b_id(b_head, h->bed_id);
                        Ward *old_ward = find_ward_by_w_id(w_head, h->ward_id);
                        int old_bed_status = old_bed ? old_bed->status : -1;
                        int old_ward_occupied = old_ward ? old_ward->occupied : -1;
                        int old_h_status = h->status;
                        time_t old_discharge_date = h->discharge_date;

                        if (discharge_patient(h_head, w_head, b_head, h->hosp_id))
                        {
                            int s1 = (save_hospitalizations_to_file(h_head) == 0);
                            int s2 = (save_beds_to_file(b_head) == 0);
                            int s3 = (save_wards_to_file(w_head) == 0);

                            if (s1 && s2 && s3)
                                printf("患者已成功办理出院！\n");
                            else
                            {
                                if (old_bed)
                                    old_bed->status = old_bed_status;
                                if (old_ward)
                                    old_ward->occupied = old_ward_occupied;
                                h->status = old_h_status;
                                h->discharge_date = old_discharge_date;

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

/* 添加住院记录 */
void add_hospitalization_record()
{
    char visit_id[MAX_ID_LEN];
    char p_id[MAX_ID_LEN];
    char preferred_ward_id[MAX_ID_LEN];

    Hospitalization *h_head = load_hospitalizations_from_file();
    Ward *w_head = load_wards_from_file();
    Bed *b_head = load_beds_from_file();
    Registration *r_head = load_registrations_from_file();
    Visit *v_head = load_visits_from_file();
    Patient *p_head = load_patients_from_file();
    Doctor *d_head = load_doctors_from_file();

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
            continue;
        }

        Visit *visit = find_visit_by_v_id(v_head, visit_id);

        if (!visit)
        {
            printf("未找到该看诊ID！\n");
            wait_enter();
            clear_screen();
            continue;
        }

        printf("请输入患者ID(输入0返回): ");
        safe_input(p_id, sizeof(p_id));
        if (strcmp(p_id, "0") == 0)
            break;

        if (p_id[0] == '\0')
        {
            printf("输入错误！患者ID不能为空。\n");
            wait_enter();
            clear_screen();
            continue;
        }

        if (!find_patient_by_p_id(p_head, p_id))
        {
            printf("未找到该患者ID！\n");
            wait_enter();
            clear_screen();
            continue;
        }

        Registration *reg = find_registration_by_r_id(r_head, visit->reg_id);
        if (!reg)
        {
            printf("看诊记录关联的挂号记录不存在，数据异常！\n");
            wait_enter();
            clear_screen();
            continue;
        }

        if (strcmp(reg->p_id, p_id) != 0)
        {
            printf("看诊ID与患者ID不匹配！\n");
            wait_enter();
            clear_screen();
            continue;
        }

        if (find_ongoing_hospitalization_by_p_id(h_head, p_id))
        {
            printf("该患者已住院中！\n");
            wait_enter();
            clear_screen();
            continue;
        }

        printf("请输入优先病房ID(回车=自动分配，输入0返回): ");
        safe_input(preferred_ward_id, sizeof(preferred_ward_id));
        if (strcmp(preferred_ward_id, "0") == 0) // 0 代表取消
            break;

        if (preferred_ward_id[0] != '\0') // 用户输入了优先病房ID，进行验证
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
        printf("确认添加住院记录？(y/n): ");
        safe_input(confirm, sizeof(confirm));

        if (!(confirm[0] == 'y' || confirm[0] == 'Y'))
        {
            printf("已取消添加住院记录\n");
            wait_enter();
            clear_screen();
            continue;
        }
        else
        {
            const char *auto_pref_ward_id = NULL;

            /* 按科室字段匹配病房 */
            if (preferred_ward_id[0] == '\0')
            {
                Doctor *doc = find_doctor_by_d_id(d_head, reg->d_id);
                if (doc && doc->department[0] != '\0')
                {
                    for (Ward *w = w_head; w; w = w->next)
                    {
                        if (strcmp(w->department, doc->department) == 0 && w->occupied < w->capacity)
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
                final_pref_ward_id = preferred_ward_id; /* 手动输入优先级最高 */
            else if (auto_pref_ward_id)
                final_pref_ward_id = auto_pref_ward_id; /* 自动按科室优先 */
            else
                final_pref_ward_id = NULL; /* 最后全院自动 */

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

            if (admit_patient(&h_head, w_head, b_head, visit->visit_id, reg->p_id, final_pref_ward_id))
            {
                Hospitalization *new_h = find_hospitalization_by_v_id(h_head, visit->visit_id);
                Bed *new_bed = new_h ? find_bed_by_b_id(b_head, new_h->bed_id) : NULL;
                Ward *new_ward = new_h ? find_ward_by_w_id(w_head, new_h->ward_id) : NULL;

                int s1 = (save_hospitalizations_to_file(h_head) == 0);
                int s2 = (save_beds_to_file(b_head) == 0);
                int s3 = (save_wards_to_file(w_head) == 0);

                if (s1 && s2 && s3)
                    printf("添加住院记录成功！\n");
                else
                {
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
                    if (new_bed)
                        new_bed->status = BED_STATUS_FREE;
                    if (new_ward && new_ward->occupied > 0)
                        new_ward->occupied -= 1;

                    int rb1 = (save_hospitalizations_to_file(h_head) == 0);
                    int rb2 = (save_beds_to_file(b_head) == 0);
                    int rb3 = (save_wards_to_file(w_head) == 0);

                    printf("添加住院记录失败：保存文件失败。");
                    if (rb1 && rb2 && rb3)
                        printf("已回滚。\n");
                    else
                        printf("且回滚失败，请立即检查数据文件。\n");
                }
            }
            else
            {
                printf("添加住院记录失败！可能原因：患者已住院中或无空床位。\n");
            }
        }
    }
    free_hospitalizations(h_head);
    free_wards(w_head);
    free_beds(b_head);
    free_registrations(r_head);
    free_visits(v_head);
    free_patients(p_head);
    free_doctors(d_head);
}

/* 删除住院记录 */
void delete_hospitalization_record()
{
    char hosp_id[MAX_ID_LEN];
    printf("请输入要删除的住院ID(输入0返回): ");
    safe_input(hosp_id, sizeof(hosp_id));
    if (strcmp(hosp_id, "0") == 0)
    {
        clear_screen();
        return;
    }

    Hospitalization *h_head = load_hospitalizations_from_file();
    if (!h_head)
    {
        printf("加载住院记录失败！\n");
        wait_enter();
        clear_screen();
        return;
    }

    Hospitalization *current = h_head, *prev = NULL;
    while (current)
    {
        if (strcmp(current->hosp_id, hosp_id) == 0)
        {
            int h_w, v_w, p_w, ward_w, bed_w, in_w, out_w, st_w;
            Registration *r_head = load_registrations_from_file();
            Visit *v_head = load_visits_from_file();
            Patient *p_head = load_patients_from_file();
            Doctor *d_head = load_doctors_from_file();
            Ward *w_head = load_wards_from_file();
            Bed *b_head = load_beds_from_file();

            calc_hospitalization_width(h_head, v_head, r_head, p_head, w_head, b_head, &h_w, &v_w, &p_w, &ward_w,
                                       &bed_w, &in_w, &out_w, &st_w);

            printf("找到住院记录，信息如下：\n");
            print_hospitalization_header(h_w, v_w, p_w, ward_w, bed_w, in_w, out_w, st_w);
            print_hospitalization(current, v_head, r_head, p_head, w_head, b_head, h_w, v_w, p_w, ward_w, bed_w, in_w,
                                  out_w, st_w);
            print_hospitalization_line(h_w, v_w, p_w, ward_w, bed_w, in_w, out_w, st_w);

            char confirm[MAX_INPUT_LEN];
            printf("确认删除该住院记录？(y/n): ");
            safe_input(confirm, sizeof(confirm));

            if (strcmp(confirm, "n") == 0 || strcmp(confirm, "N") == 0)
            {
                printf("已取消删除。\n");
                free_hospitalizations(h_head);
                free_registrations(r_head);
                free_visits(v_head);
                free_patients(p_head);
                free_doctors(d_head);
                free_wards(w_head);
                free_beds(b_head);
                wait_enter();
                clear_screen();
                return;
            }
            else if (strcmp(confirm, "y") != 0 && strcmp(confirm, "Y") != 0)
            {
                printf("输入无效，已取消删除。\n");
                free_hospitalizations(h_head);
                free_registrations(r_head);
                free_visits(v_head);
                free_patients(p_head);
                free_doctors(d_head);
                free_wards(w_head);
                free_beds(b_head);
                wait_enter();
                clear_screen();
                return;
            }

            /** 先保存待回收的床位/病房ID 以及住院状态 */
            char del_bed_id[MAX_ID_LEN] = {0};
            char del_ward_id[MAX_ID_LEN] = {0};
            int record_status = current->status;     // 缓存状态，防止 free 后失效
            Hospitalization *deleted_node = current; // 延迟释放的住院节点，提交失败时可直接挂回链表
            Bed *bed_to_restore = NULL;              // 需要回滚的关联床位资源
            Ward *ward_to_restore = NULL;            // 需要回滚的关联病房资源
            int old_bed_status = -1;
            int old_ward_occupied = -1;

            strncpy(del_bed_id, current->bed_id, sizeof(del_bed_id) - 1);
            strncpy(del_ward_id, current->ward_id, sizeof(del_ward_id) - 1);

            /** 从住院链表删除 */
            if (prev)
                prev->next = current->next;
            else
                h_head = current->next;
            /** 先断链但不 free，等保存成功后再释放 */
            deleted_node->next = NULL;

            /** 仅当患者仍在住院中时，才回收床位和回退病房占用 */
            if (record_status == 0)
            {
                /** 回收床位 */
                if (b_head)
                {
                    bed_to_restore = find_bed_by_b_id(b_head, del_bed_id);
                    if (bed_to_restore)
                    {
                        old_bed_status = bed_to_restore->status;
                        bed_to_restore->status = 0;
                    }
                }

                /** 回退病房占用 */
                if (w_head)
                {
                    ward_to_restore = find_ward_by_w_id(w_head, del_ward_id);
                    if (ward_to_restore)
                    {
                        old_ward_occupied = ward_to_restore->occupied;
                        if (ward_to_restore->occupied > 0)
                            ward_to_restore->occupied--;
                    }
                }
            }

            /** 三份文件都成功才算删除提交 */
            int s1 = (save_hospitalizations_to_file(h_head) == 0);
            int s2 = (b_head ? (save_beds_to_file(b_head) == 0) : 0);
            int s3 = (w_head ? (save_wards_to_file(w_head) == 0) : 0);

            if (s1 && s2 && s3)
            {
                free(deleted_node);
                printf("删除成功！\n");
            }
            else
            {
                /** 回滚：恢复住院节点到原位置 */
                if (prev)
                {
                    deleted_node->next = prev->next;
                    prev->next = deleted_node;
                }
                else
                {
                    deleted_node->next = h_head;
                    h_head = deleted_node;
                }

                if (bed_to_restore && old_bed_status >= 0)
                    bed_to_restore->status = old_bed_status;
                if (ward_to_restore && old_ward_occupied >= 0)
                    ward_to_restore->occupied = old_ward_occupied;

                /** 回滚后重新持久化 */
                int rb1 = (save_hospitalizations_to_file(h_head) == 0);
                int rb2 = (b_head ? (save_beds_to_file(b_head) == 0) : 0);
                int rb3 = (w_head ? (save_wards_to_file(w_head) == 0) : 0);
                printf("删除失败：保存文件失败。");
                if (rb1 && rb2 && rb3)
                    printf("已回滚。\n");
                else
                    printf("且回滚失败，请立即检查数据文件。\n");
            }

            free_hospitalizations(h_head);
            free_registrations(r_head);
            free_visits(v_head);
            free_patients(p_head);
            free_doctors(d_head);
            free_wards(w_head);
            free_beds(b_head);
            wait_enter();
            clear_screen();
            return;
        }

        prev = current;
        current = current->next;
    }

    printf("未找到该住院ID！\n");
    free_hospitalizations(h_head);
    wait_enter();
    clear_screen();
}

/* 更新住院记录 */
void update_hospitalization_record(void)
{
    char hosp_id[MAX_ID_LEN];
    char buf[MAX_INPUT_LEN];
    int select;

    Hospitalization *h_head = load_hospitalizations_from_file();
    Hospitalization *current = NULL;

    if (!h_head)
    {
        printf("暂无住院记录可更新！\n");
        wait_enter();
        clear_screen();
        return;
    }

    printf("请输入要更新的住院ID(输入0返回): ");
    safe_input(hosp_id, sizeof(hosp_id));

    if (strcmp(hosp_id, "0") == 0)
    {
        clear_screen();
        free_hospitalizations(h_head);
        return;
    }

    if (hosp_id[0] == '\0')
    {
        printf("输入错误！住院ID不能为空。\n");
        wait_enter();
        clear_screen();
        free_hospitalizations(h_head);
        return;
    }

    for (current = h_head; current; current = current->next)
    {
        if (strcmp(current->hosp_id, hosp_id) != 0)
            continue;

        /* 命中目标住院记录后，加载关联数据 */
        Registration *r_head = load_registrations_from_file();
        Visit *v_head = load_visits_from_file();
        Patient *p_head = load_patients_from_file();
        Ward *w_head = load_wards_from_file();
        Bed *b_head = load_beds_from_file();

        int done = 0; // 0:继续菜单 1:退出函数

        while (!done)
        {
            int h_w, v_w, p_w, ward_w, bed_w, in_w, out_w, st_w;

            clear_screen();
            calc_hospitalization_width(h_head, v_head, r_head, p_head, w_head, b_head, &h_w, &v_w, &p_w, &ward_w,
                                       &bed_w, &in_w, &out_w, &st_w);

            printf("找到住院记录，信息如下：\n");
            print_hospitalization_header(h_w, v_w, p_w, ward_w, bed_w, in_w, out_w, st_w);
            print_hospitalization(current, v_head, r_head, p_head, w_head, b_head, h_w, v_w, p_w, ward_w, bed_w, in_w,
                                  out_w, st_w);
            print_hospitalization_line(h_w, v_w, p_w, ward_w, bed_w, in_w, out_w, st_w);

            printf("1. 病房\n");
            printf("2. 床位\n");
            printf("0. 返回\n");
            printf("请选择要更新的内容：");
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
                while (1)
                {
                    int id_w, name_w, type_w, dept_w, cap_w, occ_w;
                    char new_ward_id[MAX_ID_LEN];

                    clear_screen();
                    print_hospitalization_header(h_w, v_w, p_w, ward_w, bed_w, in_w, out_w, st_w);
                    print_hospitalization(current, v_head, r_head, p_head, w_head, b_head, h_w, v_w, p_w, ward_w, bed_w,
                                          in_w, out_w, st_w);
                    print_hospitalization_line(h_w, v_w, p_w, ward_w, bed_w, in_w, out_w, st_w);

                    calc_ward_width(w_head, &id_w, &name_w, &type_w, &dept_w, &cap_w, &occ_w);
                    print_ward_header(id_w, name_w, type_w, dept_w, cap_w, occ_w);
                    for (Ward *w = w_head; w; w = w->next)
                        print_ward(w, id_w, name_w, type_w, dept_w, cap_w, occ_w);
                    print_ward_line(id_w, name_w, type_w, dept_w, cap_w, occ_w);

                    printf("请输入要更新的病房ID(输入0返回): ");
                    safe_input(new_ward_id, sizeof(new_ward_id));

                    if (strcmp(new_ward_id, "0") == 0)
                    {
                        clear_screen();
                        break;
                    }

                    if (new_ward_id[0] == '\0')
                    {
                        printf("输入错误！病房ID不能为空。\n");
                        wait_enter();
                        continue;
                    }

                    Ward *new_ward = find_ward_by_w_id(w_head, new_ward_id);
                    if (!new_ward)
                    {
                        printf("病房ID不存在！\n");
                        wait_enter();
                        continue;
                    }

                    if (strcmp(current->ward_id, new_ward_id) == 0)
                    {
                        printf("新病房与当前病房相同，无需更新。\n");
                        wait_enter();
                        continue;
                    }

                    /* 先找新病房空床，保证可以自动分配 */
                    Bed *auto_bed = find_first_free_bed_in_ward(b_head, new_ward_id);
                    if (!auto_bed)
                    {
                        printf("该病房无空闲床位，无法切换病房！\n");
                        wait_enter();
                        continue;
                    }

                    char old_ward_id[MAX_ID_LEN] = {0}; // 快照当前住院位置，失败时可完整恢复
                    char old_bed_id[MAX_ID_LEN] = {0};
                    strncpy(old_ward_id, current->ward_id, sizeof(old_ward_id) - 1);
                    strncpy(old_bed_id, current->bed_id, sizeof(old_bed_id) - 1);

                    /** 释放原床位 */
                    Bed *old_bed = find_bed_by_b_id(b_head, current->bed_id);
                    int old_old_bed_status = old_bed ? old_bed->status : -1;
                    int old_auto_bed_status = auto_bed->status;
                    if (old_bed)
                        old_bed->status = 0;

                    /** 占用新床位 */
                    auto_bed->status = 1;

                    /** 调整病房占用计数 */
                    Ward *old_ward = find_ward_by_w_id(w_head, current->ward_id);
                    int old_old_ward_occupied = old_ward ? old_ward->occupied : -1;
                    int old_new_ward_occupied = new_ward->occupied;
                    if (old_ward && old_ward->occupied > 0)
                        old_ward->occupied -= 1;
                    new_ward->occupied += 1;

                    /** 更新住院记录的病房和床位 */
                    strncpy(current->ward_id, new_ward_id, sizeof(current->ward_id) - 1);
                    current->ward_id[sizeof(current->ward_id) - 1] = '\0';

                    strncpy(current->bed_id, auto_bed->bed_id, sizeof(current->bed_id) - 1);
                    current->bed_id[sizeof(current->bed_id) - 1] = '\0';

                    /** 新住院位置落盘：住院表+病房表+床位表 */
                    int s1 = (save_hospitalizations_to_file(h_head) == 0);
                    int s2 = (save_wards_to_file(w_head) == 0);
                    int s3 = (save_beds_to_file(b_head) == 0);
                    if (s1 && s2 && s3)
                    {
                        printf("病房更新成功！原床位已释放，已自动分配新床位：%s\n", auto_bed->bed_id);
                    }
                    else
                    {
                        if (old_bed && old_old_bed_status >= 0)
                            old_bed->status = old_old_bed_status;
                        auto_bed->status = old_auto_bed_status;
                        if (old_ward && old_old_ward_occupied >= 0)
                            old_ward->occupied = old_old_ward_occupied;
                        new_ward->occupied = old_new_ward_occupied;

                        strncpy(current->ward_id, old_ward_id, sizeof(current->ward_id) - 1);
                        current->ward_id[sizeof(current->ward_id) - 1] = '\0';
                        strncpy(current->bed_id, old_bed_id, sizeof(current->bed_id) - 1);
                        current->bed_id[sizeof(current->bed_id) - 1] = '\0';

                        /** 回滚后的原状态重新落盘 */
                        int rb1 = (save_hospitalizations_to_file(h_head) == 0);
                        int rb2 = (save_wards_to_file(w_head) == 0);
                        int rb3 = (save_beds_to_file(b_head) == 0);
                        printf("病房更新失败：保存文件失败。");
                        if (rb1 && rb2 && rb3)
                            printf("已回滚。\n");
                        else
                            printf("且回滚失败，请立即检查数据文件。\n");
                    }

                    wait_enter();
                    clear_screen();
                    break;
                }

                continue;
            }

            if (select == 2)
            {
                /* 仅允许当前病房空闲床位 */
                int page_size = PAGE_SIZE;
                int current_page = 1;
                int id_w, w_w, b_w, status_w;

                while (1)
                {
                    int free_total = 0;
                    int start_index, end_index;
                    int printed = 0;
                    int seen_free = 0;
                    char new_bed_id[MAX_ID_LEN];

                    for (Bed *b = b_head; b; b = b->next)
                    {
                        if (b->status == 0 && strcmp(b->ward_id, current->ward_id) == 0)
                            free_total++;
                    }

                    if (free_total <= 0)
                    {
                        printf("当前病房无空闲床位！\n");
                        wait_enter();
                        clear_screen();
                        break;
                    }

                    int total_pages = (free_total + page_size - 1) / page_size;
                    if (current_page > total_pages)
                        current_page = total_pages;
                    if (current_page < 1)
                        current_page = 1;

                    start_index = (current_page - 1) * page_size;
                    end_index = start_index + page_size;
                    if (end_index > free_total)
                        end_index = free_total;

                    clear_screen();
                    calc_bed_width(b_head, w_head, &id_w, &w_w, &b_w, &status_w);

                    print_hospitalization_header(h_w, v_w, p_w, ward_w, bed_w, in_w, out_w, st_w);
                    print_hospitalization(current, v_head, r_head, p_head, w_head, b_head, h_w, v_w, p_w, ward_w, bed_w,
                                          in_w, out_w, st_w);
                    print_hospitalization_line(h_w, v_w, p_w, ward_w, bed_w, in_w, out_w, st_w);

                    printf("===== 当前病房空闲床位列表 (第 %d/%d 页) =====\n", current_page, total_pages);
                    print_bed_header(id_w, w_w, b_w, status_w);

                    for (Bed *b = b_head; b; b = b->next)
                    {
                        if (b->status == 0 && strcmp(b->ward_id, current->ward_id) == 0)
                        {
                            if (seen_free >= start_index && seen_free < end_index)
                            {
                                print_bed(b, w_head, id_w, w_w, b_w, status_w);
                                printed++;
                            }
                            seen_free++;
                            if (printed >= page_size)
                                break;
                        }
                    }

                    print_bed_line(id_w, w_w, b_w, status_w);

                    printf("\n[n]下一页  [p]上一页  [q]退出\n");
                    printf("请输入新的床位ID(n,p,q=操作): ");
                    safe_input(new_bed_id, sizeof(new_bed_id));

                    if (strcmp(new_bed_id, "n") == 0 || strcmp(new_bed_id, "N") == 0)
                    {
                        if (current_page < total_pages)
                            current_page++;
                        continue;
                    }
                    else if (strcmp(new_bed_id, "p") == 0 || strcmp(new_bed_id, "P") == 0)
                    {
                        if (current_page > 1)
                            current_page--;
                        continue;
                    }
                    else if (strcmp(new_bed_id, "q") == 0 || strcmp(new_bed_id, "Q") == 0)
                    {
                        clear_screen();
                        break;
                    }

                    if (new_bed_id[0] == '\0')
                    {
                        printf("输入错误！床位ID不能为空。\n");
                        wait_enter();
                        clear_screen();
                        continue;
                    }

                    Bed *new_bed = find_bed_by_b_id(b_head, new_bed_id);
                    if (!new_bed)
                    {
                        printf("床位ID不存在！\n");
                        wait_enter();
                        clear_screen();
                        continue;
                    }
                    if (new_bed->status == 1)
                    {
                        printf("该床位已被占用！\n");
                        wait_enter();
                        clear_screen();
                        continue;
                    }
                    if (strcmp(new_bed->ward_id, current->ward_id) != 0)
                    {
                        printf("该床位不属于当前病房，不能分配！\n");
                        wait_enter();
                        clear_screen();
                        continue;
                    }

                    char old_bed_id[MAX_ID_LEN] = {0}; // 快照原床位，保存失败时恢复
                    strncpy(old_bed_id, current->bed_id, sizeof(old_bed_id) - 1);

                    Bed *old_bed = find_bed_by_b_id(b_head, current->bed_id);
                    int old_old_bed_status = old_bed ? old_bed->status : -1;
                    int old_new_bed_status = new_bed->status;

                    new_bed->status = 1;
                    if (old_bed && old_bed != new_bed)
                        old_bed->status = 0;

                    strncpy(current->bed_id, new_bed_id, sizeof(current->bed_id) - 1);
                    current->bed_id[sizeof(current->bed_id) - 1] = '\0';

                    /** 床位调整落盘：住院表+床位表 */
                    int s1 = (save_hospitalizations_to_file(h_head) == 0);
                    int s2 = (save_beds_to_file(b_head) == 0);
                    if (s1 && s2)
                        printf("床位更新成功！\n");
                    else
                    {
                        new_bed->status = old_new_bed_status;
                        if (old_bed && old_bed != new_bed && old_old_bed_status >= 0)
                            old_bed->status = old_old_bed_status;

                        strncpy(current->bed_id, old_bed_id, sizeof(current->bed_id) - 1);
                        current->bed_id[sizeof(current->bed_id) - 1] = '\0';

                        /** 回滚后重写文件，尽量回到一致状态 */
                        int rb1 = (save_hospitalizations_to_file(h_head) == 0);
                        int rb2 = (save_beds_to_file(b_head) == 0);
                        printf("床位更新失败：保存文件失败。");
                        if (rb1 && rb2)
                            printf("已回滚。\n");
                        else
                            printf("且回滚失败，请立即检查数据文件。\n");
                    }

                    wait_enter();
                    clear_screen();
                }

                continue;
            }
        }

        free_registrations(r_head);
        free_visits(v_head);
        free_patients(p_head);
        free_wards(w_head);
        free_beds(b_head);

        free_hospitalizations(h_head);
        return;
    }

    printf("未找到指定ID的记录！\n");
    wait_enter();
    clear_screen();
    free_hospitalizations(h_head);
}

/* 查询住院记录 */
void query_hospitalization_record(void)
{
    Hospitalization *h_head = load_hospitalizations_from_file();
    Visit *v_head = load_visits_from_file();
    Registration *r_head = load_registrations_from_file();
    Patient *p_head = load_patients_from_file();
    Ward *w_head = load_wards_from_file();
    Bed *b_head = load_beds_from_file();

    char buf[MAX_INPUT_LEN];
    int select;

    if (!h_head)
    {
        printf("暂无住院记录！\n");
        wait_enter();
        clear_screen();
        free_visits(v_head);
        free_registrations(r_head);
        free_patients(p_head);
        free_wards(w_head);
        free_beds(b_head);
        return;
    }

    while (1)
    {
        clear_screen();
        printf("===== 查询住院记录 =====\n");
        printf("1. 按住院ID查询\n");
        printf("2. 按看诊ID查询\n");
        printf("3. 按患者ID查询\n");
        printf("4. 按患者姓名模糊查询\n");
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

        if (select == 0)
        {
            clear_screen();
            break;
        }

        if (select == 1)
        {
            char hosp_id[MAX_ID_LEN];
            Hospitalization *h = NULL;

            printf("请输入住院ID(输入0返回): ");
            safe_input(hosp_id, sizeof(hosp_id));

            if (strcmp(hosp_id, "0") == 0)
            {
                clear_screen();
                continue;
            }
            if (hosp_id[0] == '\0')
            {
                printf("输入错误！住院ID不能为空。\n");
                wait_enter();
                clear_screen();
                continue;
            }

            h = find_hospitalization_by_h_id(h_head, hosp_id);

            if (h)
            {
                int h_w, v_w, p_w, ward_w, bed_w, in_w, out_w, st_w;
                calc_hospitalization_width(h_head, v_head, r_head, p_head, w_head, b_head, &h_w, &v_w, &p_w, &ward_w,
                                           &bed_w, &in_w, &out_w, &st_w);

                clear_screen();
                printf("查询结果：\n");
                print_hospitalization_header(h_w, v_w, p_w, ward_w, bed_w, in_w, out_w, st_w);
                print_hospitalization(h, v_head, r_head, p_head, w_head, b_head, h_w, v_w, p_w, ward_w, bed_w, in_w,
                                      out_w, st_w);
                print_hospitalization_line(h_w, v_w, p_w, ward_w, bed_w, in_w, out_w, st_w);
            }
            else
            {
                printf("未找到该住院ID的记录！\n");
            }

            wait_enter();
            clear_screen();
            continue;
        }

        if (select == 2)
        {
            char visit_id[MAX_ID_LEN];
            Hospitalization *h = NULL;

            printf("请输入看诊ID(输入0返回): ");
            safe_input(visit_id, sizeof(visit_id));

            if (strcmp(visit_id, "0") == 0)
            {
                clear_screen();
                continue;
            }
            if (visit_id[0] == '\0')
            {
                printf("输入错误！看诊ID不能为空。\n");
                wait_enter();
                clear_screen();
                continue;
            }

            h = find_hospitalization_by_v_id(h_head, visit_id);

            if (h)
            {
                int h_w, v_w, p_w, ward_w, bed_w, in_w, out_w, st_w;
                calc_hospitalization_width(h_head, v_head, r_head, p_head, w_head, b_head, &h_w, &v_w, &p_w, &ward_w,
                                           &bed_w, &in_w, &out_w, &st_w);

                clear_screen();
                printf("查询结果：\n");
                print_hospitalization_header(h_w, v_w, p_w, ward_w, bed_w, in_w, out_w, st_w);
                print_hospitalization(h, v_head, r_head, p_head, w_head, b_head, h_w, v_w, p_w, ward_w, bed_w, in_w,
                                      out_w, st_w);
                print_hospitalization_line(h_w, v_w, p_w, ward_w, bed_w, in_w, out_w, st_w);
            }
            else
            {
                printf("未找到该看诊ID的记录！\n");
            }

            wait_enter();
            clear_screen();
            continue;
        }

        if (select == 3)
        {
            char patient_id[MAX_ID_LEN];
            Hospitalization *h_cur = NULL;
            int found = 0;

            printf("请输入患者ID(输入0返回): ");
            safe_input(patient_id, sizeof(patient_id));

            if (strcmp(patient_id, "0") == 0)
            {
                clear_screen();
                continue;
            }
            if (patient_id[0] == '\0')
            {
                printf("输入错误！患者ID不能为空。\n");
                wait_enter();
                clear_screen();
                continue;
            }

            {
                int h_w, v_w, p_w, ward_w, bed_w, in_w, out_w, st_w;
                calc_hospitalization_width(h_head, v_head, r_head, p_head, w_head, b_head, &h_w, &v_w, &p_w, &ward_w,
                                           &bed_w, &in_w, &out_w, &st_w);

                clear_screen();
                printf("查询结果：\n");
                print_hospitalization_header(h_w, v_w, p_w, ward_w, bed_w, in_w, out_w, st_w);

                h_cur = find_hospitalization_by_p_id(h_head, patient_id);
                while (h_cur)
                {
                    print_hospitalization(h_cur, v_head, r_head, p_head, w_head, b_head, h_w, v_w, p_w, ward_w, bed_w,
                                          in_w, out_w, st_w);
                    found = 1;
                    h_cur = find_hospitalization_by_p_id(h_cur->next, patient_id);
                }

                print_hospitalization_line(h_w, v_w, p_w, ward_w, bed_w, in_w, out_w, st_w);

                if (!found)
                    printf("未找到该患者ID的住院记录！\n");
            }

            wait_enter();
            clear_screen();
            continue;
        }

        if (select == 4) /* 按患者姓名模糊查询住院记录 */
        {
            char name_query[MAX_INPUT_LEN];
            int found = 0;

            printf("请输入患者姓名关键字(输入0返回): ");
            safe_input(name_query, sizeof(name_query));

            if (strcmp(name_query, "0") == 0)
            {
                clear_screen();
                continue;
            }
            if (name_query[0] == '\0')
            {
                printf("输入错误！姓名不能为空。\n");
                wait_enter();
                clear_screen();
                continue;
            }

            int h_w, v_w, p_w, ward_w, bed_w, in_w, out_w, st_w;
            calc_hospitalization_width(h_head, v_head, r_head, p_head, w_head, b_head, &h_w, &v_w, &p_w, &ward_w,
                                       &bed_w, &in_w, &out_w, &st_w);

            clear_screen();
            printf("查询结果：\n");
            print_hospitalization_header(h_w, v_w, p_w, ward_w, bed_w, in_w, out_w, st_w);

            for (Hospitalization *h = h_head; h; h = h->next)
            {
                Patient *pat = find_patient_by_p_id(p_head, h->p_id);
                if (pat && strstr(pat->name, name_query) != NULL)
                {
                    print_hospitalization(h, v_head, r_head, p_head, w_head, b_head, h_w, v_w, p_w, ward_w, bed_w, in_w,
                                          out_w, st_w);
                    found = 1;
                }
            }

            print_hospitalization_line(h_w, v_w, p_w, ward_w, bed_w, in_w, out_w, st_w);

            if (!found)
                printf("未找到匹配的住院记录！\n");

            wait_enter();
            clear_screen();
            continue;
        }

        printf("未知错误: %d\n", select);
        wait_enter();
        clear_screen();
    }

    free_hospitalizations(h_head);
    free_visits(v_head);
    free_registrations(r_head);
    free_patients(p_head);
    free_wards(w_head);
    free_beds(b_head);
}

/* 显示所有住院记录 */
void show_all_hospitalization_records()
{
    Hospitalization *h_head = load_hospitalizations_from_file();
    if (!h_head)
    {
        printf("暂无住院记录！\n");
        wait_enter();
        clear_screen();
        return;
    }
    Visit *v_head = load_visits_from_file();
    if (!v_head)
    {
        printf("加载看诊记录失败！\n");
        free_hospitalizations(h_head);
        wait_enter();
        clear_screen();
        return;
    }
    Registration *r_head = load_registrations_from_file();
    if (!r_head)
    {
        printf("加载挂号记录失败！\n");
        free_hospitalizations(h_head);
        free_visits(v_head);
        wait_enter();
        clear_screen();
        return;
    }
    Patient *p_head = load_patients_from_file();
    if (!p_head)
    {
        printf("加载患者记录失败！\n");
        free_hospitalizations(h_head);
        free_visits(v_head);
        free_registrations(r_head);
        wait_enter();
        clear_screen();
        return;
    }
    Ward *w_head = load_wards_from_file();
    if (!w_head)
    {
        printf("加载病房记录失败！\n");
        free_hospitalizations(h_head);
        free_visits(v_head);
        free_registrations(r_head);
        free_patients(p_head);
        wait_enter();
        clear_screen();
        return;
    }
    Bed *b_head = load_beds_from_file();
    if (!b_head)
    {
        printf("加载床位记录失败！\n");
        free_hospitalizations(h_head);
        free_visits(v_head);
        free_registrations(r_head);
        free_patients(p_head);
        free_wards(w_head);
        wait_enter();
        clear_screen();
        return;
    }

    int page_size = PAGE_SIZE;
    int total = count_hospitalizations(h_head);
    int total_pages = (total + page_size - 1) / page_size;

    int current_page = 1;

    int h_w, v_w, p_w, ward_w, bed_w, in_w, out_w, st_w;
    calc_hospitalization_width(h_head, v_head, r_head, p_head, w_head, b_head, &h_w, &v_w, &p_w, &ward_w, &bed_w, &in_w,
                               &out_w, &st_w);

    while (1)
    {
        clear_screen();

        printf("===== 所有住院记录 (第 %d/%d 页) =====\n", current_page, total_pages);

        print_hospitalization_header(h_w, v_w, p_w, ward_w, bed_w, in_w, out_w, st_w);

        int start = (current_page - 1) * page_size;
        Hospitalization *h_cur = get_nth_hospitalization(h_head, start);

        for (int i = 0; i < page_size && h_cur; i++)
        {
            print_hospitalization(h_cur, v_head, r_head, p_head, w_head, b_head, h_w, v_w, p_w, ward_w, bed_w, in_w,
                                  out_w, st_w);
            h_cur = h_cur->next;
        }

        print_hospitalization_line(h_w, v_w, p_w, ward_w, bed_w, in_w, out_w, st_w);

        printf("\n[n]下一页  [p]上一页  [q]退出\n");

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
    free_hospitalizations(h_head);
    free_visits(v_head);
    free_registrations(r_head);
    free_patients(p_head);
    free_wards(w_head);
    free_beds(b_head);
    clear_screen();
}
