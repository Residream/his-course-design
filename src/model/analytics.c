/*
 * 数据分析模块
 */
#include "model/analytics.h"
#include "core/utils.h"
#include "model/department.h"
#include "model/doctor.h"
#include "model/drug.h"
#include "model/hospitalization.h"
#include "model/patient.h"
#include "model/prescription.h"
#include "model/registration.h"
#include "model/visit.h"
#include "model/ward.h"

/*
 * 数据分析工具函数
 */

/* 打印字符串并用空格右填充到 width 显示宽度, 用于表格单元格对齐 */
static void print_analytics_align(const char *s, int width)
{
    int w = str_width(s);
    printf("%s", s);

    for (int i = 0; i < width - w; i++)
        putchar(' ');
}

/* 打印表格分隔线, 形如 "+-----+------+------+", widths[] 指定每列的内部宽度 */
static void print_analytics_line(int cols, const int *widths)
{
    printf("+");
    for (int i = 0; i < cols; i++)
    {
        /* 每列内部宽度两侧各留一个空格, 因此分隔线长度 = widths[i] + 2 */
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

    /* 进度条固定宽度 BAR_WIDTH 格, 显示宽度 = 1([) + BAR_WIDTH + 1(]) */
    if (ANALYTICS_BAR_DISPLAY_WIDTH > *bar_w)
        *bar_w = ANALYTICS_BAR_DISPLAY_WIDTH;

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
    if (*rate_w < ANALYTICS_RATE_COL_MIN_WIDTH)
        *rate_w = ANALYTICS_RATE_COL_MIN_WIDTH;
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

    char bar_buf[ANALYTICS_BAR_BUF_SIZE];
    render_bar(ratio, ANALYTICS_BAR_WIDTH, bar_buf);

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
    if (*turn_w < ANALYTICS_RATE_COL_MIN_WIDTH)
        *turn_w = ANALYTICS_RATE_COL_MIN_WIDTH;
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

    if (ANALYTICS_BAR_DISPLAY_WIDTH > *bar_w)
        *bar_w = ANALYTICS_BAR_DISPLAY_WIDTH;

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

    char bar_buf[ANALYTICS_BAR_BUF_SIZE];
    render_bar(stat->empty_rate, ANALYTICS_BAR_WIDTH, bar_buf);

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

    if (*bar_w < ANALYTICS_BAR_WIDTH)
        *bar_w = ANALYTICS_BAR_WIDTH;

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

    char hbar_buf[ANALYTICS_BAR_BUF_SIZE];
    render_hbar((int)(stat->avg_days + 0.5f), max_avg, ANALYTICS_BAR_WIDTH, hbar_buf);

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

/* ========== 病房利用率分析 ==========
 * 流程: 加载病房/住院数据 -> 依次输出以下四部分报表 -> 释放资源:
 *   1) 病房利用率总览 (当前 occupied/capacity + 进度条)
 *   2) 病房周转率统计 (discharged/capacity)
 *   3) 空床率排名 (按空床率降序, qsort 排序)
 *   4) 各病房历史平均住院天数 (基于已出院记录)
 */
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
    char total_bar[ANALYTICS_BAR_BUF_SIZE];
    render_bar(total_ratio, ANALYTICS_SUMMARY_BAR_WIDTH, total_bar);
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
                    int days = (int)((h->discharge_date - h->admit_date) / ANALYTICS_SECONDS_PER_DAY);
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

/* 科室门诊统计结构体 */
typedef struct DeptWorkloadStats
{
    char name[MAX_NAME_LEN];
    int total;      // 总挂号量
    int done;       // 已完成(已就诊)
    int canceled;   // 已取消
    int recent_30d; // 近30天挂号量
    int prev_30d;   // 前30天挂号量
    float cancel_rate;
    float trend_pct; // 趋势百分比
} DeptWorkloadStats;

/* 医生接诊量统计结构体 */
typedef struct DoctorVisitStats
{
    char d_id[MAX_ID_LEN];
    char name[MAX_NAME_LEN];
    char department[MAX_NAME_LEN];
    int visit_count;
} DoctorVisitStats;

/* 医生接诊量降序比较函数 */
static int compare_doctor_visit_desc(const void *a, const void *b)
{
    const DoctorVisitStats *da = (const DoctorVisitStats *)a;
    const DoctorVisitStats *db = (const DoctorVisitStats *)b;
    return db->visit_count - da->visit_count;
}

/* 计算科室门诊量表格列宽 */
static void calc_dept_workload_width(DeptWorkloadStats *stats, int count, int *dept_w, int *total_w, int *done_w,
                                     int *cancel_w, int *rate_w, int *trend_w)
{
    *dept_w = str_width("科室");
    *total_w = str_width("总数");
    *done_w = str_width("已诊");
    *cancel_w = str_width("取消");
    *rate_w = str_width("取消率");
    *trend_w = str_width("趋势");

    /* 趋势列至少容纳 "↑ +100%" 约10字符 */
    if (*trend_w < ANALYTICS_TREND_COL_MIN_WIDTH)
        *trend_w = ANALYTICS_TREND_COL_MIN_WIDTH;

    for (int i = 0; i < count; i++)
    {
        int nw = str_width(stats[i].name);
        if (nw > *dept_w)
            *dept_w = nw;

        char tmp[32];
        snprintf(tmp, sizeof(tmp), "%d", stats[i].total);
        int tw = str_width(tmp);
        if (tw > *total_w)
            *total_w = tw;

        snprintf(tmp, sizeof(tmp), "%d", stats[i].done);
        int dw = str_width(tmp);
        if (dw > *done_w)
            *done_w = dw;

        snprintf(tmp, sizeof(tmp), "%d", stats[i].canceled);
        int cw = str_width(tmp);
        if (cw > *cancel_w)
            *cancel_w = cw;
    }

    if (*rate_w < ANALYTICS_RATE_COL_MIN_WIDTH)
        *rate_w = ANALYTICS_RATE_COL_MIN_WIDTH;
}

/* 打印科室门诊量表头 */
static void print_dept_workload_header(int dept_w, int total_w, int done_w, int cancel_w, int rate_w, int trend_w)
{
    int widths[] = {dept_w, total_w, done_w, cancel_w, rate_w, trend_w};
    print_analytics_line(6, widths);

    printf("| ");
    print_analytics_align("科室", dept_w);
    printf(" | ");
    print_analytics_align("总数", total_w);
    printf(" | ");
    print_analytics_align("已诊", done_w);
    printf(" | ");
    print_analytics_align("取消", cancel_w);
    printf(" | ");
    print_analytics_align("取消率", rate_w);
    printf(" | ");
    print_analytics_align("趋势", trend_w);
    printf(" |\n");

    print_analytics_line(6, widths);
}

/* 打印科室门诊量数据行 */
static void print_dept_workload_line(DeptWorkloadStats *stat, int dept_w, int total_w, int done_w, int cancel_w,
                                     int rate_w, int trend_w)
{
    char total_str[32], done_str[32], cancel_str[32], rate_str[32], trend_str[64];
    snprintf(total_str, sizeof(total_str), "%d", stat->total);
    snprintf(done_str, sizeof(done_str), "%d", stat->done);
    snprintf(cancel_str, sizeof(cancel_str), "%d", stat->canceled);
    snprintf(rate_str, sizeof(rate_str), "%.1f%%", stat->cancel_rate);

    /* 趋势箭头 */
    if (stat->prev_30d == 0 && stat->recent_30d == 0)
    {
        snprintf(trend_str, sizeof(trend_str), "→ 无数据");
    }
    else if (stat->prev_30d == 0)
    {
        snprintf(trend_str, sizeof(trend_str), "↑ 新增");
    }
    else if (stat->trend_pct > ANALYTICS_TREND_THRESHOLD)
    {
        snprintf(trend_str, sizeof(trend_str), "↑ +%.0f%%", stat->trend_pct);
    }
    else if (stat->trend_pct < -ANALYTICS_TREND_THRESHOLD)
    {
        snprintf(trend_str, sizeof(trend_str), "↓ %.0f%%", stat->trend_pct);
    }
    else
    {
        snprintf(trend_str, sizeof(trend_str), "→ 持平");
    }

    printf("| ");
    print_analytics_align(stat->name, dept_w);
    printf(" | ");
    print_analytics_align(total_str, total_w);
    printf(" | ");
    print_analytics_align(done_str, done_w);
    printf(" | ");
    print_analytics_align(cancel_str, cancel_w);
    printf(" | ");
    print_analytics_align(rate_str, rate_w);
    printf(" | ");
    print_analytics_align(trend_str, trend_w);
    printf(" |\n");
}

/* 计算医生接诊量表格列宽 */
static void calc_doctor_visit_width(DoctorVisitStats *stats, int count, int max_visits, int *rank_w, int *id_w,
                                    int *dept_w, int *cnt_w, int *bar_w)
{
    *rank_w = str_width("排名");
    *id_w = str_width("工号");
    *dept_w = str_width("科室");
    *cnt_w = str_width("次数");
    *bar_w = str_width("分布");

    if (*bar_w < ANALYTICS_BAR_WIDTH)
        *bar_w = ANALYTICS_BAR_WIDTH;

    for (int i = 0; i < count; i++)
    {
        int nw = str_width(stats[i].name);
        /* 工号列显示 "姓名(ID)" 格式 */
        char id_buf[64];
        snprintf(id_buf, sizeof(id_buf), "%s", stats[i].d_id);
        int iw = str_width(id_buf);
        if (iw > *id_w)
            *id_w = iw;

        int dw = str_width(stats[i].department);
        if (dw > *dept_w)
            *dept_w = dw;

        /* 姓名也放到工号列 */
        if (nw > *id_w)
            *id_w = nw;
    }

    (void)max_visits;
}

/* 打印医生接诊量表头 */
static void print_doctor_visit_header(int rank_w, int id_w, int dept_w, int cnt_w, int bar_w)
{
    int widths[] = {rank_w, id_w, dept_w, cnt_w, bar_w};
    print_analytics_line(5, widths);

    printf("| ");
    print_analytics_align("排名", rank_w);
    printf(" | ");
    print_analytics_align("工号", id_w);
    printf(" | ");
    print_analytics_align("科室", dept_w);
    printf(" | ");
    print_analytics_align("次数", cnt_w);
    printf(" | ");
    print_analytics_align("分布", bar_w);
    printf(" |\n");

    print_analytics_line(5, widths);
}

/* 打印医生接诊量数据行 */
static void print_doctor_visit_line(DoctorVisitStats *stat, int rank, int max_visits, int rank_w, int id_w, int dept_w,
                                    int cnt_w, int bar_w)
{
    char rank_str[16], cnt_str[16];
    snprintf(rank_str, sizeof(rank_str), "%d", rank);
    snprintf(cnt_str, sizeof(cnt_str), "%d", stat->visit_count);

    char hbar_buf[ANALYTICS_BAR_BUF_SIZE];
    render_hbar(stat->visit_count, max_visits, ANALYTICS_BAR_WIDTH, hbar_buf);

    printf("| ");
    print_analytics_align(rank_str, rank_w);
    printf(" | ");
    print_analytics_align(stat->d_id, id_w);
    printf(" | ");
    print_analytics_align(stat->department, dept_w);
    printf(" | ");
    print_analytics_align(cnt_str, cnt_w);
    printf(" | ");
    print_analytics_align(hbar_buf, bar_w);
    printf(" |\n");
}

/* ========== 科室门诊量与趋势分析 ==========
 * 流程: 加载挂号/医生/看诊数据 -> 输出以下两部分报表 -> 释放资源:
 *   1) 按科室聚合挂号量/完成量/取消率, 并基于最近N天 vs 前N天窗口计算趋势百分比
 *   2) 医生接诊量 Top N (通过 reg->d_id 反查 visit 聚合)
 */
void analytics_department_workload(void)
{
    clear_screen();

    /* 加载数据 */
    Registration *r_head = load_registrations_from_file();
    Doctor *d_head = load_doctors_from_file();
    Visit *v_head = load_visits_from_file();

    if (!r_head)
    {
        printf("暂无挂号数据！\n");
        if (d_head)
            free_doctors(d_head);
        if (v_head)
            free_visits(v_head);
        wait_enter();
        return;
    }
    if (!d_head)
    {
        printf("暂无医生数据！\n");
        free_registrations(r_head);
        if (v_head)
            free_visits(v_head);
        wait_enter();
        return;
    }

    /* ===== 第一部分: 科室门诊量统计 ===== */
    printf("===== 科室门诊量统计 =====\n");

    /* 收集所有科室名称（从医生链表去重） */
    char dept_names[ANALYTICS_MAX_DEPT_COUNT][MAX_NAME_LEN];
    int dept_count = 0;

    for (Doctor *doc = d_head; doc; doc = doc->next)
    {
        int found = 0;
        for (int i = 0; i < dept_count; i++)
        {
            if (strcmp(dept_names[i], doc->department) == 0)
            {
                found = 1;
                break;
            }
        }
        if (!found && dept_count < ANALYTICS_MAX_DEPT_COUNT)
        {
            strncpy(dept_names[dept_count], doc->department, MAX_NAME_LEN - 1);
            dept_names[dept_count][MAX_NAME_LEN - 1] = '\0';
            dept_count++;
        }
    }

    /* 分配科室统计数组 */
    DeptWorkloadStats *dept_stats = (DeptWorkloadStats *)calloc(dept_count, sizeof(DeptWorkloadStats));
    if (!dept_stats)
    {
        printf("内存分配失败！\n");
        free_registrations(r_head);
        free_doctors(d_head);
        if (v_head)
            free_visits(v_head);
        wait_enter();
        return;
    }

    /* 初始化科室统计 */
    for (int i = 0; i < dept_count; i++)
    {
        strncpy(dept_stats[i].name, dept_names[i], MAX_NAME_LEN - 1);
    }

    /* 计算趋势对比的时间窗口: 最近N天 vs 前N天 */
    time_t now = time(NULL);
    time_t t_recent_start = now - (time_t)ANALYTICS_TREND_WINDOW_DAYS * ANALYTICS_SECONDS_PER_DAY;
    time_t t_prev_start = now - (time_t)(2 * ANALYTICS_TREND_WINDOW_DAYS) * ANALYTICS_SECONDS_PER_DAY;

    /* 遍历挂号记录，按科室分桶统计 */
    for (Registration *r = r_head; r; r = r->next)
    {
        /* 通过 d_id 关联 Doctor 获取科室 */
        Doctor *doc = find_doctor_by_d_id(d_head, r->d_id);
        if (!doc)
            continue;

        /* 找到对应科室 */
        int dept_idx = -1;
        for (int i = 0; i < dept_count; i++)
        {
            if (strcmp(dept_stats[i].name, doc->department) == 0)
            {
                dept_idx = i;
                break;
            }
        }
        if (dept_idx < 0)
            continue;

        /* 总量统计 */
        dept_stats[dept_idx].total++;

        if (r->status == REG_STATUS_DONE)
            dept_stats[dept_idx].done++;
        else if (r->status == REG_STATUS_CANCELED)
            dept_stats[dept_idx].canceled++;

        /* 时间窗口统计: 落入最近N天窗口 -> recent; 落入前N天窗口 -> prev */
        if (r->when >= t_recent_start)
            dept_stats[dept_idx].recent_30d++;
        else if (r->when >= t_prev_start)
            dept_stats[dept_idx].prev_30d++;
    }

    /* 计算取消率和趋势 */
    for (int i = 0; i < dept_count; i++)
    {
        dept_stats[i].cancel_rate =
            (dept_stats[i].total > 0) ? (float)dept_stats[i].canceled / dept_stats[i].total * 100.0f : 0.0f;

        if (dept_stats[i].prev_30d > 0)
        {
            dept_stats[i].trend_pct =
                (float)(dept_stats[i].recent_30d - dept_stats[i].prev_30d) / dept_stats[i].prev_30d * 100.0f;
        }
        else
        {
            dept_stats[i].trend_pct = 0.0f;
        }
    }

    /* 计算列宽 */
    int dept_w, total_w, done_w, cancel_w, rate_w, trend_w;
    calc_dept_workload_width(dept_stats, dept_count, &dept_w, &total_w, &done_w, &cancel_w, &rate_w, &trend_w);

    /* 打印表头 */
    print_dept_workload_header(dept_w, total_w, done_w, cancel_w, rate_w, trend_w);

    /* 打印数据行 */
    for (int i = 0; i < dept_count; i++)
    {
        print_dept_workload_line(&dept_stats[i], dept_w, total_w, done_w, cancel_w, rate_w, trend_w);
    }

    /* 打印分隔线 */
    int d_widths[] = {dept_w, total_w, done_w, cancel_w, rate_w, trend_w};
    print_analytics_line(6, d_widths);

    free(dept_stats);

    /* ===== 第二部分: 医生接诊量 Top 5 ===== */
    printf("\n===== 医生接诊量 Top 5 =====\n");

    /* 统计医生数量 */
    int doc_count = 0;
    for (Doctor *doc = d_head; doc; doc = doc->next)
        doc_count++;

    /* 分配医生接诊量统计数组 */
    DoctorVisitStats *doc_stats = (DoctorVisitStats *)calloc(doc_count, sizeof(DoctorVisitStats));
    if (!doc_stats)
    {
        printf("内存分配失败！\n");
        free_registrations(r_head);
        free_doctors(d_head);
        if (v_head)
            free_visits(v_head);
        wait_enter();
        return;
    }

    /* 初始化医生统计 */
    int idx = 0;
    for (Doctor *doc = d_head; doc; doc = doc->next, idx++)
    {
        strncpy(doc_stats[idx].d_id, doc->id, MAX_ID_LEN - 1);
        strncpy(doc_stats[idx].name, doc->name, MAX_NAME_LEN - 1);
        strncpy(doc_stats[idx].department, doc->department, MAX_NAME_LEN - 1);
        doc_stats[idx].visit_count = 0;
    }

    /* 遍历看诊记录统计接诊量（已完成的看诊） */
    if (v_head)
    {
        for (Visit *v = v_head; v; v = v->next)
        {
            if (v->status != VISIT_STATUS_DONE)
                continue;

            /* 通过 reg_id 找到挂号记录，获取 d_id */
            Registration *reg = find_registration_by_r_id(r_head, v->reg_id);
            if (!reg)
                continue;

            /* 在医生统计数组中找到对应医生 */
            for (int i = 0; i < doc_count; i++)
            {
                if (strcmp(doc_stats[i].d_id, reg->d_id) == 0)
                {
                    doc_stats[i].visit_count++;
                    break;
                }
            }
        }
    }

    /* 按接诊量降序排序 */
    qsort(doc_stats, doc_count, sizeof(DoctorVisitStats), compare_doctor_visit_desc);

    /* 取 Top DOCTOR_TOP_N */
    int top_n = (doc_count < ANALYTICS_DOCTOR_TOP_N) ? doc_count : ANALYTICS_DOCTOR_TOP_N;
    int max_visits = (top_n > 0) ? doc_stats[0].visit_count : 1;
    if (max_visits < 1)
        max_visits = 1;

    /* 计算列宽 */
    int r_rank_w, r_id_w, r_dept_w, r_cnt_w, r_bar_w;
    calc_doctor_visit_width(doc_stats, top_n, max_visits, &r_rank_w, &r_id_w, &r_dept_w, &r_cnt_w, &r_bar_w);

    /* 打印表头 */
    print_doctor_visit_header(r_rank_w, r_id_w, r_dept_w, r_cnt_w, r_bar_w);

    /* 打印数据行 */
    for (int i = 0; i < top_n; i++)
    {
        print_doctor_visit_line(&doc_stats[i], i + 1, max_visits, r_rank_w, r_id_w, r_dept_w, r_cnt_w, r_bar_w);
    }

    /* 打印分隔线 */
    int r_widths[] = {r_rank_w, r_id_w, r_dept_w, r_cnt_w, r_bar_w};
    print_analytics_line(5, r_widths);

    /* 释放资源 */
    free(doc_stats);
    free_registrations(r_head);
    free_doctors(d_head);
    if (v_head)
        free_visits(v_head);
    wait_enter();
}

/*
 * =============== 住院分析与病房优化相关结构体和函数 ===============
 */

/* 病房住院统计结构体 */
typedef struct WardHospStats
{
    char ward_id[MAX_ID_LEN];
    char name[MAX_NAME_LEN];
    int total_admit;
    int current_in;
    int total_days;
    int discharged_count;
    int min_days;
    int max_days;
    float avg_days;
    int capacity;
    float utilization_rate;
} WardHospStats;

/* 科室-病房矩阵结构体 */
typedef struct DeptWardMatrixRow
{
    char dept_name[MAX_NAME_LEN];
    int counts[ANALYTICS_MAX_WARD_COUNT];
} DeptWardMatrixRow;

/* 计算病房住院统计表格列宽 */
static void calc_ward_hosp_stats_width(WardHospStats *stats, int count, int *name_w, int *admit_w, int *current_w,
                                       int *avg_w, int *min_w, int *max_w)
{
    *name_w = str_width("病房名称");
    *admit_w = str_width("入院人次");
    *current_w = str_width("在院中");
    *avg_w = str_width("平均天数");
    *min_w = str_width("最短天数");
    *max_w = str_width("最长天数");

    for (int i = 0; i < count; i++)
    {
        int nw = str_width(stats[i].name);
        if (nw > *name_w)
            *name_w = nw;
    }

    if (*avg_w < 8)
        *avg_w = 8;
}

/* 打印病房住院统计表头 */
static void print_ward_hosp_stats_header(int name_w, int admit_w, int current_w, int avg_w, int min_w, int max_w)
{
    int widths[] = {name_w, admit_w, current_w, avg_w, min_w, max_w};
    print_analytics_line(6, widths);

    printf("| ");
    print_analytics_align("病房名称", name_w);
    printf(" | ");
    print_analytics_align("入院人次", admit_w);
    printf(" | ");
    print_analytics_align("在院中", current_w);
    printf(" | ");
    print_analytics_align("平均天数", avg_w);
    printf(" | ");
    print_analytics_align("最短天数", min_w);
    printf(" | ");
    print_analytics_align("最长天数", max_w);
    printf(" |\n");

    print_analytics_line(6, widths);
}

/* 打印病房住院统计数据行 */
static void print_ward_hosp_stats_line(WardHospStats *stat, int name_w, int admit_w, int current_w, int avg_w,
                                       int min_w, int max_w)
{
    char admit_str[16], current_str[16], avg_str[16], min_str[16], max_str[16];
    snprintf(admit_str, sizeof(admit_str), "%d", stat->total_admit);
    snprintf(current_str, sizeof(current_str), "%d", stat->current_in);
    snprintf(avg_str, sizeof(avg_str), "%.1f", stat->avg_days);
    snprintf(min_str, sizeof(min_str), "%d", stat->discharged_count > 0 ? stat->min_days : 0);
    snprintf(max_str, sizeof(max_str), "%d", stat->discharged_count > 0 ? stat->max_days : 0);

    printf("| ");
    print_analytics_align(stat->name, name_w);
    printf(" | ");
    print_analytics_align(admit_str, admit_w);
    printf(" | ");
    print_analytics_align(current_str, current_w);
    printf(" | ");
    print_analytics_align(avg_str, avg_w);
    printf(" | ");
    print_analytics_align(min_str, min_w);
    printf(" | ");
    print_analytics_align(max_str, max_w);
    printf(" |\n");
}

/* 计算科室-病房矩阵列宽 */
static void calc_dept_ward_matrix_width(DeptWardMatrixRow *rows, int row_count, Ward *w_head, int *dept_w,
                                        int ward_widths[ANALYTICS_MAX_WARD_COUNT])
{
    *dept_w = str_width("科室");
    int idx = 0;
    for (Ward *w = w_head; w; w = w->next, idx++)
    {
        ward_widths[idx] = str_width(w->name);
        if (ward_widths[idx] < 4)
            ward_widths[idx] = 4;
    }

    for (int i = 0; i < row_count; i++)
    {
        int dw = str_width(rows[i].dept_name);
        if (dw > *dept_w)
            *dept_w = dw;

        for (int j = 0; j < idx; j++)
        {
            char tmp[16];
            snprintf(tmp, sizeof(tmp), "%d", rows[i].counts[j]);
            int cw = str_width(tmp);
            if (cw > ward_widths[j])
                ward_widths[j] = cw;
        }
    }
}

/* 打印科室-病房矩阵分隔线 */
static void print_dept_ward_matrix_line(int dept_w, Ward *w_head, const int ward_widths[ANALYTICS_MAX_WARD_COUNT])
{
    printf("+");
    for (int i = 0; i < dept_w + 2; i++)
        printf("-");
    printf("+");

    int idx = 0;
    for (Ward *w = w_head; w; w = w->next, idx++)
    {
        (void)w;
        for (int i = 0; i < ward_widths[idx] + 2; i++)
            printf("-");
        printf("+");
    }
    printf("\n");
}

/* 打印科室-病房矩阵表头 */
static void print_dept_ward_matrix_header(int dept_w, Ward *w_head, const int ward_widths[ANALYTICS_MAX_WARD_COUNT])
{
    print_dept_ward_matrix_line(dept_w, w_head, ward_widths);

    printf("| ");
    print_analytics_align("科室", dept_w);
    printf(" | ");

    int idx = 0;
    for (Ward *w = w_head; w; w = w->next, idx++)
    {
        print_analytics_align(w->name, ward_widths[idx]);
        printf(" | ");
    }
    printf("\n");

    print_dept_ward_matrix_line(dept_w, w_head, ward_widths);
}

/* 打印科室-病房矩阵数据行 */
static void print_dept_ward_matrix_row(DeptWardMatrixRow *row, int dept_w, Ward *w_head,
                                       const int ward_widths[ANALYTICS_MAX_WARD_COUNT])
{
    printf("| ");
    print_analytics_align(row->dept_name, dept_w);
    printf(" | ");

    int idx = 0;
    for (Ward *w = w_head; w; w = w->next, idx++)
    {
        char tmp[16];
        snprintf(tmp, sizeof(tmp), "%d", row->counts[idx]);
        print_analytics_align(tmp, ward_widths[idx]);
        printf(" | ");
    }
    printf("\n");
}

/* ========== 住院分析与病房优化 ==========
 * 流程: 加载 hosp/visit/reg/doctor/ward 数据 -> 输出三部分报表 -> 释放资源:
 *   1) 病房住院统计 (入院人次/在院中/平均/最短/最长天数)
 *   2) 科室-病房住院人次分布矩阵 (通过 hosp->visit->reg->doctor 关联出科室)
 *   3) 病房优化建议, 触发规则:
 *      - 科室需求占比 - 关联床位占比 > DEMAND_BED_GAP   → 建议扩容
 *      - 病房利用率 < LOW_UTIL_THRESHOLD                → 建议缩减/合并
 *      - 综合病房被 ≥ MULTI_DEPT_THRESHOLD 个科室共用   → 作为缓冲病区
 */
void analytics_ward_optimization(void)
{
    clear_screen();

    Hospitalization *h_head = load_hospitalizations_from_file();
    Visit *v_head = load_visits_from_file();
    Registration *r_head = load_registrations_from_file();
    Doctor *d_head = load_doctors_from_file();
    Ward *w_head = load_wards_from_file();

    if (!h_head || !v_head || !r_head || !d_head || !w_head)
    {
        printf("分析所需数据不足！\n");
        if (h_head)
            free_hospitalizations(h_head);
        if (v_head)
            free_visits(v_head);
        if (r_head)
            free_registrations(r_head);
        if (d_head)
            free_doctors(d_head);
        if (w_head)
            free_wards(w_head);
        wait_enter();
        return;
    }

    int ward_count = 0;
    for (Ward *w = w_head; w; w = w->next)
        ward_count++;

    char dept_names[ANALYTICS_MAX_DEPT_COUNT][MAX_NAME_LEN];
    int dept_count = 0;
    for (Doctor *doc = d_head; doc; doc = doc->next)
    {
        int found = 0;
        for (int i = 0; i < dept_count; i++)
        {
            if (strcmp(dept_names[i], doc->department) == 0)
            {
                found = 1;
                break;
            }
        }
        if (!found && dept_count < ANALYTICS_MAX_DEPT_COUNT)
        {
            strncpy(dept_names[dept_count], doc->department, MAX_NAME_LEN - 1);
            dept_names[dept_count][MAX_NAME_LEN - 1] = '\0';
            dept_count++;
        }
    }

    /* ===== 第一部分: 病房住院统计 ===== */
    printf("===== 病房住院统计 =====\n");

    WardHospStats *ward_stats = (WardHospStats *)calloc(ward_count, sizeof(WardHospStats));
    if (!ward_stats)
    {
        printf("内存分配失败！\n");
        free_hospitalizations(h_head);
        free_visits(v_head);
        free_registrations(r_head);
        free_doctors(d_head);
        free_wards(w_head);
        wait_enter();
        return;
    }

    int ward_idx = 0;
    for (Ward *w = w_head; w; w = w->next, ward_idx++)
    {
        strncpy(ward_stats[ward_idx].ward_id, w->ward_id, MAX_ID_LEN - 1);
        strncpy(ward_stats[ward_idx].name, w->name, MAX_NAME_LEN - 1);
        ward_stats[ward_idx].capacity = w->capacity;
        ward_stats[ward_idx].min_days = 0;
        ward_stats[ward_idx].max_days = 0;
        ward_stats[ward_idx].utilization_rate = (w->capacity > 0) ? (float)w->occupied / w->capacity : 0.0f;
    }

    for (Hospitalization *h = h_head; h; h = h->next)
    {
        for (int i = 0; i < ward_count; i++)
        {
            if (strcmp(ward_stats[i].ward_id, h->ward_id) == 0)
            {
                ward_stats[i].total_admit++;
                if (h->status == HOSP_STATUS_ONGOING)
                    ward_stats[i].current_in++;

                if (h->status == HOSP_STATUS_DISCHARGED && h->discharge_date > h->admit_date)
                {
                    int days = (int)((h->discharge_date - h->admit_date) / ANALYTICS_SECONDS_PER_DAY);
                    if (days < 1)
                        days = 1;

                    ward_stats[i].total_days += days;
                    ward_stats[i].discharged_count++;

                    if (ward_stats[i].min_days == 0 || days < ward_stats[i].min_days)
                        ward_stats[i].min_days = days;
                    if (days > ward_stats[i].max_days)
                        ward_stats[i].max_days = days;
                }
                break;
            }
        }
    }

    for (int i = 0; i < ward_count; i++)
    {
        if (ward_stats[i].discharged_count > 0)
            ward_stats[i].avg_days = (float)ward_stats[i].total_days / ward_stats[i].discharged_count;
    }

    int name_w, admit_w, current_w, avg_w, min_w, max_w;
    calc_ward_hosp_stats_width(ward_stats, ward_count, &name_w, &admit_w, &current_w, &avg_w, &min_w, &max_w);
    print_ward_hosp_stats_header(name_w, admit_w, current_w, avg_w, min_w, max_w);
    for (int i = 0; i < ward_count; i++)
        print_ward_hosp_stats_line(&ward_stats[i], name_w, admit_w, current_w, avg_w, min_w, max_w);
    int hosp_widths[] = {name_w, admit_w, current_w, avg_w, min_w, max_w};
    print_analytics_line(6, hosp_widths);

    /* ===== 第二部分: 科室-病房住院人次分布 ===== */
    printf("\n===== 科室-病房 住院人次分布 =====\n");

    DeptWardMatrixRow *matrix_rows = (DeptWardMatrixRow *)calloc(dept_count, sizeof(DeptWardMatrixRow));
    if (!matrix_rows)
    {
        printf("内存分配失败！\n");
        free(ward_stats);
        free_hospitalizations(h_head);
        free_visits(v_head);
        free_registrations(r_head);
        free_doctors(d_head);
        free_wards(w_head);
        wait_enter();
        return;
    }

    for (int i = 0; i < dept_count; i++)
        strncpy(matrix_rows[i].dept_name, dept_names[i], MAX_NAME_LEN - 1);

    for (Hospitalization *h = h_head; h; h = h->next)
    {
        Visit *v = find_visit_by_v_id(v_head, h->visit_id);
        if (!v)
            continue;
        Registration *r = find_registration_by_r_id(r_head, v->reg_id);
        if (!r)
            continue;
        Doctor *doc = find_doctor_by_d_id(d_head, r->d_id);
        if (!doc)
            continue;

        int dept_idx = -1;
        for (int i = 0; i < dept_count; i++)
        {
            if (strcmp(matrix_rows[i].dept_name, doc->department) == 0)
            {
                dept_idx = i;
                break;
            }
        }
        if (dept_idx < 0)
            continue;

        int cur_ward_idx = 0;
        for (Ward *w = w_head; w; w = w->next, cur_ward_idx++)
        {
            if (strcmp(w->ward_id, h->ward_id) == 0)
            {
                matrix_rows[dept_idx].counts[cur_ward_idx]++;
                break;
            }
        }
    }

    int dept_w;
    int ward_widths[ANALYTICS_MAX_WARD_COUNT] = {0};
    calc_dept_ward_matrix_width(matrix_rows, dept_count, w_head, &dept_w, ward_widths);
    print_dept_ward_matrix_header(dept_w, w_head, ward_widths);
    for (int i = 0; i < dept_count; i++)
        print_dept_ward_matrix_row(&matrix_rows[i], dept_w, w_head, ward_widths);
    print_dept_ward_matrix_line(dept_w, w_head, ward_widths);

    /* ===== 第三部分: 病房优化建议 ===== */
    printf("\n===== 病房优化建议 =====\n");

    int suggestion_no = 1;
    int total_demand = 0;
    for (int i = 0; i < ward_count; i++)
        total_demand += ward_stats[i].total_admit + ward_stats[i].current_in;

    int total_capacity = 0;
    for (Ward *w = w_head; w; w = w->next)
        total_capacity += w->capacity;

    for (int d = 0; d < dept_count; d++)
    {
        int dept_demand = 0;
        int dept_primary_beds = 0;

        for (int w = 0; w < ward_count; w++)
            dept_demand += matrix_rows[d].counts[w];

        for (int w = 0; w < ward_count; w++)
        {
            if (strstr(ward_stats[w].name, matrix_rows[d].dept_name) != NULL)
                dept_primary_beds += ward_stats[w].capacity;
        }

        float demand_ratio = (total_demand > 0) ? (float)dept_demand / total_demand : 0.0f;
        float bed_ratio = (total_capacity > 0) ? (float)dept_primary_beds / total_capacity : 0.0f;

        if (dept_demand > 0 && demand_ratio - bed_ratio > ANALYTICS_DEMAND_BED_GAP)
        {
            printf("  [%d] %s住院需求占比 %.0f%%，关联床位占比 %.0f%%\n", suggestion_no++, matrix_rows[d].dept_name,
                   demand_ratio * 100.0f, bed_ratio * 100.0f);
            printf("      -> 建议: 优先为%s关联病房扩容或预留更多弹性床位\n", matrix_rows[d].dept_name);
        }
    }

    for (int i = 0; i < ward_count; i++)
    {
        if (ward_stats[i].utilization_rate < ANALYTICS_LOW_UTIL_THRESHOLD)
        {
            printf("  [%d] %s当前利用率仅 %.1f%%\n", suggestion_no++, ward_stats[i].name,
                   ward_stats[i].utilization_rate * 100.0f);
            printf("      -> 建议: 评估缩减床位或与相邻病区合并管理\n");
        }
    }

    for (int i = 0; i < ward_count; i++)
    {
        if (strstr(ward_stats[i].name, "综合") != NULL)
        {
            int used_dept = 0;
            for (int d = 0; d < dept_count; d++)
            {
                if (matrix_rows[d].counts[i] > 0)
                    used_dept++;
            }
            if (used_dept >= ANALYTICS_MULTI_DEPT_THRESHOLD)
            {
                printf("  [%d] %s被 %d 个科室共同使用\n", suggestion_no++, ward_stats[i].name, used_dept);
                printf("      -> 建议: 保持弹性分配，作为全院溢出缓冲病区\n");
            }
        }
    }

    if (suggestion_no == 1)
        printf("  当前病房资源配置整体较均衡，暂未发现明显扩缩容建议。\n");

    free(matrix_rows);
    free(ward_stats);
    free_hospitalizations(h_head);
    free_visits(v_head);
    free_registrations(r_head);
    free_doctors(d_head);
    free_wards(w_head);
    wait_enter();
}

/*
 * =============== 药品使用分析相关结构体和函数 ===============
 */

/* 药品使用统计结构体 */
typedef struct DrugUsageStats
{
    char drug_id[MAX_ID_LEN];
    char name[MAX_NAME_LEN];
    int prescription_count;
    float price;
    int stock;
} DrugUsageStats;

/* 科室用药金额统计结构体 */
typedef struct DeptDrugCostStats
{
    char dept_name[MAX_NAME_LEN];
    int prescription_count;
    float total_cost;
} DeptDrugCostStats;

/* 库存预警统计结构体 */
typedef struct DrugStockWarnStats
{
    char name[MAX_NAME_LEN];
    int stock;
    float monthly_usage;
    float months_left;
} DrugStockWarnStats;

/* 药品使用量降序比较函数 */
static int compare_drug_usage_desc(const void *a, const void *b)
{
    const DrugUsageStats *da = (const DrugUsageStats *)a;
    const DrugUsageStats *db = (const DrugUsageStats *)b;
    return db->prescription_count - da->prescription_count;
}

/* 计算药品排行表格列宽 */
static void calc_drug_rank_width(DrugUsageStats *stats, int count, int total_pr, int *rank_w, int *name_w, int *cnt_w,
                                 int *pct_w, int *bar_w)
{
    *rank_w = str_width("排名");
    *name_w = str_width("药品名称");
    *cnt_w = str_width("次数");
    *pct_w = str_width("占比");
    *bar_w = str_width("分布");

    if (*bar_w < ANALYTICS_BAR_WIDTH)
        *bar_w = ANALYTICS_BAR_WIDTH;
    if (*pct_w < ANALYTICS_RATE_COL_MIN_WIDTH)
        *pct_w = ANALYTICS_RATE_COL_MIN_WIDTH;

    for (int i = 0; i < count; i++)
    {
        int nw = str_width(stats[i].name);
        if (nw > *name_w)
            *name_w = nw;
    }

    (void)total_pr;
}

/* 打印药品排行表头 */
static void print_drug_rank_header(int rank_w, int name_w, int cnt_w, int pct_w, int bar_w)
{
    int widths[] = {rank_w, name_w, cnt_w, pct_w, bar_w};
    print_analytics_line(5, widths);

    printf("| ");
    print_analytics_align("排名", rank_w);
    printf(" | ");
    print_analytics_align("药品名称", name_w);
    printf(" | ");
    print_analytics_align("次数", cnt_w);
    printf(" | ");
    print_analytics_align("占比", pct_w);
    printf(" | ");
    print_analytics_align("分布", bar_w);
    printf(" |\n");

    print_analytics_line(5, widths);
}

/* 打印药品排行数据行 */
static void print_drug_rank_line(DrugUsageStats *stat, int rank, int max_count, int total_pr, int rank_w, int name_w,
                                 int cnt_w, int pct_w, int bar_w)
{
    char rank_str[16], cnt_str[16], pct_str[16];
    snprintf(rank_str, sizeof(rank_str), "%d", rank);
    snprintf(cnt_str, sizeof(cnt_str), "%d", stat->prescription_count);
    float pct = (total_pr > 0) ? (float)stat->prescription_count / total_pr * 100.0f : 0.0f;
    snprintf(pct_str, sizeof(pct_str), "%.1f%%", pct);

    char hbar_buf[ANALYTICS_BAR_BUF_SIZE];
    render_hbar(stat->prescription_count, max_count, ANALYTICS_BAR_WIDTH, hbar_buf);

    printf("| ");
    print_analytics_align(rank_str, rank_w);
    printf(" | ");
    print_analytics_align(stat->name, name_w);
    printf(" | ");
    print_analytics_align(cnt_str, cnt_w);
    printf(" | ");
    print_analytics_align(pct_str, pct_w);
    printf(" | ");
    print_analytics_align(hbar_buf, bar_w);
    printf(" |\n");
}

/* 计算科室用药金额表格列宽 */
static void calc_dept_drug_cost_width(DeptDrugCostStats *stats, int count, int *dept_w, int *cnt_w, int *cost_w)
{
    *dept_w = str_width("科室");
    *cnt_w = str_width("处方次数");
    *cost_w = str_width("金额(元)");

    if (*cost_w < 10)
        *cost_w = 10;

    for (int i = 0; i < count; i++)
    {
        int dw = str_width(stats[i].dept_name);
        if (dw > *dept_w)
            *dept_w = dw;
    }
}

/* 打印科室用药金额表头 */
static void print_dept_drug_cost_header(int dept_w, int cnt_w, int cost_w)
{
    int widths[] = {dept_w, cnt_w, cost_w};
    print_analytics_line(3, widths);

    printf("| ");
    print_analytics_align("科室", dept_w);
    printf(" | ");
    print_analytics_align("处方次数", cnt_w);
    printf(" | ");
    print_analytics_align("金额(元)", cost_w);
    printf(" |\n");

    print_analytics_line(3, widths);
}

/* 打印科室用药金额数据行 */
static void print_dept_drug_cost_line(DeptDrugCostStats *stat, int dept_w, int cnt_w, int cost_w)
{
    char cnt_str[16], cost_str[32];
    snprintf(cnt_str, sizeof(cnt_str), "%d", stat->prescription_count);
    snprintf(cost_str, sizeof(cost_str), "%.1f", stat->total_cost);

    printf("| ");
    print_analytics_align(stat->dept_name, dept_w);
    printf(" | ");
    print_analytics_align(cnt_str, cnt_w);
    printf(" | ");
    print_analytics_align(cost_str, cost_w);
    printf(" |\n");
}

/* 计算库存预警表格列宽 */
static void calc_stock_warn_width(DrugStockWarnStats *stats, int count, int *name_w, int *stock_w, int *usage_w,
                                  int *months_w, int *status_w)
{
    *name_w = str_width("药品名称");
    *stock_w = str_width("库存");
    *usage_w = str_width("月均消耗");
    *months_w = str_width("可用月数");
    *status_w = str_width("状态");

    /* 状态列至少容纳 "⚠ 补货" */
    if (*status_w < ANALYTICS_STATUS_COL_MIN_WIDTH)
        *status_w = ANALYTICS_STATUS_COL_MIN_WIDTH;

    for (int i = 0; i < count; i++)
    {
        int nw = str_width(stats[i].name);
        if (nw > *name_w)
            *name_w = nw;
    }
}

/* 打印库存预警表头 */
static void print_stock_warn_header(int name_w, int stock_w, int usage_w, int months_w, int status_w)
{
    int widths[] = {name_w, stock_w, usage_w, months_w, status_w};
    print_analytics_line(5, widths);

    printf("| ");
    print_analytics_align("药品名称", name_w);
    printf(" | ");
    print_analytics_align("库存", stock_w);
    printf(" | ");
    print_analytics_align("月均消耗", usage_w);
    printf(" | ");
    print_analytics_align("可用月数", months_w);
    printf(" | ");
    print_analytics_align("状态", status_w);
    printf(" |\n");

    print_analytics_line(5, widths);
}

/* 打印库存预警数据行 */
static void print_stock_warn_line(DrugStockWarnStats *stat, int name_w, int stock_w, int usage_w, int months_w,
                                  int status_w)
{
    char stock_str[16], usage_str[16], months_str[16], status_str[32];
    snprintf(stock_str, sizeof(stock_str), "%d", stat->stock);
    snprintf(usage_str, sizeof(usage_str), "~%.0f", stat->monthly_usage);
    snprintf(months_str, sizeof(months_str), "%.1f", stat->months_left);

    if (stat->months_left < ANALYTICS_STOCK_WARN_CRITICAL)
        snprintf(status_str, sizeof(status_str), "⚠ 补货");
    else if (stat->months_left < ANALYTICS_STOCK_WARN_WATCH)
        snprintf(status_str, sizeof(status_str), "⚠ 关注");
    else
        snprintf(status_str, sizeof(status_str), "✓ 正常");

    printf("| ");
    print_analytics_align(stat->name, name_w);
    printf(" | ");
    print_analytics_align(stock_str, stock_w);
    printf(" | ");
    print_analytics_align(usage_str, usage_w);
    printf(" | ");
    print_analytics_align(months_str, months_w);
    printf(" | ");
    print_analytics_align(status_str, status_w);
    printf(" |\n");
}

/* ========== 药品使用分析 ==========
 * 流程: 加载处方/药品/医生数据 -> 输出三部分报表 -> 释放资源:
 *   1) 药品使用排行 Top DRUG_TOP_N (按处方次数降序)
 *   2) 科室用药金额估算 (通过 pr->d_id 关联科室, 处方次数 * 药品单价)
 *   3) 库存预警 (月均消耗 = 总处方数 / DATA_WINDOW_MONTHS, 按可用月数分级告警)
 */
void analytics_drug_usage(void)
{
    clear_screen();

    Prescription *pr_head = load_prescriptions_from_file();
    Drug *drug_head = load_drugs_from_file();
    Doctor *d_head = load_doctors_from_file();

    if (!pr_head)
    {
        printf("暂无处方数据！\n");
        if (drug_head)
            free_drugs(drug_head);
        if (d_head)
            free_doctors(d_head);
        wait_enter();
        return;
    }
    if (!drug_head)
    {
        printf("暂无药品数据！\n");
        free_prescriptions(pr_head);
        if (d_head)
            free_doctors(d_head);
        wait_enter();
        return;
    }

    /* ===== 第一部分: 药品使用排行 Top 10 ===== */
    printf("===== 药品使用排行 Top 10 =====\n");

    int drug_count = 0;
    for (Drug *d = drug_head; d; d = d->next)
        drug_count++;

    DrugUsageStats *drug_stats = (DrugUsageStats *)calloc(drug_count, sizeof(DrugUsageStats));
    if (!drug_stats)
    {
        printf("内存分配失败！\n");
        free_prescriptions(pr_head);
        free_drugs(drug_head);
        if (d_head)
            free_doctors(d_head);
        wait_enter();
        return;
    }

    int idx = 0;
    for (Drug *d = drug_head; d; d = d->next, idx++)
    {
        strncpy(drug_stats[idx].drug_id, d->id, MAX_ID_LEN - 1);
        strncpy(drug_stats[idx].name, d->trade_name, MAX_NAME_LEN - 1);
        drug_stats[idx].price = d->price;
        drug_stats[idx].stock = d->stock;
    }

    int total_pr = 0;
    for (Prescription *pr = pr_head; pr; pr = pr->next)
    {
        total_pr++;
        for (int i = 0; i < drug_count; i++)
        {
            if (strcmp(drug_stats[i].drug_id, pr->drug_id) == 0)
            {
                drug_stats[i].prescription_count++;
                break;
            }
        }
    }

    qsort(drug_stats, drug_count, sizeof(DrugUsageStats), compare_drug_usage_desc);

    int top_n = (drug_count < ANALYTICS_DRUG_TOP_N) ? drug_count : ANALYTICS_DRUG_TOP_N;
    int max_count = (top_n > 0) ? drug_stats[0].prescription_count : 1;
    if (max_count < 1)
        max_count = 1;

    int rank_w, dname_w, cnt_w, pct_w, bar_w;
    calc_drug_rank_width(drug_stats, top_n, total_pr, &rank_w, &dname_w, &cnt_w, &pct_w, &bar_w);
    print_drug_rank_header(rank_w, dname_w, cnt_w, pct_w, bar_w);
    for (int i = 0; i < top_n; i++)
        print_drug_rank_line(&drug_stats[i], i + 1, max_count, total_pr, rank_w, dname_w, cnt_w, pct_w, bar_w);
    int rank_widths[] = {rank_w, dname_w, cnt_w, pct_w, bar_w};
    print_analytics_line(5, rank_widths);

    /* ===== 第二部分: 科室用药金额估算 ===== */
    printf("\n===== 科室用药金额估算 =====\n");

    char dept_names[ANALYTICS_MAX_DEPT_COUNT][MAX_NAME_LEN];
    int dept_count = 0;
    if (d_head)
    {
        for (Doctor *doc = d_head; doc; doc = doc->next)
        {
            int found = 0;
            for (int i = 0; i < dept_count; i++)
            {
                if (strcmp(dept_names[i], doc->department) == 0)
                {
                    found = 1;
                    break;
                }
            }
            if (!found && dept_count < ANALYTICS_MAX_DEPT_COUNT)
            {
                strncpy(dept_names[dept_count], doc->department, MAX_NAME_LEN - 1);
                dept_names[dept_count][MAX_NAME_LEN - 1] = '\0';
                dept_count++;
            }
        }
    }

    DeptDrugCostStats *cost_stats = (DeptDrugCostStats *)calloc(dept_count, sizeof(DeptDrugCostStats));
    if (cost_stats && d_head)
    {
        for (int i = 0; i < dept_count; i++)
            strncpy(cost_stats[i].dept_name, dept_names[i], MAX_NAME_LEN - 1);

        for (Prescription *pr = pr_head; pr; pr = pr->next)
        {
            Doctor *doc = find_doctor_by_d_id(d_head, pr->d_id);
            if (!doc)
                continue;

            Drug *drug = find_drug_by_id(drug_head, pr->drug_id);
            float price = drug ? drug->price : 0.0f;

            for (int i = 0; i < dept_count; i++)
            {
                if (strcmp(cost_stats[i].dept_name, doc->department) == 0)
                {
                    cost_stats[i].prescription_count++;
                    cost_stats[i].total_cost += price;
                    break;
                }
            }
        }

        int dept_w, dcnt_w, dcost_w;
        calc_dept_drug_cost_width(cost_stats, dept_count, &dept_w, &dcnt_w, &dcost_w);
        print_dept_drug_cost_header(dept_w, dcnt_w, dcost_w);
        for (int i = 0; i < dept_count; i++)
            print_dept_drug_cost_line(&cost_stats[i], dept_w, dcnt_w, dcost_w);
        int cost_widths[] = {dept_w, dcnt_w, dcost_w};
        print_analytics_line(3, cost_widths);

        free(cost_stats);
    }

    /* ===== 第三部分: 库存预警 ===== */
    printf("\n===== 库存预警 =====\n");

    /* 数据跨度：gen_data.py 生成90天数据，约3个月 */
    float data_months = ANALYTICS_DATA_WINDOW_MONTHS;

    DrugStockWarnStats *warn_stats = (DrugStockWarnStats *)calloc(drug_count, sizeof(DrugStockWarnStats));
    if (warn_stats)
    {
        for (int i = 0; i < drug_count; i++)
        {
            strncpy(warn_stats[i].name, drug_stats[i].name, MAX_NAME_LEN - 1);
            warn_stats[i].stock = drug_stats[i].stock;
            warn_stats[i].monthly_usage =
                (data_months > 0) ? (float)drug_stats[i].prescription_count / data_months : 0.0f;
            warn_stats[i].months_left =
                (warn_stats[i].monthly_usage > 0) ? (float)warn_stats[i].stock / warn_stats[i].monthly_usage : 999.0f;
        }

        int wname_w, wstock_w, wusage_w, wmonths_w, wstatus_w;
        calc_stock_warn_width(warn_stats, drug_count, &wname_w, &wstock_w, &wusage_w, &wmonths_w, &wstatus_w);
        print_stock_warn_header(wname_w, wstock_w, wusage_w, wmonths_w, wstatus_w);

        for (int i = 0; i < drug_count; i++)
        {
            if (drug_stats[i].prescription_count > 0)
                print_stock_warn_line(&warn_stats[i], wname_w, wstock_w, wusage_w, wmonths_w, wstatus_w);
        }

        int warn_widths[] = {wname_w, wstock_w, wusage_w, wmonths_w, wstatus_w};
        print_analytics_line(5, warn_widths);

        printf("  状态说明: ⚠ 补货 = 可用不足3月需立即补货\n");
        printf("            ⚠ 关注 = 可用不足6月建议关注\n");
        printf("            ✓ 正常 = 库存充足\n");

        free(warn_stats);
    }

    free(drug_stats);
    free_prescriptions(pr_head);
    free_drugs(drug_head);
    if (d_head)
        free_doctors(d_head);
    wait_enter();
}

/*
 * =============== 住院时长分布与预测相关结构体和函数 ===============
 */

/* 住院天数分桶的标签与区间边界(闭区间) */
static const char *STAY_BUCKET_LABELS[ANALYTICS_STAY_BUCKET_COUNT] = {"1-3 天", "4-7 天", "8-14 天", "15-30 天",
                                                                      ">30 天"};
static const int STAY_BUCKET_MIN[ANALYTICS_STAY_BUCKET_COUNT] = {1, 4, 8, 15, 31};
static const int STAY_BUCKET_MAX[ANALYTICS_STAY_BUCKET_COUNT] = {3, 7, 14, 30, 9999};

/* 住院天数分桶统计结构体 */
typedef struct StayBucketStats
{
    const char *label;
    int count;
} StayBucketStats;

/* 分病房平均住院天数结构体（复用 WardStayStats） */

/* 在院患者关注提醒结构体 */
typedef struct InpatientAlertStats
{
    char p_id[MAX_ID_LEN];
    char p_name[MAX_NAME_LEN];
    char ward_name[MAX_NAME_LEN];
    int stayed_days;
    float ward_avg;
    int is_overdue; // 1=超期, 0=正常
} InpatientAlertStats;

/* 计算住院天数分布表格列宽 */
static void calc_stay_bucket_width(StayBucketStats *stats, int count, int max_count, int *label_w, int *cnt_w,
                                   int *bar_w)
{
    *label_w = str_width("天数区间");
    *cnt_w = str_width("人次");
    *bar_w = str_width("分布");

    if (*bar_w < ANALYTICS_BAR_DISPLAY_WIDTH)
        *bar_w = ANALYTICS_BAR_DISPLAY_WIDTH;

    for (int i = 0; i < count; i++)
    {
        int lw = str_width(stats[i].label);
        if (lw > *label_w)
            *label_w = lw;
    }

    (void)max_count;
}

/* 打印住院天数分布表头 */
static void print_stay_bucket_header(int label_w, int cnt_w, int bar_w)
{
    int widths[] = {label_w, cnt_w, bar_w};
    print_analytics_line(3, widths);

    printf("| ");
    print_analytics_align("天数区间", label_w);
    printf(" | ");
    print_analytics_align("人次", cnt_w);
    printf(" | ");
    print_analytics_align("分布", bar_w);
    printf(" |\n");

    print_analytics_line(3, widths);
}

/* 打印住院天数分布数据行 */
static void print_stay_bucket_line(StayBucketStats *stat, int max_count, int label_w, int cnt_w, int bar_w)
{
    char cnt_str[16];
    snprintf(cnt_str, sizeof(cnt_str), "%d", stat->count);

    float ratio = (max_count > 0) ? (float)stat->count / max_count : 0.0f;
    char bar_buf[ANALYTICS_BAR_BUF_SIZE];
    render_bar(ratio, ANALYTICS_BAR_WIDTH, bar_buf);

    printf("| ");
    print_analytics_align(stat->label, label_w);
    printf(" | ");
    print_analytics_align(cnt_str, cnt_w);
    printf(" | ");
    print_analytics_align(bar_buf, bar_w);
    printf(" |\n");
}

/* 计算在院患者关注提醒表格列宽 */
static void calc_inpatient_alert_width(InpatientAlertStats *stats, int count, int *pid_w, int *name_w, int *ward_w,
                                       int *stayed_w, int *avg_w, int *status_w)
{
    *pid_w = str_width("患者ID");
    *name_w = str_width("姓名");
    *ward_w = str_width("所在病房");
    *stayed_w = str_width("已住天数");
    *avg_w = str_width("病房均值");
    *status_w = str_width("状态");

    if (*status_w < ANALYTICS_STATUS_COL_MIN_WIDTH)
        *status_w = ANALYTICS_STATUS_COL_MIN_WIDTH;

    for (int i = 0; i < count; i++)
    {
        int pw = str_width(stats[i].p_id);
        if (pw > *pid_w)
            *pid_w = pw;

        int nw = str_width(stats[i].p_name);
        if (nw > *name_w)
            *name_w = nw;

        int ww = str_width(stats[i].ward_name);
        if (ww > *ward_w)
            *ward_w = ww;
    }
}

/* 打印在院患者关注提醒表头 */
static void print_inpatient_alert_header(int pid_w, int name_w, int ward_w, int stayed_w, int avg_w, int status_w)
{
    int widths[] = {pid_w, name_w, ward_w, stayed_w, avg_w, status_w};
    print_analytics_line(6, widths);

    printf("| ");
    print_analytics_align("患者ID", pid_w);
    printf(" | ");
    print_analytics_align("姓名", name_w);
    printf(" | ");
    print_analytics_align("所在病房", ward_w);
    printf(" | ");
    print_analytics_align("已住天数", stayed_w);
    printf(" | ");
    print_analytics_align("病房均值", avg_w);
    printf(" | ");
    print_analytics_align("状态", status_w);
    printf(" |\n");

    print_analytics_line(6, widths);
}

/* 打印在院患者关注提醒数据行 */
static void print_inpatient_alert_line(InpatientAlertStats *stat, int pid_w, int name_w, int ward_w, int stayed_w,
                                       int avg_w, int status_w)
{
    char stayed_str[16], avg_str[16], status_str[32];
    snprintf(stayed_str, sizeof(stayed_str), "%d", stat->stayed_days);
    snprintf(avg_str, sizeof(avg_str), "%.1f", stat->ward_avg);

    if (stat->is_overdue)
        snprintf(status_str, sizeof(status_str), "⚠ 超期");
    else
        snprintf(status_str, sizeof(status_str), "✓ 正常");

    printf("| ");
    print_analytics_align(stat->p_id, pid_w);
    printf(" | ");
    print_analytics_align(stat->p_name, name_w);
    printf(" | ");
    print_analytics_align(stat->ward_name, ward_w);
    printf(" | ");
    print_analytics_align(stayed_str, stayed_w);
    printf(" | ");
    print_analytics_align(avg_str, avg_w);
    printf(" | ");
    print_analytics_align(status_str, status_w);
    printf(" |\n");
}

/* ========== 住院时长分布与预测 ==========
 * 流程: 加载住院/病房/患者数据 -> 输出三部分报表 -> 释放资源:
 *   1) 住院天数分布 (按 STAY_BUCKET 分桶已出院记录)
 *   2) 分病房平均住院天数 (复用 WardStayStats)
 *   3) 在院患者关注提醒 (已住天数 > 病房均值 * OVERDUE_MULTIPLIER → 超期)
 */
void analytics_hospitalization_duration(void)
{
    clear_screen();

    Hospitalization *h_head = load_hospitalizations_from_file();
    Ward *w_head = load_wards_from_file();
    Patient *p_head = load_patients_from_file();

    if (!h_head)
    {
        printf("暂无住院数据！\n");
        if (w_head)
            free_wards(w_head);
        if (p_head)
            free_patients(p_head);
        wait_enter();
        return;
    }

    /* ===== 第一部分: 住院天数分布（已出院）===== */
    printf("===== 住院天数分布（已出院）=====\n");

    StayBucketStats buckets[ANALYTICS_STAY_BUCKET_COUNT];
    for (int i = 0; i < ANALYTICS_STAY_BUCKET_COUNT; i++)
    {
        buckets[i].label = STAY_BUCKET_LABELS[i];
        buckets[i].count = 0;
    }

    int total_discharged = 0;
    int total_stay_days = 0;

    for (Hospitalization *h = h_head; h; h = h->next)
    {
        if (h->status == HOSP_STATUS_DISCHARGED && h->discharge_date > h->admit_date)
        {
            int days = (int)((h->discharge_date - h->admit_date) / ANALYTICS_SECONDS_PER_DAY);
            if (days < 1)
                days = 1;

            total_discharged++;
            total_stay_days += days;

            for (int i = 0; i < ANALYTICS_STAY_BUCKET_COUNT; i++)
            {
                if (days >= STAY_BUCKET_MIN[i] && days <= STAY_BUCKET_MAX[i])
                {
                    buckets[i].count++;
                    break;
                }
            }
        }
    }

    int max_bucket = 0;
    for (int i = 0; i < ANALYTICS_STAY_BUCKET_COUNT; i++)
    {
        if (buckets[i].count > max_bucket)
            max_bucket = buckets[i].count;
    }

    int label_w, bcnt_w, bbar_w;
    calc_stay_bucket_width(buckets, ANALYTICS_STAY_BUCKET_COUNT, max_bucket, &label_w, &bcnt_w, &bbar_w);
    print_stay_bucket_header(label_w, bcnt_w, bbar_w);
    for (int i = 0; i < ANALYTICS_STAY_BUCKET_COUNT; i++)
        print_stay_bucket_line(&buckets[i], max_bucket, label_w, bcnt_w, bbar_w);
    int bucket_widths[] = {label_w, bcnt_w, bbar_w};
    print_analytics_line(3, bucket_widths);

    float overall_avg = (total_discharged > 0) ? (float)total_stay_days / total_discharged : 0.0f;
    printf("  全院平均住院天数: %.1f 天\n", overall_avg);

    /* ===== 第二部分: 分病房平均住院天数 ===== */
    printf("\n===== 分病房平均住院天数 =====\n");

    int ward_count = 0;
    for (Ward *w = w_head; w; w = w->next)
        ward_count++;

    /* 复用 WardStayStats 结构体 */
    WardStayStats *ward_stays = (WardStayStats *)calloc(ward_count, sizeof(WardStayStats));
    if (ward_stays)
    {
        int idx = 0;
        for (Ward *w = w_head; w; w = w->next, idx++)
        {
            strncpy(ward_stays[idx].ward_id, w->ward_id, MAX_ID_LEN - 1);
            strncpy(ward_stays[idx].name, w->name, MAX_NAME_LEN - 1);
        }

        for (Hospitalization *h = h_head; h; h = h->next)
        {
            if (h->status == HOSP_STATUS_DISCHARGED && h->discharge_date > h->admit_date)
            {
                int days = (int)((h->discharge_date - h->admit_date) / ANALYTICS_SECONDS_PER_DAY);
                if (days < 1)
                    days = 1;
                for (int i = 0; i < ward_count; i++)
                {
                    if (strcmp(ward_stays[i].ward_id, h->ward_id) == 0)
                    {
                        ward_stays[i].total_days += days;
                        ward_stays[i].discharged_count++;
                        break;
                    }
                }
            }
        }

        float max_avg = 0.0f;
        for (int i = 0; i < ward_count; i++)
        {
            if (ward_stays[i].discharged_count > 0)
            {
                ward_stays[i].avg_days = (float)ward_stays[i].total_days / ward_stays[i].discharged_count;
                if (ward_stays[i].avg_days > max_avg)
                    max_avg = ward_stays[i].avg_days;
            }
        }

        int s_name_w, s_cnt_w, s_avg_w, s_bar_w;
        calc_ward_stay_width(ward_stays, ward_count, &s_name_w, &s_cnt_w, &s_avg_w, &s_bar_w);
        print_ward_stay_header(s_name_w, s_cnt_w, s_avg_w, s_bar_w);

        int max_int = (int)(max_avg + 0.5f);
        if (max_int < 1)
            max_int = 1;
        for (int i = 0; i < ward_count; i++)
            print_ward_stay_line(&ward_stays[i], max_int, s_name_w, s_cnt_w, s_avg_w, s_bar_w);
        print_ward_stay_line_separator(s_name_w, s_cnt_w, s_avg_w, s_bar_w);

        /* ===== 第三部分: 在院患者关注提醒 ===== */
        printf("\n===== 在院患者关注提醒 =====\n");

        int ongoing_count = 0;
        for (Hospitalization *h = h_head; h; h = h->next)
        {
            if (h->status == HOSP_STATUS_ONGOING)
                ongoing_count++;
        }

        if (ongoing_count > 0 && p_head)
        {
            InpatientAlertStats *alerts = (InpatientAlertStats *)calloc(ongoing_count, sizeof(InpatientAlertStats));
            if (alerts)
            {
                int alert_idx = 0;
                time_t now = time(NULL);

                for (Hospitalization *h = h_head; h; h = h->next)
                {
                    if (h->status != HOSP_STATUS_ONGOING)
                        continue;

                    int stayed = (int)((now - h->admit_date) / ANALYTICS_SECONDS_PER_DAY);
                    if (stayed < 1)
                        stayed = 1;

                    /* 查找病房平均值 */
                    float ward_avg = overall_avg;
                    const char *ward_name = h->ward_id;
                    for (int i = 0; i < ward_count; i++)
                    {
                        if (strcmp(ward_stays[i].ward_id, h->ward_id) == 0)
                        {
                            if (ward_stays[i].avg_days > 0)
                                ward_avg = ward_stays[i].avg_days;
                            ward_name = ward_stays[i].name;
                            break;
                        }
                    }

                    /* 查找患者姓名 */
                    Patient *pat = find_patient_by_p_id(p_head, h->p_id);
                    const char *p_name = pat ? pat->name : h->p_id;

                    strncpy(alerts[alert_idx].p_id, h->p_id, MAX_ID_LEN - 1);
                    strncpy(alerts[alert_idx].p_name, p_name, MAX_NAME_LEN - 1);
                    strncpy(alerts[alert_idx].ward_name, ward_name, MAX_NAME_LEN - 1);
                    alerts[alert_idx].stayed_days = stayed;
                    alerts[alert_idx].ward_avg = ward_avg;
                    alerts[alert_idx].is_overdue = (stayed > ward_avg * ANALYTICS_OVERDUE_MULTIPLIER) ? 1 : 0;
                    alert_idx++;
                }

                int pid_w, aname_w, award_w, astayed_w, aavg_w, astatus_w;
                calc_inpatient_alert_width(alerts, alert_idx, &pid_w, &aname_w, &award_w, &astayed_w, &aavg_w,
                                           &astatus_w);
                print_inpatient_alert_header(pid_w, aname_w, award_w, astayed_w, aavg_w, astatus_w);
                for (int i = 0; i < alert_idx; i++)
                    print_inpatient_alert_line(&alerts[i], pid_w, aname_w, award_w, astayed_w, aavg_w, astatus_w);
                int alert_widths[] = {pid_w, aname_w, award_w, astayed_w, aavg_w, astatus_w};
                print_analytics_line(6, alert_widths);

                printf("  状态说明: ⚠ 超期 = 已住天数超过该病房平均值2倍以上\n");
                printf("            ✓ 正常 = 住院天数在合理范围内\n");

                free(alerts);
            }
        }
        else
        {
            printf("  当前无在院患者。\n");
        }

        free(ward_stays);
    }

    /* 释放资源 */
    free_hospitalizations(h_head);
    if (w_head)
        free_wards(w_head);
    if (p_head)
        free_patients(p_head);
    wait_enter();
}
