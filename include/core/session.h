/*
 * 会话管理模块
 */
#ifndef SESSION_H
#define SESSION_H

#include "core/config.h"

typedef struct
{
    int logged_in;
    char role[16];            // "patient" / "doctor"
    char user_id[MAX_ID_LEN]; // 患者/医生ID；管理员可存用户名
} Session;

extern Session g_session;

/* 设置会话信息 */
void session_set(const char *role, const char *user_id);
/* 清除会话信息 */
void session_clear(void);

#endif