/*
 * 工具函数模块
 */
#ifndef utils_h
#define utils_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "core/config.h"

/*
 * 工具函数声明
 */
void safe_input(char *buf, int size);                           // 安全读取一行输入
void clear_screen(void);                                        // 清屏(多系统)
void trim_newline(char *s);                                     // 去掉字符串末尾换行(\r\n)
void wait_enter(void);                                          // 提示并等待回车
void generate_salt(unsigned char *salt, int length);            // 生成随机盐
int is_all_chinese_utf8(const char *s);                         // 校验姓名：仅允许汉字（UTF-8常用汉字区）
int validate_choice(const char *str, int max);                  // 校验有效输入
void bytes_to_hex(const unsigned char *in, int len, char *out); // 将字节数组转为十六进制字符串
int str_width(const char *s);                                   // 计算字符串显示宽度头
int format_beijing_time(time_t t, char *buf, size_t buf_size);  // 格式化时间戳为北京时间字符串
void print_align(const char *s, int width);                     // 打印对齐的字符串

FILE *safe_fopen_tmp(const char *final_path, char *tmp_path, size_t tmp_size);  // 打开临时文件用于安全写入
int safe_fclose_commit(FILE *fp, const char *tmp_path, const char *final_path); // 关闭并原子提交(rename)

void render_bar(float ratio, int width, char *buf);             // 渲染进度条 [████░░░░]
void render_hbar(int value, int max_val, int width, char *buf); // 渲染柱状图 ████████

#endif