/*
 * 数据分析模块
 */
#ifndef ANALYTICS_H
#define ANALYTICS_H

#include "core/structs.h"

/*
 * 通用渲染常量
 */
#define ANALYTICS_SECONDS_PER_DAY 86400 // 一天的秒数, 用于时间戳转天数
#define ANALYTICS_BAR_WIDTH 14          // 表格内图表主体宽度(格子数)
#define ANALYTICS_SUMMARY_BAR_WIDTH 10  // 汇总行短进度条宽度
#define ANALYTICS_BAR_DISPLAY_WIDTH 16  // 带方括号的进度条最小列宽: [+BAR_WIDTH+]
#define ANALYTICS_BAR_BUF_SIZE 256      // 渲染进度条/柱状图的缓冲区大小

/*
 * 表格列宽下限
 */
#define ANALYTICS_RATE_COL_MIN_WIDTH 6    // 百分比列最小宽度, 容纳 "100.0%"
#define ANALYTICS_STATUS_COL_MIN_WIDTH 10 // 状态列最小宽度, 容纳 "⚠ 补货" 等
#define ANALYTICS_TREND_COL_MIN_WIDTH 10  // 趋势列最小宽度, 容纳 "↑ +100%"

/*
 * 聚合上限
 */
#define ANALYTICS_MAX_DEPT_COUNT 20  // 单次分析支持的最大科室数
#define ANALYTICS_MAX_WARD_COUNT 20  // 单次分析支持的最大病房数
#define ANALYTICS_MAX_DRUG_COUNT 100 // 单次分析支持的最大药品数

/*
 * 门诊趋势
 */
#define ANALYTICS_TREND_WINDOW_DAYS 30 // 门诊趋势对比的时间窗口(天)
#define ANALYTICS_TREND_THRESHOLD 5.0f // ±5% 以内显示 "持平 →"

/*
 * 病房优化阈值
 */
#define ANALYTICS_LOW_UTIL_THRESHOLD 0.30f // 利用率低于此值 → 建议缩减/合并
#define ANALYTICS_DEMAND_BED_GAP 0.10f     // 需求占比与床位占比差超过此值 → 建议扩容
#define ANALYTICS_MULTI_DEPT_THRESHOLD 3   // 综合病房被N个及以上科室共用 → 建议作为缓冲病区
#define ANALYTICS_OVERDUE_MULTIPLIER 2.0f  // 在院天数超过病房均值的倍数即判定为超期

/*
 * 药品库存预警
 */
#define ANALYTICS_STOCK_WARN_CRITICAL 3.0f // 可用不足3月需立即补货
#define ANALYTICS_STOCK_WARN_WATCH 6.0f    // 可用不足6月建议关注
#define ANALYTICS_DATA_WINDOW_MONTHS 3.0f  // 生成数据跨度(与 gen_data.py 的90天保持一致)

/*
 * 排行榜与分桶
 */
#define ANALYTICS_DOCTOR_TOP_N 5      // 医生接诊量排行前N
#define ANALYTICS_DRUG_TOP_N 10       // 药品使用排行前N
#define ANALYTICS_STAY_BUCKET_COUNT 5 // 住院天数分桶数量

/*
 * 五大分析入口
 */
void analytics_ward_utilization(void);         // 病房利用率分析
void analytics_department_workload(void);      // 科室门诊量与趋势分析
void analytics_ward_optimization(void);        // 住院分析与病房优化
void analytics_drug_usage(void);               // 药品使用分析
void analytics_hospitalization_duration(void); // 住院时长分布与预测

#endif
