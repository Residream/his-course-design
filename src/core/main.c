/*
 * 主函数
 */
#include "ui/menu.h"

int main(void)
{
    session_clear(); // 确保会话状态初始为未登录
    login_menu();    // 进入登录界面，登录成功后会进入主菜单
    return 0;
}
