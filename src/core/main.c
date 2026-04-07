/*
 * 主函数
 */
#include "ui/menu.h"

int main(void)
{
    session_clear(); // 确保会话状态初始为未登录
    login_menu();
    return 0;
}
