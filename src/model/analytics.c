/*
 * 数据分析模块
 */
#include "model/analytics.h"

/*
 * 渲染工具
 */

/* 进度条 [████░░░░] */
void render_bar(float ratio, int width, char *buf)
{
    if (!buf)
        return;
    if (width < 1)
    {
        buf[0] = '\0';
        return;
    }
    if (ratio < 0.0f)
        ratio = 0.0f;
    if (ratio > 1.0f)
        ratio = 1.0f;

    int filled = (int)(ratio * width + 0.5f); // 四舍五入
    if (filled > width)
        filled = width;

    char *p = buf;
    *p++ = '[';
    for (int i = 0; i < filled; i++)
    {
        *p++ = (char)0xE2;
        *p++ = (char)0x96;
        *p++ = (char)0x88; /* █ U+2588 */
    }
    for (int i = filled; i < width; i++)
    {
        *p++ = (char)0xE2;
        *p++ = (char)0x96;
        *p++ = (char)0x91; /* ░ U+2591 */
    }
    *p++ = ']';
    *p = '\0';
}

/* 柱状图 ████████ */
void render_hbar(int value, int max_val, int width, char *buf)
{
    if (!buf)
        return;
    if (width < 1)
    {
        buf[0] = '\0';
        return;
    }

    int filled = 0;
    if (max_val > 0 && value > 0)
    {
        /* 按比例计算填充数, 四舍五入 */
        filled = (int)((double)value / max_val * width + 0.5);
        if (filled > width)
            filled = width;
        /* 有值但舍入为0时, 至少显示1格 */
        if (filled == 0)
            filled = 1;
    }

    char *p = buf;
    for (int i = 0; i < filled; i++)
    {
        *p++ = (char)0xE2;
        *p++ = (char)0x96;
        *p++ = (char)0x88; /* █ U+2588 */
    }
    *p = '\0';
}

/*
 * 五大分析入口
 */

/* 病房利用率分析 */
void analytics_ward_utilization(void)
{
}

/* 科室门诊量与趋势分析 */
void analytics_department_workload(void)
{
}

/* 住院分析与病房优化 */
void analytics_ward_optimization(void)
{
}

/* 药品使用分析 */
void analytics_drug_usage(void)
{
}

/* 住院时长分布与预测 */
void analytics_stay_duration(void)
{
}