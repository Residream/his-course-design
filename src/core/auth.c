/*
 * 登录验证模块
 */
#include "core/auth.h"
#include "core/sha256.h"
#include "core/utils.h"
#include "model/patient.h"

/* 单个十六进制字符串转整数 */
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
    int n = (int)strlen(hex); // 输入十六进制字符串长度必须是输出字节数组长度的两倍
    if (n != out_len * 2)
        return 0;

    for (int i = 0; i < out_len; i++) // 每两个十六进制字符转换成一个字节
    {
        int hi = hex_char_to_val(hex[i * 2]);     // 高4位，即第一个十六进制字符
        int lo = hex_char_to_val(hex[i * 2 + 1]); // 低4位，即第二个十六进制字符

        if (hi < 0 || lo < 0) // 如果遇到非十六进制字符，转换失败
            return 0;

        out[i] = (unsigned char)((hi << 4) | lo); // 将高4位和低4位合成一个字节
    }
    return 1;
}

/* 校验输入密码是否与存储的 hash/salt 匹配 */
int verify_password(const char *input_password, const char *stored_salt_hex, const char *stored_hash_hex)
{
    if (!input_password || !stored_salt_hex || !stored_hash_hex) // 输入参数不能为空
        return 0;

    unsigned char salt[SALT_RAW_LEN]; // 存储盐的原始字节数组

    if (!hex_to_bytes(stored_salt_hex, salt, SALT_RAW_LEN)) // 将存储的十六进制盐转换成字节数组，如果失败则返回验证失败
        return 0;

    char computed_hash[MAX_PWD_HASH]; // 存储计算得到的密码哈希值的十六进制字符串

    build_pass_hash(salt, input_password, computed_hash); // 对输入密码加盐后计算哈希值，并转换成十六进制字符串

    return strcmp(computed_hash, stored_hash_hex) == 0; // 如果计算得到的哈希值与存储的哈希值匹配，则返回1；否则返回0
}

/* 对输入password加盐后加密 */
void build_pass_hash(const unsigned char salt[SALT_RAW_LEN], const char *password, char out_hex[65])
{
    uint8_t buf[SALT_RAW_LEN + MAX_INPUT_LEN]; // 存储盐和密码组合的字节数组，长度为盐的长度加上密码的最大长度

    size_t pass_len = strlen(password); // 计算输入密码的长度，如果超过最大输入长度，则截断到最大输入长度

    if (pass_len > MAX_INPUT_LEN) // 如果输入密码长度超过最大限制，截断到最大长度
        pass_len = MAX_INPUT_LEN;

    memcpy(buf, salt, SALT_RAW_LEN);                // 将盐复制到缓冲区的前面
    memcpy(buf + SALT_RAW_LEN, password, pass_len); // 将密码复制到缓冲区的后面，形成盐+密码的组合

    uint8_t hash[32]; // 存储计算得到的SHA256哈希值，长度为32字节

    sha256(buf, SALT_RAW_LEN + pass_len, hash); // 对盐和密码的组合进行SHA256哈希计算，结果存储在hash数组中

    sha256_to_hex(hash,
                  out_hex); // 将计算得到的哈希值转换成十六进制字符串，存储在out_hex中，长度为64字符加上字符串结束符
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

        /* 读取name */
        char *token = strtok(line, "|");
        if (!token)
            continue;
        if (strcmp(token, name) != 0) // 如果name不匹配，继续下一行
            continue;

        /* 读取pwd_hash */
        token = strtok(NULL, "|");
        if (!token)
            continue;
        char stored_hash[MAX_PWD_HASH];
        strncpy(stored_hash, token, sizeof(stored_hash) - 1);
        stored_hash[sizeof(stored_hash) - 1] = '\0';

        /* 读取salt */
        token = strtok(NULL, "|");
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

        /* 读取id */
        char *token = strtok(line, "|");
        if (!token)
            continue;
        if (strcmp(token, id) != 0) // 寻找匹配的患者ID，如果不匹配，继续下一行
            continue;

        /* 跳过name */
        token = strtok(NULL, "|");
        if (!token)
            continue;
        /* 跳过gender */
        token = strtok(NULL, "|");
        if (!token)
            continue;
        /* 跳过age */
        token = strtok(NULL, "|");
        if (!token)
            continue;

        /* 读取pwd_hash */
        token = strtok(NULL, "|");
        if (!token)
            continue;
        char stored_hash[MAX_PWD_HASH];
        strncpy(stored_hash, token, sizeof(stored_hash) - 1);
        stored_hash[sizeof(stored_hash) - 1] = '\0';

        /* 读取salt */
        token = strtok(NULL, "|");
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

        /* id */
        char *token = strtok(line, "|");
        if (!token)
            continue;
        if (strcmp(token, id) != 0) // 寻找匹配的患者ID，如果不匹配，继续下一行
            continue;

        /* 跳过name */
        token = strtok(NULL, "|");
        if (!token)
            continue;
        /* 跳过gender */
        token = strtok(NULL, "|");
        if (!token)
            continue;
        /* 跳过department */
        token = strtok(NULL, "|");
        if (!token)
            continue;

        /* 读取pwd_hash */
        token = strtok(NULL, "|");
        if (!token)
            continue;
        char stored_hash[MAX_PWD_HASH];
        strncpy(stored_hash, token, sizeof(stored_hash) - 1);
        stored_hash[sizeof(stored_hash) - 1] = '\0';

        /* 读取salt */
        token = strtok(NULL, "|");
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
void patient_register(void)
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
            is_number = 0; // 只有符号没有数字
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
