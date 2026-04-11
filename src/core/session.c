/*
 * 会话管理模块
 */
#include <string.h>

#include "core/session.h"

Session g_session = {0, "", ""};

/* 设置会话信息 */
void session_set(const char *role, const char *user_id)
{
    g_session.logged_in = 1;

    strncpy(g_session.role, role, sizeof(g_session.role) - 1);
    g_session.role[sizeof(g_session.role) - 1] = '\0';

    strncpy(g_session.user_id, user_id, sizeof(g_session.user_id) - 1);
    g_session.user_id[sizeof(g_session.user_id) - 1] = '\0';
}

/* 清除会话信息 */
void session_clear(void)
{
    g_session.logged_in = 0;
    g_session.role[0] = '\0';
    g_session.user_id[0] = '\0';
}