/*
 * 数据分析模块
 */
#include "model/analytics.h"
#include "core/utils.h"
#include "model/hospitalization.h"
#include "model/ward.h"

/*
 * 数据分析工具函数
 */

/* 打印分析结果并对齐 */
static void print_analytics_align(const char *s, int width)
{
    int w = str_width(s);
    printf("%s", s);

    for (int i = 0; i < width - w; i++)
        putchar(' ');
}

/* 打印分析表格分隔线 */
static void print_analytics_line(int cols, const int *widths)
{
    printf("+");
    for (int i = 0; i < cols; i++)
    {
        for (int j = 0; j < widths[i] + 2; j++)
            printf("-");
        printf("+");
    }
    printf("\n");
}

/*
 * =============== 病房利用率分析相关结构体和函数 ===============
 */

/* 病房利用率统计结构体 */
typedef struct WardUtilStats
{
    char ward_id[MAX_ID_LEN];
    char name[MAX_NAME_LEN];
    int capacity;
    int occupied;
    float utilization_rate;
} WardUtilStats;

/* 病房周转率统计结构体 */
typedef struct WardTurnoverStats
{
    char ward_id[MAX_ID_LEN];
    char name[MAX_NAME_LEN];
    int capacity;
    int discharged;
    float turnover_rate;
} WardTurnoverStats;

/* 空床率统计结构体（用于排名） */
typedef struct WardEmptyStats
{
    char name[MAX_NAME_LEN];
    int capacity;
    int occupied;
    float empty_rate;
} WardEmptyStats;

/* 住院天数统计结构体 */
typedef struct WardStayStats
{
    char ward_id[MAX_ID_LEN];
    char name[MAX_NAME_LEN];
    int total_days;
    int discharged_count;
    float avg_days;
} WardStayStats;

/* 计算病房利用率表格列宽 */
static void calc_ward_utilization_width(Ward *w_head, int *name_w, int *cap_w, int *occ_w, int *rate_w, int *bar_w)
{
    *name_w = str_width("病房名称");
    *cap_w = str_width("床位");
    *occ_w = str_width("占用");
    *rate_w = str_width("利用率");
    *bar_w = str_width("利用率分布");

    /* 进度条固定宽度14格, 显示宽度 = 1([) + 14 + 1(]) = 16 */
    int bar_display_w = 16;
    if (bar_display_w > *bar_w)
        *bar_w = bar_display_w;

    /* 遍历计算最大列宽 */
    for (Ward *w = w_head; w; w = w->next)
    {
        int nw = str_width(w->name);
        if (nw > *name_w)
            *name_w = nw;

        char tmp[32];
        snprintf(tmp, sizeof(tmp), "%d", w->capacity);
        int cw = str_width(tmp);
        if (cw > *cap_w)
            *cap_w = cw;

        snprintf(tmp, sizeof(tmp), "%d", w->occupied);
        int ow = str_width(tmp);
        if (ow > *occ_w)
            *occ_w = ow;
    }

    /* 利用率列宽至少容纳 "100.0%" */
    if (*rate_w < 6)
        *rate_w = 6;
}

/* 打印病房利用率表头 */
static void print_ward_utilization_header(int name_w, int cap_w, int occ_w, int rate_w, int bar_w)
{
    int widths[] = {name_w, cap_w, occ_w, rate_w, bar_w};
    print_analytics_line(5, widths);

    printf("| ");
    print_analytics_align("病房名称", name_w);
    printf(" | ");
    print_analytics_align("床位", cap_w);
    printf(" | ");
    print_analytics_align("占用", occ_w);
    printf(" | ");
    print_analytics_align("利用率", rate_w);
    printf(" | ");
    print_analytics_align("利用率分布", bar_w);
    printf(" |\n");

    print_analytics_line(5, widths);
}

/* 打印病房利用率数据行 */
static void print_ward_utilization_line(Ward *w, int name_w, int cap_w, int occ_w, int rate_w, int bar_w)
{
    float ratio = (w->capacity > 0) ? (float)w->occupied / w->capacity : 0.0f;

    char cap_str[32], occ_str[32], rate_str[32];
    snprintf(cap_str, sizeof(cap_str), "%d", w->capacity);
    snprintf(occ_str, sizeof(occ_str), "%d", w->occupied);
    snprintf(rate_str, sizeof(rate_str), "%.1f%%", ratio * 100.0f);

    char bar_buf[256];
    render_bar(ratio, 14, bar_buf);

    printf("| ");
    print_analytics_align(w->name, name_w);
    printf(" | ");
    print_analytics_align(cap_str, cap_w);
    printf(" | ");
    print_analytics_align(occ_str, occ_w);
    printf(" | ");
    print_analytics_align(rate_str, rate_w);
    printf(" | ");
    print_analytics_align(bar_buf, bar_w);
    printf(" |\n");
}

/* 打印病房利用率分隔线 */
static void print_ward_utilization_line_separator(int name_w, int cap_w, int occ_w, int rate_w, int bar_w)
{
    int widths[] = {name_w, cap_w, occ_w, rate_w, bar_w};
    print_analytics_line(5, widths);
}

/* 计算病房周转率表格列宽 */
static void calc_ward_turnover_width(WardTurnoverStats *stats, int count, int *name_w, int *dis_w, int *turn_w)
{
    *name_w = str_width("病房名称");
    *dis_w = str_width("出院人次");
    *turn_w = str_width("周转率");

    for (int i = 0; i < count; i++)
    {
        int nw = str_width(stats[i].name);
        if (nw > *name_w)
            *name_w = nw;

        char tmp[32];
        snprintf(tmp, sizeof(tmp), "%d", stats[i].discharged);
        int dw = str_width(tmp);
        if (dw > *dis_w)
            *dis_w = dw;
    }

    /* 周转率格式 "0.00" 最多5字符 */
    if (*turn_w < 6)
        *turn_w = 6;
}

/* 打印病房周转率表头 */
static void print_ward_turnover_header(int name_w, int dis_w, int turn_w)
{
    int widths[] = {name_w, dis_w, turn_w};
    print_analytics_line(3, widths);

    printf("| ");
    print_analytics_align("病房名称", name_w);
    printf(" | ");
    print_analytics_align("出院人次", dis_w);
    printf(" | ");
    print_analytics_align("周转率", turn_w);
    printf(" |\n");

    print_analytics_line(3, widths);
}

/* 打印病房周转率数据行 */
static void print_ward_turnover_line(WardTurnoverStats *stat, int name_w, int dis_w, int turn_w)
{
    char dis_str[32], turn_str[32];
    snprintf(dis_str, sizeof(dis_str), "%d", stat->discharged);
    snprintf(turn_str, sizeof(turn_str), "%.2f", stat->turnover_rate);

    printf("| ");
    print_analytics_align(stat->name, name_w);
    printf(" | ");
    print_analytics_align(dis_str, dis_w);
    printf(" | ");
    print_analytics_align(turn_str, turn_w);
    printf(" |\n");
}

/* 计算空床率排名表格列宽 */
static void calc_ward_empty_width(WardEmptyStats *stats, int count, int *rank_w, int *name_w, int *empty_w, int *rate_w,
                                  int *bar_w)
{
    *rank_w = str_width("排名");
    *name_w = str_width("病房名称");
    *empty_w = str_width("空床数");
    *rate_w = str_width("空床率");
    *bar_w = str_width("分布");

    int bar_display_w = 16;
    if (bar_display_w > *bar_w)
        *bar_w = bar_display_w;

    for (int i = 0; i < count; i++)
    {
        int nw = str_width(stats[i].name);
        if (nw > *name_w)
            *name_w = nw;
    }
}

/* 打印空床率排名表头 */
static void print_ward_empty_header(int rank_w, int name_w, int empty_w, int rate_w, int bar_w)
{
    int widths[] = {rank_w, name_w, empty_w, rate_w, bar_w};
    print_analytics_line(5, widths);

    printf("| ");
    print_analytics_align("排名", rank_w);
    printf(" | ");
    print_analytics_align("病房名称", name_w);
    printf(" | ");
    print_analytics_align("空床数", empty_w);
    printf(" | ");
    print_analytics_align("空床率", rate_w);
    printf(" | ");
    print_analytics_align("分布", bar_w);
    printf(" |\n");

    print_analytics_line(5, widths);
}

/* 打印空床率排名数据行 */
static void print_ward_empty_line(WardEmptyStats *stat, int rank, int rank_w, int name_w, int empty_w, int rate_w,
                                  int bar_w)
{
    int free_beds = stat->capacity - stat->occupied;
    char rank_str[16], empty_str[16], rate_str[16];
    snprintf(rank_str, sizeof(rank_str), "%d", rank);
    snprintf(empty_str, sizeof(empty_str), "%d", free_beds);
    snprintf(rate_str, sizeof(rate_str), "%.1f%%", stat->empty_rate * 100.0f);

    char bar_buf[256];
    render_bar(stat->empty_rate, 14, bar_buf);

    printf("| ");
    print_analytics_align(rank_str, rank_w);
    printf(" | ");
    print_analytics_align(stat->name, name_w);
    printf(" | ");
    print_analytics_align(empty_str, empty_w);
    printf(" | ");
    print_analytics_align(rate_str, rate_w);
    printf(" | ");
    print_analytics_align(bar_buf, bar_w);
    printf(" |\n");
}

/* 打印空床率排名分隔线 */
static void print_ward_empty_line_separator(int rank_w, int name_w, int empty_w, int rate_w, int bar_w)
{
    int widths[] = {rank_w, name_w, empty_w, rate_w, bar_w};
    print_analytics_line(5, widths);
}

/* 空床率降序比较函数 */
static int compare_ward_empty_desc(const void *a, const void *b)
{
    const WardEmptyStats *wa = (const WardEmptyStats *)a;
    const WardEmptyStats *wb = (const WardEmptyStats *)b;
    if (wb->empty_rate > wa->empty_rate)
        return 1;
    if (wb->empty_rate < wa->empty_rate)
        return -1;
    return 0;
}

/* 计算住院天数表格列宽 */
static void calc_ward_stay_width(WardStayStats *stats, int count, int *name_w, int *cnt_w, int *avg_w, int *bar_w)
{
    *name_w = str_width("病房名称");
    *cnt_w = str_width("出院人次");
    *avg_w = str_width("平均天数");
    *bar_w = str_width("分布");

    if (*bar_w < 14)
        *bar_w = 14;

    for (int i = 0; i < count; i++)
    {
        int nw = str_width(stats[i].name);
        if (nw > *name_w)
            *name_w = nw;
    }
}

/* 打印住院天数表头 */
static void print_ward_stay_header(int name_w, int cnt_w, int avg_w, int bar_w)
{
    int widths[] = {name_w, cnt_w, avg_w, bar_w};
    print_analytics_line(4, widths);

    printf("| ");
    print_analytics_align("病房名称", name_w);
    printf(" | ");
    print_analytics_align("出院人次", cnt_w);
    printf(" | ");
    print_analytics_align("平均天数", avg_w);
    printf(" | ");
    print_analytics_align("分布", bar_w);
    printf(" |\n");

    print_analytics_line(4, widths);
}

/* 打印住院天数数据行 */
static void print_ward_stay_line(WardStayStats *stat, int max_avg, int name_w, int cnt_w, int avg_w, int bar_w)
{
    char cnt_str[16], avg_str[16];
    snprintf(cnt_str, sizeof(cnt_str), "%d", stat->discharged_count);
    snprintf(avg_str, sizeof(avg_str), "%.1f", stat->avg_days);

    char hbar_buf[256];
    render_hbar((int)(stat->avg_days + 0.5f), max_avg, 14, hbar_buf);

    printf("| ");
    print_analytics_align(stat->name, name_w);
    printf(" | ");
    print_analytics_align(cnt_str, cnt_w);
    printf(" | ");
    print_analytics_align(avg_str, avg_w);
    printf(" | ");
    print_analytics_align(hbar_buf, bar_w);
    printf(" |\n");
}

/* 打印住院天数分隔线 */
static void print_ward_stay_line_separator(int name_w, int cnt_w, int avg_w, int bar_w)
{
    int widths[] = {name_w, cnt_w, avg_w, bar_w};
    print_analytics_line(4, widths);
}

/* ========== 病房利用率分析 ========== */
void analytics_ward_utilization(void)
{
    clear_screen();

    /* 加载数据 */
    Ward *w_head = load_wards_from_file();
    if (!w_head)
    {
        printf("暂无病房数据！\n");
        wait_enter();
        return;
    }

    Hospitalization *h_head = load_hospitalizations_from_file();

    /* ===== 第一部分: 病房利用率总览 ===== */
    printf("===== 病房利用率总览 =====\n");

    /* 计算列宽 */
    int name_w, cap_w, occ_w, rate_w, bar_w;
    calc_ward_utilization_width(w_head, &name_w, &cap_w, &occ_w, &rate_w, &bar_w);

    /* 打印表头 */
    print_ward_utilization_header(name_w, cap_w, occ_w, rate_w, bar_w);

    /* 计算全院总床位数和占用数 */
    int total_capacity = 0, total_occupied = 0;
    for (Ward *w = w_head; w; w = w->next)
    {
        total_capacity += w->capacity;
        total_occupied += w->occupied;
    }

    /* 打印每行数据 */
    for (Ward *w = w_head; w; w = w->next)
    {
        print_ward_utilization_line(w, name_w, cap_w, occ_w, rate_w, bar_w);
    }

    /* 打印分隔线 */
    print_ward_utilization_line_separator(name_w, cap_w, occ_w, rate_w, bar_w);

    /* 全院综合利用率 */
    float total_ratio = (total_capacity > 0) ? (float)total_occupied / total_capacity : 0.0f;
    char total_bar[256];
    render_bar(total_ratio, 10, total_bar);
    printf("全院综合利用率: %s %.1f%%  (%d/%d)\n\n", total_bar, total_ratio * 100.0f, total_occupied, total_capacity);

    /* ===== 第二部分: 病房周转率统计 ===== */
    printf("===== 病房周转率统计 =====\n");

    /* 统计病房数量 */
    int ward_count = 0;
    for (Ward *w = w_head; w; w = w->next)
        ward_count++;

    /* 分配周转率统计数组 */
    WardTurnoverStats *turnovers = (WardTurnoverStats *)calloc(ward_count, sizeof(WardTurnoverStats));
    if (!turnovers)
    {
        printf("内存分配失败！\n");
        free_wards(w_head);
        if (h_head)
            free_hospitalizations(h_head);
        wait_enter();
        return;
    }

    /* 初始化周转率统计 */
    int idx = 0;
    for (Ward *w = w_head; w; w = w->next, idx++)
    {
        strncpy(turnovers[idx].ward_id, w->ward_id, MAX_ID_LEN - 1);
        strncpy(turnovers[idx].name, w->name, MAX_NAME_LEN - 1);
        turnovers[idx].capacity = w->capacity;
        turnovers[idx].discharged = 0;
        turnovers[idx].turnover_rate = 0.0f;
    }

    /* 遍历住院记录统计出院人次 */
    if (h_head)
    {
        for (Hospitalization *h = h_head; h; h = h->next)
        {
            if (h->status == HOSP_STATUS_DISCHARGED)
            {
                for (int i = 0; i < ward_count; i++)
                {
                    if (strcmp(turnovers[i].ward_id, h->ward_id) == 0)
                    {
                        turnovers[i].discharged++;
                        break;
                    }
                }
            }
        }
    }

    /* 计算周转率 */
    for (int i = 0; i < ward_count; i++)
    {
        turnovers[i].turnover_rate =
            (turnovers[i].capacity > 0) ? (float)turnovers[i].discharged / turnovers[i].capacity : 0.0f;
    }

    /* 计算周转率表格列宽 */
    int t_name_w, t_dis_w, t_turn_w;
    calc_ward_turnover_width(turnovers, ward_count, &t_name_w, &t_dis_w, &t_turn_w);

    /* 打印周转率表头 */
    print_ward_turnover_header(t_name_w, t_dis_w, t_turn_w);

    /* 打印周转率数据行 */
    for (int i = 0; i < ward_count; i++)
    {
        print_ward_turnover_line(&turnovers[i], t_name_w, t_dis_w, t_turn_w);
    }

    /* 打印周转率分隔线 */
    int t_widths[] = {t_name_w, t_dis_w, t_turn_w};
    print_analytics_line(3, t_widths);

    free(turnovers);

    /* ===== 第三部分: 空床率排名 ===== */
    printf("\n===== 空床率排名（从高到低）=====\n");

    /* 构建空床率统计数组 */
    WardEmptyStats *empties = (WardEmptyStats *)calloc(ward_count, sizeof(WardEmptyStats));
    if (!empties)
    {
        printf("内存分配失败！\n");
        free_wards(w_head);
        if (h_head)
            free_hospitalizations(h_head);
        wait_enter();
        return;
    }

    /* 初始化空床率统计 */
    idx = 0;
    for (Ward *w = w_head; w; w = w->next, idx++)
    {
        strncpy(empties[idx].name, w->name, MAX_NAME_LEN - 1);
        empties[idx].capacity = w->capacity;
        empties[idx].occupied = w->occupied;
        empties[idx].empty_rate = (w->capacity > 0) ? (float)(w->capacity - w->occupied) / w->capacity : 0.0f;
    }

    /* 按空床率降序排序 */
    qsort(empties, ward_count, sizeof(WardEmptyStats), compare_ward_empty_desc);

    /* 计算空床率表格列宽 */
    int r_rank_w, r_name_w, r_empty_w, r_rate_w, r_bar_w;
    calc_ward_empty_width(empties, ward_count, &r_rank_w, &r_name_w, &r_empty_w, &r_rate_w, &r_bar_w);

    /* 打印空床率表头 */
    print_ward_empty_header(r_rank_w, r_name_w, r_empty_w, r_rate_w, r_bar_w);

    /* 打印空床率数据行 */
    for (int i = 0; i < ward_count; i++)
    {
        print_ward_empty_line(&empties[i], i + 1, r_rank_w, r_name_w, r_empty_w, r_rate_w, r_bar_w);
    }

    /* 打印空床率分隔线 */
    print_ward_empty_line_separator(r_rank_w, r_name_w, r_empty_w, r_rate_w, r_bar_w);

    free(empties);

    /* ===== 第四部分: 各病房历史平均住院天数 ===== */
    if (h_head)
    {
        printf("\n===== 各病房历史平均住院天数 =====\n");

        /* 分配住院天数统计数组 */
        WardStayStats *stays = (WardStayStats *)calloc(ward_count, sizeof(WardStayStats));
        if (stays)
        {
            /* 初始化住院天数统计 */
            idx = 0;
            for (Ward *w = w_head; w; w = w->next, idx++)
            {
                strncpy(stays[idx].ward_id, w->ward_id, MAX_ID_LEN - 1);
                strncpy(stays[idx].name, w->name, MAX_NAME_LEN - 1);
                stays[idx].total_days = 0;
                stays[idx].discharged_count = 0;
                stays[idx].avg_days = 0.0f;
            }

            /* 统计各病房住院天数 */
            for (Hospitalization *h = h_head; h; h = h->next)
            {
                if (h->status == HOSP_STATUS_DISCHARGED && h->discharge_date > h->admit_date)
                {
                    int days = (int)((h->discharge_date - h->admit_date) / 86400);
                    if (days < 1)
                        days = 1;
                    for (int i = 0; i < ward_count; i++)
                    {
                        if (strcmp(stays[i].ward_id, h->ward_id) == 0)
                        {
                            stays[i].total_days += days;
                            stays[i].discharged_count++;
                            break;
                        }
                    }
                }
            }

            /* 计算平均住院天数 */
            float max_avg = 0.0f;
            for (int i = 0; i < ward_count; i++)
            {
                if (stays[i].discharged_count > 0)
                {
                    stays[i].avg_days = (float)stays[i].total_days / stays[i].discharged_count;
                    if (stays[i].avg_days > max_avg)
                        max_avg = stays[i].avg_days;
                }
            }

            /* 计算住院天数表格列宽 */
            int s_name_w, s_cnt_w, s_avg_w, s_bar_w;
            calc_ward_stay_width(stays, ward_count, &s_name_w, &s_cnt_w, &s_avg_w, &s_bar_w);

            /* 打印住院天数表头 */
            print_ward_stay_header(s_name_w, s_cnt_w, s_avg_w, s_bar_w);

            /* 打印住院天数数据行 */
            int max_int = (int)(max_avg + 0.5f);
            if (max_int < 1)
                max_int = 1;

            for (int i = 0; i < ward_count; i++)
            {
                print_ward_stay_line(&stays[i], max_int, s_name_w, s_cnt_w, s_avg_w, s_bar_w);
            }

            /* 打印住院天数分隔线 */
            print_ward_stay_line_separator(s_name_w, s_cnt_w, s_avg_w, s_bar_w);

            free(stays);
        }
    }

    /* 释放资源 */
    free_wards(w_head);
    if (h_head)
        free_hospitalizations(h_head);
    wait_enter();
}

/*
 * =============== 科室门诊量与趋势分析相关结构体和函数 ===============
 */

/* ========== 科室门诊量与趋势分析 ========== */
void analytics_department_workload(void)
{
    clear_screen();
    printf("===== 科室门诊量与趋势分析 ======\n");
    wait_enter();
}

/*
 * =============== 住院分析与病房优化相关结构体和函数 ===============
 */

/* ========== 住院分析与病房优化 ========== */
void analytics_ward_optimization(void)
{
    clear_screen();
    printf("===== 住院分析与病房优化 ======\n");
    wait_enter();
}

/*
 * =============== 药品使用分析相关结构体和函数 ===============
 */

/* ========== 药品使用分析 ========== */
void analytics_drug_usage(void)
{
    clear_screen();
    printf("===== 药品使用分析 ======\n");
    wait_enter();
}

/*
 * =============== 住院时长分布与预测相关结构体和函数 ===============
 */

/* ========== 住院时长分布与预测 ========== */
void analytics_hospitalization_duration(void)
{
    clear_screen();
    printf("===== 住院时长分布与预测 ======\n");
    wait_enter();
}
