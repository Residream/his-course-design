/*
 * 会话管理模块
 */
#include <string.h>

#include "core/session.h"

Session g_session = {0, "", ""}; // 全局会话变量，初始为未登录状态

/* 设置会话信息 */
void session_set(const char *role, const char *user_id)
{
    g_session.logged_in = 1; // 标记为已登录

    strncpy(g_session.role, role, sizeof(g_session.role) - 1); // 安全复制角色字符串
    g_session.role[sizeof(g_session.role) - 1] = '\0';         // 字符串以'\0'结尾

    strncpy(g_session.user_id, user_id, sizeof(g_session.user_id) - 1); // 安全复制用户ID字符串
    g_session.user_id[sizeof(g_session.user_id) - 1] = '\0';            // 字符串以'\0'结尾
}

/* 清除会话信息 */
void session_clear(void)
{
    g_session.logged_in = 0;     // 标记为未登录
    g_session.role[0] = '\0';    // 清空角色字符串
    g_session.user_id[0] = '\0'; // 清空用户ID字符串
}