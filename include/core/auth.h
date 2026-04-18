/*
 * 登录验证模块
 */
#ifndef AUTH_H
#define AUTH_H

#include "core/config.h"

/* 验证输入密码是否正确 */
int verify_password(const char *input_password, const char *stored_salt_hex, const char *stored_hash_hex);

/* 对输入password加盐后加密 */
void build_pass_hash(const unsigned char salt[SALT_RAW_LEN], const char *password, char out_hex[65]);

/* 管理员登录验证 */
int admin_login_by_file(const char *file, const char *name, const char *password);

/* 患者登录验证 */
int patient_login_by_file(const char *file, const char *id, const char *password);

/* 医生登录验证 */
int doctor_login_by_file(const char *file, const char *id, const char *password);

/* 患者注册 */
void patient_register(void);

#endif