/*
 * 工具函数模块
 */
#include "core/utils.h"
#include <errno.h>

/*
 * 工具函数实现
 */

/* 安全读取一行输入并保证缓冲区无残留 */
void safe_input(char *str, int size)
{
    if (fgets(str, size, stdin) != NULL) // 从标准输入读取一行，最多size-1个字符，剩余一个位置留给'\0'
    {
        size_t len = strlen(str); // 计算实际读取的字符串长度

        if (len > 0 && str[len - 1] == '\n') // 如果末尾是换行符
        {
            str[len - 1] = '\0'; // 替换换行符为字符串结束符
        }
        else
        {
            int c; // 如果没有读取到换行符，说明输入过长，清空输入缓冲区
            while ((c = getchar()) != '\n' && c != EOF)
            {
            }
        }
    }
    else
    {
        str[0] = '\0'; // 读取失败时返回空字符串
    }

    /* 过滤管道符'|'，防止破坏文件分隔格式 */
    char *r = str, *w = str; // 定义读写指针，初始都指向字符串开头
    while (*r)
    {
        if (*r != '|') // 只复制非管道符的字符
            *w++ = *r;
        r++;
    }
    *w = '\0';
}

/* 清屏(多系统) */
void clear_screen(void)
{
#ifdef _WIN32
    system("cls"); // Windows系统使用cls命令清屏
#else
    system("clear"); // Unix/Linux/Mac系统使用clear命令清屏
#endif
}

/* 去掉字符串末尾换行(\r\n) */
void trim_newline(char *s)
{
    if (!s) // 空指针防御
        return;
    s[strcspn(s, "\r\n")] = '\0'; // strcspn返回字符串中首次出现\r或\n的位置，这里把它替换为字符串结束符
}

/* 提示并等待回车 */
void wait_enter(void)
{
    printf("\n按回车键继续...");
    getchar(); // 等待用户按下回车键
}

/* 生成随机盐 */
void generate_salt(unsigned char *salt, int length)
{
    static int seeded = 0; // 静态变量确保只初始化一次随机数生成器
    if (!seeded)
    {
        srand((unsigned int)time(NULL)); // 仅初始化一次
        seeded = 1;
    }

    for (int i = 0; i < length; i++)
    {
        salt[i] = (unsigned char)(rand() & 0xFF); // 生成0~255之间的随机字节
    }
}

/* 校验姓名：仅允许汉字（UTF-8常用汉字区） */
int is_all_chinese_utf8(const char *s)
{
    /* 空值与空串防御 */
    if (!s || *s == '\0')
        return 0;

    const unsigned char *p = (const unsigned char *)s; // 定义一个指向输入字符串的无符号字符指针，方便处理UTF-8字节
    while (*p)
    {
        /* 常见汉字UTF-8三字节：E4~E9 开头 */
        if (p[0] >= 0xE4 && p[0] <= 0xE9)
        {
            if (p[1] && p[2] && p[1] >= 0x80 && p[1] <= 0xBF && p[2] >= 0x80 && p[2] <= 0xBF)
            {
                p += 3;
                continue;
            }
        }
        return 0; // 只要出现非汉字就失败
    }
    return 1;
}

/* 校验有效输入：支持多位数字，范围0~max */
int validate_choice(const char *str, int max)
{
    /* 空值与空串防御 */
    if (!str || str[0] == '\0')
        return 0;

    /* 拒绝前导零(允许单独输入一个 "0") */
    if (str[0] == '0' && str[1] != '\0')
        return 0;

    /* 确保全为数字(拒绝负号、小数点、空格等) */
    for (int i = 0; str[i] != '\0'; i++)
    {
        if (str[i] < '0' || str[i] > '9')
            return 0;
    }

    /* 安全地转换为 long */
    errno = 0;
    char *endptr;
    long val =
        strtol(str, &endptr, 10); // 把十进制字符串转换成long，如果溢出会设置errno为ERANGE，而atoi无法准确检测错误

    /* 检查是否发生整数溢出 */
    if (errno == ERANGE)
        return 0;

    /* 校验上限 */
    if (val > max)
        return 0;

    return 1;
}

/* 将字节数组转为十六进制字符串 */
void bytes_to_hex(const unsigned char *in, int len, char *out)
{
    for (int i = 0; i < len; i++)
        snprintf(out + i * 2, 3, "%02x", in[i]);
    out[len * 2] = '\0';
}

/* 计算字符串显示宽度 */
int str_width(const char *s)
{
    int width = 0;
    while (*s)
    {
        unsigned char c0 = (unsigned char)s[0];

        if (c0 < 0x80)
        {
            /* ASCII: 1字节，宽度1 */
            width += 1;
            s += 1;
        }
        else if (c0 < 0xC0)
        {
            /* 孤立后续字节(异常数据)，跳过 */
            s += 1;
        }
        else if (c0 < 0xE0)
        {
            /* 2字节序列：默认宽度1 */
            width += 1;

            int step = 1; // 实际字节数，默认为1

            if ((unsigned char)s[1] >= 0x80 && (unsigned char)s[1] <= 0xBF) // 如果第二字节合法，确认为2字节序列
                step = 2;

            s += step;
        }
        else if (c0 < 0xF0)
        {
            /* 3字节序列：默认宽度2（中日韩等） */
            int w = 2;

            unsigned char c1 = (unsigned char)s[1];
            unsigned char c2 = (unsigned char)s[2];

            /* 常用单列符号特例 */
            if (
                /* ↑ U+2191 */ (c0 == 0xE2 && c1 == 0x86 && c2 == 0x91) ||
                /* → U+2192 */ (c0 == 0xE2 && c1 == 0x86 && c2 == 0x92) ||
                /* ↓ U+2193 */ (c0 == 0xE2 && c1 == 0x86 && c2 == 0x93) ||
                /* ✓ U+2713 */ (c0 == 0xE2 && c1 == 0x9C && c2 == 0x93) ||
                /* ⚠ U+26A0 */ (c0 == 0xE2 && c1 == 0x9A && c2 == 0xA0))
            {
                w = 1;
            }

            width += w;

            int step = 1; // 实际字节数，默认为1

            if ((unsigned char)s[1] >= 0x80 && (unsigned char)s[1] <= 0xBF) // 如果第二字节合法，至少是2字节序列
            {
                step = 2;
                if ((unsigned char)s[2] >= 0x80 && (unsigned char)s[2] <= 0xBF) // 如果第三字节也合法，确认为3字节序列
                    step = 3;
            }

            s += step;
        }
        else
        {
            /* 4字节序列(emoji等)：默认宽度2 */
            width += 2;

            int step = 1; // 实际字节数，默认为1

            if ((unsigned char)s[1] >= 0x80 && (unsigned char)s[1] <= 0xBF) // 如果第二字节合法，至少是2字节序列
            {
                step = 2;
                if ((unsigned char)s[2] >= 0x80 && (unsigned char)s[2] <= 0xBF) // 如果第三字节也合法，至少是3字节序列
                {
                    step = 3;
                    if ((unsigned char)s[3] >= 0x80 &&
                        (unsigned char)s[3] <= 0xBF) // 如果第四字节也合法，确认为4字节序列
                        step = 4;
                }
            }

            s += step;
        }
    }
    return width;
}

/* 格式化时间戳为北京时间字符串 */
int format_beijing_time(time_t t, char *buf, size_t buf_size)
{
    if (!buf || buf_size < 20)
        return 0;

    time_t bj = t + 8 * 3600;     // UTC+8
    struct tm *ptm = gmtime(&bj); // 把北京时间当成UTC时间来格式化，把时间戳拆分

    if (!ptm) // 时间无效时gmtime返回NULL
        return 0;

    strftime(buf, buf_size, "%Y-%m-%d %H:%M:%S", ptm); // 按模板格式化时间字符串
    return 1;
}

/* 左对齐打印字符串 */
void print_align(const char *s, int width)
{
    int w = str_width(s); // 计算字符串显示宽度

    printf("%s", s); // 先输出字符串

    for (int i = w; i < width; i++) // 再输出足够的空格来填充到指定宽度
        printf(" ");
}

/* 打开临时文件用于安全写入，写完后用 safe_fclose_commit 提交 */
FILE *safe_fopen_tmp(const char *final_path, char *tmp_path, size_t tmp_size)
{
    snprintf(tmp_path, tmp_size, "%s.tmp", final_path); // 把目标路径变为xxx.txt.tmp
    return fopen(tmp_path, "w");                        // 打开临时文件写入
}

/* 关闭临时文件并原子替换目标文件；失败时删除临时文件并返回-1 */
int safe_fclose_commit(FILE *fp, const char *tmp_path, const char *final_path)
{
    if (fflush(fp) != 0) // 确保数据写入磁盘
    {
        fclose(fp);
        remove(tmp_path); // 如果失败就删除临时文件
        return -1;
    }
    if (fclose(fp) != 0) // 确保文件正确关闭
    {
        remove(tmp_path); // 如果失败就删除临时文件
        return -1;
    }

    /* rename 在同一文件系统上是原子操作；Windows上需先删目标 */
#ifdef _WIN32
    remove(final_path); // Windows不允许直接覆盖现有文件，所以先删除目标文件
#endif
    if (rename(tmp_path, final_path) != 0) // 原子替换目标文件，成功返回0，失败返回-1
    {
        remove(tmp_path); // 如果失败就删除临时文件
        return -1;
    }
    return 0;
}

/* 渲染进度条 [████░░░░] */
void render_bar(float ratio, int width, char *buf)
{
    if (!buf) // 输出缓冲区必须存在
        return;

    if (width < 1) // 宽度必须至少为1
    {
        buf[0] = '\0';
        return;
    }

    if (ratio < 0.0f) // 比例不能为负
        ratio = 0.0f;

    if (ratio > 1.0f) // 比例不能超过1
        ratio = 1.0f;

    int filled = (int)(ratio * width + 0.5f); // 四舍五入

    if (filled > width) // 填充数不能超过宽度
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

/* 渲染柱状图 ████████ */
void render_hbar(int value, int max_val, int width, char *buf)
{
    if (!buf) // 输出缓冲区必须存在
        return;

    if (width < 1) // 宽度必须至少为1
    {
        buf[0] = '\0';
        return;
    }

    int filled = 0;
    if (max_val > 0 && value > 0) // 避免除以零，并且只有当有值时才显示柱状图
    {
        /* 按比例计算填充数, 四舍五入 */
        filled = (int)((double)value / max_val * width + 0.5);
        if (filled > width) // 填充数不能超过宽度
            filled = width;
        /* 有值但舍入为0时, 至少显示1格 */
        if (filled == 0) // 避免有值但显示空白的情况
            filled = 1;
    }

    char *p = buf;
    for (int i = 0; i < filled; i++)
    {
        *p++ = (char)0xE2;
        *p++ = (char)0x96;
        *p++ = (char)0x88; /* █ U+2588 */
    }
    for (int i = filled; i < width; i++)
    {
        *p++ = ' '; // 空格填充剩余部分
    }
    *p = '\0';
}