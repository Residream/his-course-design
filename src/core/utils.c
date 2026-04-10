/*
 * 工具函数模块
 */
#include "core/utils.h"

/*
 * 工具函数实现
 */

/* 安全读取一行输入并保证缓冲区无残留 */
void safe_input(char *str, int size)
{
    if (fgets(str, size, stdin) != NULL)
    {
        size_t len = strlen(str);

        if (len > 0 && str[len - 1] == '\n')
        {
            str[len - 1] = '\0';
        }
        else
        {
            int c;
            while ((c = getchar()) != '\n' && c != EOF)
            {
            }
        }
    }
    else
    {
        str[0] = '\0';
    }

    /* 过滤管道符'|'，防止破坏文件分隔格式 */
    char *r = str, *w = str;
    while (*r)
    {
        if (*r != '|')
            *w++ = *r;
        r++;
    }
    *w = '\0';
}

/* 清屏(多系统) */
void clear_screen(void)
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

/* 去掉字符串末尾换行(\r\n) */
void trim_newline(char *s)
{
    if (!s)
        return;
    s[strcspn(s, "\r\n")] = '\0';
}

/* 提示并等待回车 */
void wait_enter(void)
{
    printf("\n按回车键继续...");
    getchar();
}

/* 生成随机盐 */
void generate_salt(unsigned char *salt, int length)
{
    static int seeded = 0;
    if (!seeded)
    {
        srand((unsigned int)time(NULL)); // 仅初始化一次
        seeded = 1;
    }

    for (int i = 0; i < length; i++)
    {
        salt[i] = (unsigned char)(rand() & 0xFF);
    }
}

/* 校验姓名：仅允许汉字（UTF-8常用汉字区） */
int is_all_chinese_utf8(const char *s)
{
    if (!s || *s == '\0')
        return 0;

    const unsigned char *p = (const unsigned char *)s;
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
    if (!str || str[0] == '\0')
        return 0;

    /* 检查是否全为数字 */
    for (int i = 0; str[i] != '\0'; i++)
    {
        if (str[i] < '0' || str[i] > '9')
            return 0;
    }

    char *endptr;
    long val = strtol(str, &endptr, 10);
    if (*endptr != '\0' || val < 0 || val > max)
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
        unsigned char c = (unsigned char)*s;
        if (c < 0x80)
        {
            /* ASCII: 1字节，宽度1 */
            width += 1;
            s += 1;
        }
        else if (c < 0xC0)
        {
            /* 孤立的后续字节(数据损坏)，跳过1字节 */
            s += 1;
        }
        else if (c < 0xE0)
        {
            /* 2字节序列(拉丁扩展等)，宽度1 */
            width += 1;
            s += ((unsigned char)s[1] >= 0x80 && (unsigned char)s[1] <= 0xBF) ? 2 : 1;
        }
        else if (c < 0xF0)
        {
            /* 3字节序列(中日韩汉字等)，宽度2 */
            width += 2;
            int step = 1;
            if ((unsigned char)s[1] >= 0x80 && (unsigned char)s[1] <= 0xBF)
            {
                step = 2;
                if ((unsigned char)s[2] >= 0x80 && (unsigned char)s[2] <= 0xBF)
                    step = 3;
            }
            s += step;
        }
        else
        {
            /* 4字节序列(emoji等)，宽度2 */
            width += 2;
            int step = 1;
            if ((unsigned char)s[1] >= 0x80 && (unsigned char)s[1] <= 0xBF)
            {
                step = 2;
                if ((unsigned char)s[2] >= 0x80 && (unsigned char)s[2] <= 0xBF)
                {
                    step = 3;
                    if ((unsigned char)s[3] >= 0x80 && (unsigned char)s[3] <= 0xBF)
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

    time_t bj = t + 8 * 3600; // UTC+8
    struct tm *ptm = gmtime(&bj);
    if (!ptm)
        return 0;

    strftime(buf, buf_size, "%Y-%m-%d %H:%M:%S", ptm);
    return 1;
}

/* 打印对齐的字符串 */
void print_align(const char *s, int width)
{
    int w = str_width(s);
    printf("%s", s);

    for (int i = w; i < width; i++)
        printf(" ");
}

/* 打开临时文件用于安全写入，写完后用 safe_fclose_commit 提交 */
FILE *safe_fopen_tmp(const char *final_path, char *tmp_path, size_t tmp_size)
{
    snprintf(tmp_path, tmp_size, "%s.tmp", final_path);
    return fopen(tmp_path, "w");
}

/* 关闭临时文件并原子替换目标文件；失败时删除临时文件并返回-1 */
int safe_fclose_commit(FILE *fp, const char *tmp_path, const char *final_path)
{
    if (fflush(fp) != 0)
    {
        fclose(fp);
        remove(tmp_path);
        return -1;
    }
    if (fclose(fp) != 0)
    {
        remove(tmp_path);
        return -1;
    }

    /* rename 在同一文件系统上是原子操作；Windows上需先删目标 */
#ifdef _WIN32
    remove(final_path);
#endif
    if (rename(tmp_path, final_path) != 0)
    {
        remove(tmp_path);
        return -1;
    }
    return 0;
}

/* 渲染进度条 [████░░░░] */
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

/* 渲染柱状图 ████████ */
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
    for (int i = filled; i < width; i++)
    {
        *p++ = ' ';
    }
    *p = '\0';
}