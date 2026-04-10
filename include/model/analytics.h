/*
 * 数据分析模块
 */
#ifndef ANALYTICS_H
#define ANALYTICS_H

#include "core/config.h"
#include "core/structs.h"

/*
 * 渲染工具
 */
void render_bar(float ratio, int width, char *buf);             // 进度条 [████░░░░]
void render_hbar(int value, int max_val, int width, char *buf); // 柱状图 ████████

/*
 * 五大分析入口
 */
void analytics_ward_utilization(void);    // 病房利用率分析
void analytics_department_workload(void); // 科室门诊量与趋势分析
void analytics_ward_optimization(void);   // 住院分析与病房优化
void analytics_drug_usage(void);          // 药品使用分析
void analytics_stay_duration(void);       // 住院时长分布与预测

#endif
