/*
 * 登录验证模块
 */
#include "core/auth.h"
#include "core/sha256.h"
#include "core/utils.h"
#include "model/patient.h"

/* 单个十六进制字符串转十六进制 */
static int hex_char_to_val(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return -1;
}

/* 十六进制字符串转字节数组 */
static int hex_to_bytes(const char *hex, unsigned char *out, int out_len)
{
    int n = (int)strlen(hex);
    if (n != out_len * 2)
        return 0;

    for (int i = 0; i < out_len; i++)
    {
        int hi = hex_char_to_val(hex[i * 2]);
        int lo = hex_char_to_val(hex[i * 2 + 1]);
        if (hi < 0 || lo < 0)
            return 0;
        out[i] = (unsigned char)((hi << 4) | lo);
    }
    return 1;
}

/* 校验输入密码是否与存储的 hash/salt 匹配 */
int verify_password(const char *input_password, const char *stored_salt_hex, const char *stored_hash_hex)
{
    if (!input_password || !stored_salt_hex || !stored_hash_hex)
        return 0;

    unsigned char salt[SALT_RAW_LEN];
    if (!hex_to_bytes(stored_salt_hex, salt, SALT_RAW_LEN))
        return 0;

    char computed_hash[MAX_PWD_HASH];
    build_pass_hash(salt, input_password, computed_hash);

    return strcmp(computed_hash, stored_hash_hex) == 0;
}

/* 对输入password加盐后加密 */
void build_pass_hash(const unsigned char salt[SALT_RAW_LEN], const char *password, char out_hex[65])
{
    uint8_t buf[SALT_RAW_LEN + MAX_INPUT_LEN];
    size_t pass_len = strlen(password);
    if (pass_len > MAX_INPUT_LEN)
        pass_len = MAX_INPUT_LEN;

    memcpy(buf, salt, SALT_RAW_LEN);
    memcpy(buf + SALT_RAW_LEN, password, pass_len);

    uint8_t hash[32];
    sha256(buf, SALT_RAW_LEN + pass_len, hash);
    sha256_to_hex(hash, out_hex);
}

/* 管理员登录验证 */
int admin_login_by_file(const char *file, const char *name, const char *password)
{
    FILE *fp = fopen(file, "r");
    if (!fp)
        return 0;

    char line[MAX_LINE_LEN];
    while (fgets(line, sizeof(line), fp))
    {
        trim_newline(line);
        if (!line[0])
            continue;

        char *token = strtok(line, "|"); // 读取name
        if (!token)
            continue;
        if (strcmp(token, name) != 0) // 如果name不匹配，继续下一行
            continue;

        token = strtok(NULL, "|"); // 读取pwd_hash
        if (!token)
            continue;
        char stored_hash[MAX_PWD_HASH];
        strncpy(stored_hash, token, sizeof(stored_hash) - 1);
        stored_hash[sizeof(stored_hash) - 1] = '\0';

        token = strtok(NULL, "|"); // 读取salt
        if (!token)
            continue;
        char stored_salt[SALT_HEX_LEN];
        strncpy(stored_salt, token, sizeof(stored_salt) - 1);
        stored_salt[sizeof(stored_salt) - 1] = '\0';

        fclose(fp);
        return verify_password(password, stored_salt, stored_hash); // 如果哈希值匹配，登录成功
    }

    fclose(fp);
    return 0;
}

/* 患者登录验证 */
int patient_login_by_file(const char *file, const char *id, const char *password)
{
    FILE *fp = fopen(file, "r");
    if (!fp)
        return 0;

    char line[MAX_LINE_LEN];
    while (fgets(line, sizeof(line), fp))
    {
        trim_newline(line);
        if (!line[0])
            continue;

        char *token = strtok(line, "|"); // 读取id
        if (!token)
            continue;
        if (strcmp(token, id) != 0) // 如果id不匹配，继续下一行
            continue;

        token = strtok(NULL, "|"); // 跳过name
        if (!token)
            continue;
        token = strtok(NULL, "|"); // 跳过gender
        if (!token)
            continue;
        token = strtok(NULL, "|"); // 跳过age
        if (!token)
            continue;

        token = strtok(NULL, "|"); // 读取pwd_hash
        if (!token)
            continue;
        char stored_hash[MAX_PWD_HASH];
        strncpy(stored_hash, token, sizeof(stored_hash) - 1);
        stored_hash[sizeof(stored_hash) - 1] = '\0';

        token = strtok(NULL, "|"); // 读取salt
        if (!token)
            continue;
        char stored_salt[SALT_HEX_LEN];
        strncpy(stored_salt, token, sizeof(stored_salt) - 1);
        stored_salt[sizeof(stored_salt) - 1] = '\0';

        fclose(fp);
        return verify_password(password, stored_salt, stored_hash); // 如果哈希值匹配，登录成功
    }

    fclose(fp);
    return 0;
}

/* 医生登录验证 */
int doctor_login_by_file(const char *file, const char *id, const char *password)
{
    FILE *fp = fopen(file, "r");
    if (!fp)
        return 0;

    char line[MAX_LINE_LEN];
    while (fgets(line, sizeof(line), fp))
    {
        trim_newline(line);
        if (!line[0])
            continue;

        char *token = strtok(line, "|"); // 读取id
        if (!token)
            continue;
        if (strcmp(token, id) != 0) // 如果id不匹配，继续下一行
            continue;

        token = strtok(NULL, "|"); // 跳过name
        if (!token)
            continue;
        token = strtok(NULL, "|"); // 跳过gender
        if (!token)
            continue;
        token = strtok(NULL, "|"); // 跳过department
        if (!token)
            continue;

        token = strtok(NULL, "|"); // 读取pwd_hash
        if (!token)
            continue;
        char stored_hash[MAX_PWD_HASH];
        strncpy(stored_hash, token, sizeof(stored_hash) - 1);
        stored_hash[sizeof(stored_hash) - 1] = '\0';

        token = strtok(NULL, "|"); // 读取salt
        if (!token)
            continue;
        char stored_salt[SALT_HEX_LEN];
        strncpy(stored_salt, token, sizeof(stored_salt) - 1);
        stored_salt[sizeof(stored_salt) - 1] = '\0';

        fclose(fp);
        return verify_password(password, stored_salt, stored_hash); // 如果哈希值匹配，登录成功
    }

    fclose(fp);
    return 0;
}

/* 患者注册 */
void patient_register()
{
    char name[MAX_NAME_LEN];
    char gender[MAX_GENDER_LEN];
    char password[MAX_INPUT_LEN];
    int age = 0;

    /* 姓名 */
    while (1)
    {
        printf("请输入患者姓名(汉字 | 输入0返回): ");
        safe_input(name, sizeof(name));

        if (strcmp(name, "0") == 0)
            return;

        if (name[0] == '\0')
        {
            printf("输入错误！姓名不能为空，请重新输入。\n");
            continue;
        }
        if (!is_all_chinese_utf8(name))
        {
            printf("输入错误！姓名只能包含汉字，请重新输入。\n");
            continue;
        }
        break;
    }

    /* 性别 */
    while (1)
    {
        printf("请选择患者性别(男/女 | 输入0返回): ");
        safe_input(gender, sizeof(gender));

        if (strcmp(gender, "0") == 0)
            return;

        if (gender[0] == '\0')
        {
            printf("输入错误！性别不能为空，请重新输入。\n");
            continue;
        }
        if (strcmp(gender, "男") != 0 && strcmp(gender, "女") != 0)
        {
            printf("输入错误！请输入'男'或'女'。\n");
            continue;
        }
        break;
    }

    /* 年龄 */
    while (1)
    {
        char age_buf[MAX_INPUT_LEN];
        int start = 0;
        int is_number = 1;

        printf("请输入患者年龄(1-150 | 输入0返回): ");
        safe_input(age_buf, sizeof(age_buf));

        if (strcmp(age_buf, "0") == 0)
            return;

        if (age_buf[0] == '\0')
        {
            printf("输入错误！年龄不能为空，请重新输入。\n");
            continue;
        }

        /* 允许首字符为正负号 */
        if (age_buf[0] == '-' || age_buf[0] == '+')
            start = 1;

        if (age_buf[start] == '\0')
        {
            is_number = 0; /* 只有符号没有数字 */
        }
        else
        {
            for (int i = start; age_buf[i] != '\0'; i++)
            {
                if (age_buf[i] < '0' || age_buf[i] > '9')
                {
                    is_number = 0;
                    break;
                }
            }
        }

        if (!is_number)
        {
            printf("输入错误！年龄只能输入数字，请重新输入。\n");
            continue;
        }

        age = atoi(age_buf);
        if (age < 1 || age > 150)
        {
            printf("输入错误！年龄范围必须在1-150之间，请重新输入。\n");
            continue;
        }

        break;
    }

    /* 密码 */
    while (1)
    {
        printf("请输入患者密码(输入0返回): ");
        safe_input(password, sizeof(password));

        if (strcmp(password, "0") == 0)
            return;

        if (password[0] == '\0')
        {
            printf("输入错误！密码不能为空，请重新输入。\n");
            continue;
        }
        break;
    }

    /* 只加载一次 */
    Patient *patient_head = load_patients_from_file();

    char id[MAX_ID_LEN];
    int next_num = generate_next_patient_id(patient_head);
    snprintf(id, sizeof(id), "P%04d", next_num);

    /* 生成盐与哈希 */
    unsigned char salt[SALT_RAW_LEN];
    char salt_hex[SALT_HEX_LEN];
    char pwd_hash[MAX_PWD_HASH];

    generate_salt(salt, SALT_RAW_LEN);
    bytes_to_hex(salt, SALT_RAW_LEN, salt_hex);
    build_pass_hash(salt, password, pwd_hash);

    /* 动态分配新节点 */
    Patient *new_node = (Patient *)malloc(sizeof(Patient));
    if (!new_node)
    {
        printf("内存分配失败！\n");
        free_patients(patient_head);
        wait_enter();
        return;
    }

    *new_node = create_patient(id, name, gender, age, pwd_hash);
    strncpy(new_node->salt, salt_hex, sizeof(new_node->salt) - 1);
    new_node->salt[sizeof(new_node->salt) - 1] = '\0';
    new_node->next = NULL;

    append_patient(&patient_head, new_node);

    if (save_patients_to_file(patient_head) != 0)
        printf("保存患者信息失败！\n");
    else
        printf("注册成功！您的患者ID是: %s\n", id);

    free_patients(patient_head);
    wait_enter();
    clear_screen();
}
