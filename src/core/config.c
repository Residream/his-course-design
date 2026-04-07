/*
 * 全局配置宏
 */
#include "core/config.h"

const char *REG_STATUS_TEXT[REG_STATUS_COUNT] = {
    "未就诊",
    "已就诊",
    "已取消",
};

const char *VISIT_STATUS_TEXT[VISIT_STATUS_COUNT] = {
    "看诊中",
    "已完成",
};

const char *BED_STATUS_TEXT[BED_STATUS_COUNT] = {
    "空闲",
    "已占用",
};

const char *HOSP_STATUS_TEXT[HOSP_STATUS_COUNT] = {
    "住院中",
    "已出院",
};