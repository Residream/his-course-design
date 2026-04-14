/*
 * 药品和药房管理模块
 */
#include "model/drug.h"
#include "core/session.h"
#include "core/utils.h"
#include "model/department.h"
#include "model/doctor.h"
#include "model/patient.h"
#include "model/prescription.h"
#include "ui/menu.h"
#include <errno.h>
#include <limits.h>
#include <math.h>

/*
 * 药品基础操作
 */

/* 解析非负整数字符串，严格验证格式和范围 */
static int parse_nonnegative_int_strict(const char *text, int *out)
{
    if (!text || !out || text[0] == '\0')
        return 0;

    errno = 0;
    char *endptr = NULL;
    long value = strtol(text, &endptr, 10);
    if (errno == ERANGE || *endptr != '\0' || value < 0 || value > INT_MAX)
        return 0;

    *out = (int)value;
    return 1;
}

/* 解析非负浮点数字符串，严格验证格式和范围 */
static int parse_nonnegative_float_strict(const char *text, float *out)
{
    if (!text || !out || text[0] == '\0')
        return 0;

    errno = 0;
    char *endptr = NULL;
    float value = strtof(text, &endptr);
    if (errno == ERANGE || *endptr != '\0' || value < 0.0f || !isfinite((double)value))
        return 0;

    *out = value;
    return 1;
}

/* 从文件中加载药品数据 */
Drug *load_drugs_from_file()
{
    FILE *fp = fopen(DRUGS_FILE, "r");
    if (!fp)
        return NULL;

    Drug *head = NULL, *tail = NULL;
    char line[MAX_LINE_LEN];

    while (fgets(line, sizeof(line), fp))
    {
        trim_newline(line);
        if (!line[0])
            continue;

        Drug *node = (Drug *)malloc(sizeof(Drug));
        if (!node)
        {
            free_drugs(head);
            fclose(fp);
            return NULL;
        }
        memset(node, 0, sizeof(Drug));

        char *token = strtok(line, "|");
        if (!token)
        {
            free(node);
            continue;
        }
        strncpy(node->id, token, sizeof(node->id) - 1);

        token = strtok(NULL, "|");
        if (!token)
        {
            free(node);
            continue;
        }
        strncpy(node->generic_name, token, sizeof(node->generic_name) - 1);

        token = strtok(NULL, "|");
        if (!token)
        {
            free(node);
            continue;
        }
        strncpy(node->trade_name, token, sizeof(node->trade_name) - 1);

        token = strtok(NULL, "|");
        if (!token)
        {
            free(node);
            continue;
        }
        strncpy(node->alias, token, sizeof(node->alias) - 1);

        token = strtok(NULL, "|");
        if (!token)
        {
            free(node);
            continue;
        }
        if (!parse_nonnegative_float_strict(token, &node->price))
        {
            free(node);
            continue;
        }

        token = strtok(NULL, "|");
        if (!token)
        {
            free(node);
            continue;
        }
        if (!parse_nonnegative_int_strict(token, &node->stock))
        {
            free(node);
            continue;
        }

        token = strtok(NULL, "|");
        if (token)
        {
            strncpy(node->department, token, sizeof(node->department) - 1);
        }
        else
        {
            strcpy(node->department, "通用"); // 兼容旧数据
        }

        node->next = NULL;
        if (!head)
            head = tail = node;
        else
        {
            tail->next = node;
            tail = node;
        }
    }
    fclose(fp);
    return head;
}

/* 将药品数据保存到文件 */
int save_drugs_to_file(Drug *head)
{
    char tmp_path[MAX_LINE_LEN];
    FILE *fp = safe_fopen_tmp(DRUGS_FILE, tmp_path, sizeof(tmp_path));
    if (!fp)
        return -1;
    for (Drug *cur = head; cur; cur = cur->next)
        fprintf(fp, "%s|%s|%s|%s|%.2f|%d|%s\n", cur->id, cur->generic_name, cur->trade_name, cur->alias, cur->price,
                cur->stock, cur->department);
    return safe_fclose_commit(fp, tmp_path, DRUGS_FILE);
}

/* 释放药品数据内存 */
void free_drugs(Drug *head)
{
    while (head)
    {
        Drug *tmp = head;
        head = head->next;
        free(tmp);
    }
}

/* 根据ID查找药品 */
Drug *find_drug_by_id(Drug *head, const char *id)
{
    for (Drug *cur = head; cur; cur = cur->next)
        if (strcmp(cur->id, id) == 0)
            return cur;
    return NULL;
}

/* 生成下一个药品ID */
int generate_next_drug_id(Drug *head)
{
    int max_id = 0;
    for (Drug *cur = head; cur; cur = cur->next)
    {
        const char *id = cur->id;
        if (strncmp(id, "DR", 2) == 0)
        {
            int valid = 1;
            for (int i = 2; id[i] != '\0'; i++)
            {
                if (id[i] < '0' || id[i] > '9')
                {
                    valid = 0;
                    break;
                }
            }
            if (valid && id[2] != '\0')
            {
                int id_num = atoi(id + 2);
                if (id_num > max_id)
                    max_id = id_num;
            }
        }
    }
    return max_id + 1;
}

/*
 * 药房基础操作
 */

/* 从文件中加载药房数据 */
Pharmacy *load_pharmacies_from_file()
{
    FILE *fp = fopen(PHARMACIES_FILE, "r");
    if (!fp)
        return NULL;

    Pharmacy *head = NULL, *tail = NULL;
    char line[MAX_LINE_LEN];

    while (fgets(line, sizeof(line), fp))
    {
        trim_newline(line);
        if (!line[0])
            continue;

        Pharmacy *node = (Pharmacy *)malloc(sizeof(Pharmacy));
        if (!node)
        {
            free_pharmacies(head);
            fclose(fp);
            return NULL;
        }
        memset(node, 0, sizeof(Pharmacy));

        char *token = strtok(line, "|");
        if (!token)
        {
            free(node);
            continue;
        }
        strncpy(node->id, token, sizeof(node->id) - 1);

        token = strtok(NULL, "|");
        if (!token)
        {
            free(node);
            continue;
        }
        strncpy(node->name, token, sizeof(node->name) - 1);

        token = strtok(NULL, "|");
        if (!token)
        {
            free(node);
            continue;
        }
        strncpy(node->location, token, sizeof(node->location) - 1);

        node->next = NULL;
        if (!head)
            head = tail = node;
        else
        {
            tail->next = node;
            tail = node;
        }
    }
    fclose(fp);
    return head;
}

/* 将药房数据保存到文件 */
int save_pharmacies_to_file(Pharmacy *head)
{
    char tmp_path[MAX_LINE_LEN];
    FILE *fp = safe_fopen_tmp(PHARMACIES_FILE, tmp_path, sizeof(tmp_path));
    if (!fp)
        return -1;
    for (Pharmacy *cur = head; cur; cur = cur->next)
        fprintf(fp, "%s|%s|%s\n", cur->id, cur->name, cur->location);
    return safe_fclose_commit(fp, tmp_path, PHARMACIES_FILE);
}

/* 释放药房数据内存 */
void free_pharmacies(Pharmacy *head)
{
    while (head)
    {
        Pharmacy *tmp = head;
        head = head->next;
        free(tmp);
    }
}

/* 根据ID查找药房 */
Pharmacy *find_pharmacy_by_id(Pharmacy *head, const char *id)
{
    for (Pharmacy *cur = head; cur; cur = cur->next)
        if (strcmp(cur->id, id) == 0)
            return cur;
    return NULL;
}

/* 生成下一个药房ID */
int generate_next_pharmacy_id(Pharmacy *head)
{
    int max_id = 0;
    for (Pharmacy *cur = head; cur; cur = cur->next)
    {
        const char *id = cur->id;
        if (strncmp(id, "PH", 2) == 0)
        {
            int valid = 1;
            for (int i = 2; id[i] != '\0'; i++)
            {
                if (id[i] < '0' || id[i] > '9')
                {
                    valid = 0;
                    break;
                }
            }
            if (valid && id[2] != '\0')
            {
                int id_num = atoi(id + 2);
                if (id_num > max_id)
                    max_id = id_num;
            }
        }
    }
    return max_id + 1;
}

/*
 * 药房药品关联操作
 */

/* 从文件中加载药房药品数据 */
PharmacyDrug *load_pharmacy_drugs_from_file()
{
    FILE *fp = fopen(PHARMACY_DRUGS_FILE, "r");
    if (!fp)
        return NULL;

    PharmacyDrug *head = NULL, *tail = NULL;
    char line[MAX_LINE_LEN];

    while (fgets(line, sizeof(line), fp))
    {
        trim_newline(line);
        if (!line[0])
            continue;

        PharmacyDrug *node = (PharmacyDrug *)malloc(sizeof(PharmacyDrug));
        if (!node)
        {
            free_pharmacy_drugs(head);
            fclose(fp);
            return NULL;
        }
        memset(node, 0, sizeof(PharmacyDrug));

        char *token = strtok(line, "|");
        if (!token)
        {
            free(node);
            continue;
        }
        strncpy(node->pharmacy_id, token, sizeof(node->pharmacy_id) - 1);

        token = strtok(NULL, "|");
        if (!token)
        {
            free(node);
            continue;
        }
        strncpy(node->drug_id, token, sizeof(node->drug_id) - 1);

        token = strtok(NULL, "|");
        if (!token)
        {
            free(node);
            continue;
        }
        if (!parse_nonnegative_int_strict(token, &node->quantity))
        {
            free(node);
            continue;
        }

        node->next = NULL;
        if (!head)
            head = tail = node;
        else
        {
            tail->next = node;
            tail = node;
        }
    }
    fclose(fp);
    return head;
}

/* 将药房药品数据保存到文件 */
int save_pharmacy_drugs_to_file(PharmacyDrug *head)
{
    char tmp_path[MAX_LINE_LEN];
    FILE *fp = safe_fopen_tmp(PHARMACY_DRUGS_FILE, tmp_path, sizeof(tmp_path));
    if (!fp)
        return -1;
    for (PharmacyDrug *cur = head; cur; cur = cur->next)
        fprintf(fp, "%s|%s|%d\n", cur->pharmacy_id, cur->drug_id, cur->quantity);
    return safe_fclose_commit(fp, tmp_path, PHARMACY_DRUGS_FILE);
}

/* 释放药房药品数据内存 */
void free_pharmacy_drugs(PharmacyDrug *head)
{
    while (head)
    {
        PharmacyDrug *tmp = head;
        head = head->next;
        free(tmp);
    }
}

/*
 * 药品表格与排版
 */

/* 计算药品表格列宽 */
void calc_drug_width(Drug *head, int *id_w, int *gn_w, int *tn_w, int *al_w, int *pr_w, int *st_w, int *dept_w)
{
    *id_w = str_width("药品ID");
    *gn_w = str_width("通用名");
    *tn_w = str_width("商品名");
    *al_w = str_width("别名");
    *pr_w = str_width("价格");
    *st_w = str_width("库存");
    *dept_w = str_width("适用科室");

    for (Drug *d = head; d; d = d->next)
    {
        int w;
        char buf[32];
        w = str_width(d->id);
        if (w > *id_w)
            *id_w = w;
        w = str_width(d->generic_name);
        if (w > *gn_w)
            *gn_w = w;
        w = str_width(d->trade_name);
        if (w > *tn_w)
            *tn_w = w;
        w = str_width(d->alias);
        if (w > *al_w)
            *al_w = w;
        sprintf(buf, "%.2f", d->price);
        w = str_width(buf);
        if (w > *pr_w)
            *pr_w = w;
        sprintf(buf, "%d", d->stock);
        w = str_width(buf);
        if (w > *st_w)
            *st_w = w;
        w = str_width(d->department);
        if (w > *dept_w)
            *dept_w = w;
    }
}

/* 打印药品表格分隔线 */
void print_drug_line(int id_w, int gn_w, int tn_w, int al_w, int pr_w, int st_w, int dept_w)
{
    printf("+");
    for (int i = 0; i < id_w + 2; i++)
        printf("-");
    printf("+");
    for (int i = 0; i < gn_w + 2; i++)
        printf("-");
    printf("+");
    for (int i = 0; i < tn_w + 2; i++)
        printf("-");
    printf("+");
    for (int i = 0; i < al_w + 2; i++)
        printf("-");
    printf("+");
    for (int i = 0; i < pr_w + 2; i++)
        printf("-");
    printf("+");
    for (int i = 0; i < st_w + 2; i++)
        printf("-");
    printf("+");
    for (int i = 0; i < dept_w + 2; i++)
        printf("-");
    printf("+\n");
}

/* 打印药品表头 */
void print_drug_header(int id_w, int gn_w, int tn_w, int al_w, int pr_w, int st_w, int dept_w)
{
    print_drug_line(id_w, gn_w, tn_w, al_w, pr_w, st_w, dept_w);
    printf("| ");
    print_align("药品ID", id_w);
    printf(" | ");
    print_align("通用名", gn_w);
    printf(" | ");
    print_align("商品名", tn_w);
    printf(" | ");
    print_align("别名", al_w);
    printf(" | ");
    print_align("价格", pr_w);
    printf(" | ");
    print_align("库存", st_w);
    printf(" | ");
    print_align("适用科室", dept_w);
    printf(" |\n");
    print_drug_line(id_w, gn_w, tn_w, al_w, pr_w, st_w, dept_w);
}

/* 打印药品信息行 */
void print_drug(Drug *d, int id_w, int gn_w, int tn_w, int al_w, int pr_w, int st_w, int dept_w)
{
    char pr_buf[32], st_buf[32];
    sprintf(pr_buf, "%.2f", d->price);
    sprintf(st_buf, "%d", d->stock);

    printf("| ");
    print_align(d->id, id_w);
    printf(" | ");
    print_align(d->generic_name, gn_w);
    printf(" | ");
    print_align(d->trade_name, tn_w);
    printf(" | ");
    print_align(d->alias, al_w);
    printf(" | ");
    print_align(pr_buf, pr_w);
    printf(" | ");
    print_align(st_buf, st_w);
    printf(" | ");
    print_align(d->department[0] ? d->department : "通用", dept_w);
    printf(" |\n");
}

/* 计算药房表格列宽 */
void calc_pharmacy_width(Pharmacy *head, int *id_w, int *nm_w, int *loc_w)
{
    *id_w = str_width("药房ID");
    *nm_w = str_width("药房名称");
    *loc_w = str_width("位置");
    for (Pharmacy *p = head; p; p = p->next)
    {
        int w = str_width(p->id);
        if (w > *id_w)
            *id_w = w;
        w = str_width(p->name);
        if (w > *nm_w)
            *nm_w = w;
        w = str_width(p->location);
        if (w > *loc_w)
            *loc_w = w;
    }
}

/* 打印药房表格分隔线 */
void print_pharmacy_line(int id_w, int nm_w, int loc_w)
{
    printf("+");
    for (int i = 0; i < id_w + 2; i++)
        printf("-");
    printf("+");
    for (int i = 0; i < nm_w + 2; i++)
        printf("-");
    printf("+");
    for (int i = 0; i < loc_w + 2; i++)
        printf("-");
    printf("+\n");
}

/* 打印药房表头 */
void print_pharmacy_header(int id_w, int nm_w, int loc_w)
{
    print_pharmacy_line(id_w, nm_w, loc_w);
    printf("| ");
    print_align("药房ID", id_w);
    printf(" | ");
    print_align("药房名称", nm_w);
    printf(" | ");
    print_align("位置", loc_w);
    printf(" |\n");
    print_pharmacy_line(id_w, nm_w, loc_w);
}

/* 打印药房信息行 */
void print_pharmacy(Pharmacy *p, int id_w, int nm_w, int loc_w)
{
    printf("| ");
    print_align(p->id, id_w);
    printf(" | ");
    print_align(p->name, nm_w);
    printf(" | ");
    print_align(p->location, loc_w);
    printf(" |\n");
}

/* 统计药品数量 */
int count_drugs(Drug *head)
{
    int count = 0;
    for (; head; head = head->next)
        count++;
    return count;
}

/* 获取第n个药品节点 */
Drug *get_nth_drug(Drug *head, int n)
{
    for (int i = 0; i < n && head; i++)
        head = head->next;
    return head;
}

/* 统计药房数量 */
int count_pharmacies(Pharmacy *head)
{
    int count = 0;
    for (; head; head = head->next)
        count++;
    return count;
}

/* 获取第n个药房节点 */
Pharmacy *get_nth_pharmacy(Pharmacy *head, int n)
{
    for (int i = 0; i < n && head; i++)
        head = head->next;
    return head;
}

/* 打印药房列表提示（在需要输入药房ID前调用，方便用户查阅） */
void print_pharmacy_hint()
{
    Pharmacy *head = load_pharmacies_from_file();
    if (!head)
    {
        printf("（暂无药房数据）\n");
        return;
    }
    printf("当前可用药房:\n");
    for (Pharmacy *p = head; p; p = p->next)
        printf("  %-10s %s（%s）\n", p->id, p->name, p->location);
    free_pharmacies(head);
}

/*
 * 药品系统功能
 */

/* 添加药品 */
void add_drug()
{
    Drug *head = load_drugs_from_file();
    Drug *new_drug = (Drug *)malloc(sizeof(Drug));
    if (!new_drug)
    {
        printf("内存分配失败！\n");
        free_drugs(head);
        wait_enter();
        return;
    }

    int id_num = generate_next_drug_id(head);
    snprintf(new_drug->id, sizeof(new_drug->id), "DR%04d", id_num);

    while (1)
    {
        printf("请输入药品通用名(汉字 | 输入0返回): ");
        safe_input(new_drug->generic_name, sizeof(new_drug->generic_name));
        if (strcmp(new_drug->generic_name, "0") == 0)
        {
            free(new_drug);
            free_drugs(head);
            return;
        }
        if (new_drug->generic_name[0] == '\0')
        {
            printf("通用名不能为空！\n");
            continue;
        }
        if (is_all_chinese_utf8(new_drug->generic_name) == 0)
        {
            printf("通用名只能包含汉字！\n");
            continue;
        }
        break;
    }

    while (1)
    {
        printf("请输入药品商品名(汉字 | 输入0返回): ");
        safe_input(new_drug->trade_name, sizeof(new_drug->trade_name));
        if (strcmp(new_drug->trade_name, "0") == 0)
        {
            free(new_drug);
            free_drugs(head);
            return;
        }
        if (new_drug->trade_name[0] == '\0')
        {
            printf("商品名不能为空！\n");
            continue;
        }
        if (is_all_chinese_utf8(new_drug->trade_name) == 0)
        {
            printf("商品名只能包含汉字！\n");
            continue;
        }
        break;
    }

    while (1)
    {
        printf("请输入药品别名(汉字 | 输入0返回): ");
        safe_input(new_drug->alias, sizeof(new_drug->alias));
        if (strcmp(new_drug->alias, "0") == 0)
        {
            free(new_drug);
            free_drugs(head);
            return;
        }
        if (new_drug->alias[0] == '\0')
        {
            printf("别名不能为空！\n");
            continue;
        }
        if (is_all_chinese_utf8(new_drug->alias) == 0 && strcmp(new_drug->alias, "无") != 0)
        {
            printf("别名只能包含汉字！\n");
            continue;
        }
        break;
    }

    char price_input[32];
    while (1)
    {
        printf("请输入药品价格(输入0返回): ");
        safe_input(price_input, sizeof(price_input));

        if (strcmp(price_input, "0") == 0)
        {
            free(new_drug);
            free_drugs(head);
            return;
        }

        char *endptr;
        errno = 0;
        double p_d = strtod(price_input, &endptr);
        if (endptr == price_input || *endptr != '\0' || p_d < 0)
        {
            printf("输入有误，请重新输入正确价格！\n");
            continue;
        }
        if (p_d > MAX_DRUG_PRICE)
        {
            printf("价格超出上限 %.2f，请重新输入！\n", MAX_DRUG_PRICE);
            continue;
        }
        new_drug->price = (float)p_d;
        break;
    }

    char stock_input[32];
    while (1)
    {
        printf("请输入药品初始总库存(输入0返回): ");
        safe_input(stock_input, sizeof(stock_input));
        if (strcmp(stock_input, "0") == 0)
        {
            free(new_drug);
            free_drugs(head);
            return;
        }
        if (!parse_nonnegative_int_strict(stock_input, &new_drug->stock))
        {
            printf("输入有误，请重新输入正确库存！\n");
            continue;
        }
        break;
    }

    char department_input[MAX_NAME_LEN];
    print_department_hint();
    while (1)
    {
        printf("请输入药品适用科室(回车 = '通用' | 输入0返回): ");
        safe_input(department_input, sizeof(department_input));
        if (strcmp(department_input, "0") == 0)
        {
            free(new_drug);
            free_drugs(head);
            return;
        }
        if (department_input[0] == '\0')
        {
            strcpy(new_drug->department, "通用");
            break;
        }
        if (is_all_chinese_utf8(department_input) == 0)
        {
            printf("科室名称只能包含汉字！\n");
            continue;
        }
        if (!is_valid_department(department_input))
        {
            printf("输入的科室无效！请重新输入。\n");
            continue;
        }
        strncpy(new_drug->department, department_input, sizeof(new_drug->department) - 1);
        break;
    }

    new_drug->next = NULL;
    if (!head)
        head = new_drug;
    else
    {
        Drug *tail = head;
        while (tail->next)
            tail = tail->next;
        tail->next = new_drug;
    }

    if (save_drugs_to_file(head) != 0)
        printf("保存药品信息失败！\n");
    else
        printf("药品添加成功！分配ID: %s\n", new_drug->id);
    free_drugs(head);
    wait_enter();
    clear_screen();
}

/*
 * 删除药品(管理员功能)
 * 级联操作: 同时清除该药品在 pharmacy_drugs 表中的所有库存关联
 * 涉及两张表联动: drugs + pharmacy_drugs
 * 保存失败时回滚两张表
 */
void delete_drug()
{
    char id[MAX_ID_LEN];
    printf("请输入要删除的药品ID(输入0返回): ");
    safe_input(id, sizeof(id));
    if (strcmp(id, "0") == 0)
    {
        clear_screen();
        return;
    }
    if (id[0] == '\0')
    {
        printf("药品ID不能为空！\n");
        wait_enter();
        clear_screen();
        return;
    }

    Drug *head = load_drugs_from_file();
    Drug *prev = NULL, *cur = head;

    while (cur)
    {
        if (strcmp(cur->id, id) == 0)
        {
            int id_w, gn_w, tn_w, al_w, pr_w, st_w, dept_w;
            calc_drug_width(head, &id_w, &gn_w, &tn_w, &al_w, &pr_w, &st_w, &dept_w);

            clear_screen();
            printf("找到药品:\n");
            print_drug_header(id_w, gn_w, tn_w, al_w, pr_w, st_w, dept_w);
            print_drug(cur, id_w, gn_w, tn_w, al_w, pr_w, st_w, dept_w);
            print_drug_line(id_w, gn_w, tn_w, al_w, pr_w, st_w, dept_w);

            char confirm[16];
            printf("\n确认删除该药品及其在各药房中的库存关联吗？(y/n): ");
            safe_input(confirm, sizeof(confirm));
            if (strcmp(confirm, "y") != 0 && strcmp(confirm, "Y") != 0)
            {
                printf("已取消删除。\n");
                free_drugs(head);
                wait_enter();
                clear_screen();
                return;
            }

            PharmacyDrug *pd_head = load_pharmacy_drugs_from_file();
            PharmacyDrug *pd_prev = NULL, *pd_cur = pd_head;
            PharmacyDrug *removed_pd_head = NULL,
                         *removed_pd_tail = NULL; // 暂存被移除的药房库存节点，保存失败时可直接拼回原链表
            while (pd_cur)
            {
                if (strcmp(pd_cur->drug_id, id) == 0)
                {
                    PharmacyDrug *tmp = pd_cur;
                    if (pd_prev)
                        pd_prev->next = pd_cur->next;
                    else
                        pd_head = pd_cur->next;
                    pd_cur = pd_cur->next;
                    tmp->next = NULL;
                    if (!removed_pd_head)
                        removed_pd_head = removed_pd_tail = tmp;
                    else
                    {
                        removed_pd_tail->next = tmp;
                        removed_pd_tail = tmp;
                    }
                }
                else
                {
                    pd_prev = pd_cur;
                    pd_cur = pd_cur->next;
                }
            }
            Drug *deleted_drug = cur; // 主表被删节点先保留，直到双文件都保存成功才真正 free
            if (prev)
                prev->next = cur->next;
            else
                head = cur->next;
            deleted_drug->next = NULL;

            /* 两份文件都成功才算提交，任一失败就走回滚 */
            int s_pd = save_pharmacy_drugs_to_file(pd_head);
            int s_d = save_drugs_to_file(head);
            if (s_pd == 0 && s_d == 0)
            {
                while (removed_pd_head)
                {
                    PharmacyDrug *tmp = removed_pd_head;
                    removed_pd_head = removed_pd_head->next;
                    free(tmp);
                }
                free(deleted_drug);
                printf("删除成功！\n");
            }
            else
            {
                /* 先恢复主药品链 */
                if (prev)
                {
                    deleted_drug->next = prev->next;
                    prev->next = deleted_drug;
                }
                else
                {
                    deleted_drug->next = head;
                    head = deleted_drug;
                }

                /* 再恢复药房库存关联链 */
                if (removed_pd_head)
                {
                    removed_pd_tail->next = pd_head;
                    pd_head = removed_pd_head;
                }

                /* 回滚后的状态重新落盘，尽量保证文件与内存一致 */
                int rb1 = save_pharmacy_drugs_to_file(pd_head);
                int rb2 = save_drugs_to_file(head);
                printf("删除失败：保存文件失败。");
                if (rb1 == 0 && rb2 == 0)
                    printf("已回滚。\n");
                else
                    printf("且回滚失败，请立即检查数据文件。\n");
            }

            free_pharmacy_drugs(pd_head);
            free_drugs(head);
            wait_enter();
            clear_screen();
            return;
        }
        prev = cur;
        cur = cur->next;
    }
    printf("未找到该药品！\n");
    free_drugs(head);
    wait_enter();
    clear_screen();
}

/* 修改药品信息 */
void update_drug()
{
    char id[MAX_ID_LEN];
    printf("请输入要修改的药品ID(输入0返回): ");
    safe_input(id, sizeof(id));
    if (strcmp(id, "0") == 0)
    {
        clear_screen();
        return;
    }
    if (id[0] == '\0')
    {
        printf("药品ID不能为空！\n");
        wait_enter();
        clear_screen();
        return;
    }

    Drug *head = load_drugs_from_file();
    Drug *drug = find_drug_by_id(head, id);
    if (!drug)
    {
        printf("未找到指定ID的药品！\n");
        free_drugs(head);
        wait_enter();
        clear_screen();
        return;
    }

    int id_w, gn_w, tn_w, al_w, pr_w, st_w, dept_w;
    char buf[MAX_INPUT_LEN];

    while (1)
    {
        clear_screen();
        calc_drug_width(head, &id_w, &gn_w, &tn_w, &al_w, &pr_w, &st_w, &dept_w);
        printf("找到药品:\n");
        print_drug_header(id_w, gn_w, tn_w, al_w, pr_w, st_w, dept_w);
        print_drug(drug, id_w, gn_w, tn_w, al_w, pr_w, st_w, dept_w);
        print_drug_line(id_w, gn_w, tn_w, al_w, pr_w, st_w, dept_w);

        printf("请选择要更新的信息:\n");
        printf("1. 通用名\n");
        printf("2. 商品名\n");
        printf("3. 别名\n");
        printf("4. 价格\n");
        printf("5. 总库存 (多药房时仅允许与合计一致)\n");
        printf("6. 适用科室\n");
        printf("0. 返回\n");
        printf("请输入您的选择: ");
        safe_input(buf, sizeof(buf));

        if (!validate_choice(buf, 6))
        {
            printf("输入有误，请重新选择！\n");
            wait_enter();
            continue;
        }

        int select = atoi(buf);
        if (select == 0)
            break;

        switch (select)
        {
        case 1:
            while (1)
            {
                printf("请输入新的通用名(汉字 | 输入0取消): ");
                safe_input(buf, sizeof(buf));
                if (strcmp(buf, "0") == 0)
                    break;
                if (buf[0] == '\0')
                {
                    printf("通用名不能为空！\n");
                    continue;
                }
                if (!is_all_chinese_utf8(buf))
                {
                    printf("通用名只能包含汉字！\n");
                    continue;
                }
                strncpy(drug->generic_name, buf, sizeof(drug->generic_name) - 1);
                drug->generic_name[sizeof(drug->generic_name) - 1] = '\0';
                if (save_drugs_to_file(head) != 0)
                    printf("保存药品信息失败！\n");
                else
                    printf("更新成功！\n");
                wait_enter();
                break;
            }
            break;
        case 2:
            while (1)
            {
                printf("请输入新的商品名(汉字 | 输入0取消): ");
                safe_input(buf, sizeof(buf));
                if (strcmp(buf, "0") == 0)
                    break;
                if (buf[0] == '\0')
                {
                    printf("商品名不能为空！\n");
                    continue;
                }
                if (!is_all_chinese_utf8(buf))
                {
                    printf("商品名只能包含汉字！\n");
                    continue;
                }
                strncpy(drug->trade_name, buf, sizeof(drug->trade_name) - 1);
                drug->trade_name[sizeof(drug->trade_name) - 1] = '\0';
                if (save_drugs_to_file(head) != 0)
                    printf("保存药品信息失败！\n");
                else
                    printf("更新成功！\n");
                wait_enter();
                break;
            }
            break;
        case 3:
            while (1)
            {
                printf("请输入新的别名(汉字 | 输入0取消): ");
                safe_input(buf, sizeof(buf));
                if (strcmp(buf, "0") == 0)
                    break;
                if (buf[0] == '\0')
                {
                    printf("别名不能为空！\n");
                    continue;
                }
                if (!is_all_chinese_utf8(buf) && strcmp(buf, "无") != 0)
                {
                    printf("别名只能包含汉字！\n");
                    continue;
                }
                strncpy(drug->alias, buf, sizeof(drug->alias) - 1);
                drug->alias[sizeof(drug->alias) - 1] = '\0';
                if (save_drugs_to_file(head) != 0)
                    printf("保存药品信息失败！\n");
                else
                    printf("更新成功！\n");
                wait_enter();
                break;
            }
            break;
        case 4:
            while (1)
            {
                printf("请输入新的价格(输入q取消): ");
                safe_input(buf, sizeof(buf));
                if (strcmp(buf, "q") == 0 || strcmp(buf, "Q") == 0)
                    break;

                char *endptr;
                errno = 0;
                double p_d = strtod(buf, &endptr);
                if (endptr == buf || *endptr != '\0' || p_d < 0)
                {
                    printf("输入有误，请输入非负数字价格！\n");
                    continue;
                }
                if (p_d > MAX_DRUG_PRICE)
                {
                    printf("价格超出上限 %.2f，请重新输入！\n", MAX_DRUG_PRICE);
                    continue;
                }
                drug->price = (float)p_d;
                if (save_drugs_to_file(head) != 0)
                    printf("保存药品信息失败！\n");
                else
                    printf("更新成功！\n");
                wait_enter();
                break;
            }
            break;
        case 5:
            while (1)
            {
                printf("请输入新的总库存(输入q取消): ");
                safe_input(buf, sizeof(buf));
                if (strcmp(buf, "q") == 0 || strcmp(buf, "Q") == 0)
                    break;
                int s;
                if (!parse_nonnegative_int_strict(buf, &s))
                {
                    printf("输入有误，请输入非负整数库存！\n");
                    continue;
                }

                PharmacyDrug *pd_head = load_pharmacy_drugs_from_file();
                int pd_count = 0;
                long long pd_total = 0;
                PharmacyDrug *single_pd = NULL;
                for (PharmacyDrug *pd = pd_head; pd; pd = pd->next)
                {
                    if (strcmp(pd->drug_id, drug->id) == 0)
                    {
                        pd_count++;
                        pd_total += pd->quantity;
                        if (!single_pd)
                            single_pd = pd;
                    }
                }

                if (pd_count > 1 && (long long)s != pd_total)
                {
                    printf("该药品分布在多个药房，不能直接覆盖为新总库存。\n");
                    printf("请通过入库/发药调整；当前各药房合计库存为: %lld\n", pd_total);
                    free_pharmacy_drugs(pd_head);
                    wait_enter();
                    break;
                }

                int old_stock = drug->stock;
                int old_single_qty = 0;
                if (pd_count == 1 && single_pd)
                {
                    old_single_qty = single_pd->quantity;
                    single_pd->quantity = s;
                }
                drug->stock = s;

                int s_drug = (save_drugs_to_file(head) == 0);
                int s_pd = 1;
                if (pd_count == 1)
                    s_pd = (save_pharmacy_drugs_to_file(pd_head) == 0);

                if (s_drug && s_pd)
                {
                    if (pd_count == 1)
                        printf("更新成功！已同步单一药房库存。\n");
                    else
                        printf("更新成功！\n");
                }
                else
                {
                    drug->stock = old_stock;
                    if (pd_count == 1 && single_pd)
                        single_pd->quantity = old_single_qty;
                    int rb1 = (save_drugs_to_file(head) == 0);
                    int rb2 = 1;
                    if (pd_count == 1)
                        rb2 = (save_pharmacy_drugs_to_file(pd_head) == 0);
                    printf("更新失败：保存文件失败。");
                    if (rb1 && rb2)
                        printf("已回滚。\n");
                    else
                        printf("且回滚失败，请立即检查数据文件。\n");
                }
                free_pharmacy_drugs(pd_head);
                wait_enter();
                break;
            }
            break;
        case 6:
            print_department_hint();
            while (1)
            {
                printf("请输入新的适用科室(回车 = '通用' | 输入0取消): ");
                safe_input(buf, sizeof(buf));
                if (strcmp(buf, "0") == 0)
                    break;
                if (buf[0] == '\0')
                {
                    strcpy(buf, "通用");
                }
                else
                {
                    if (!is_all_chinese_utf8(buf))
                    {
                        printf("科室名称只能包含汉字！\n");
                        continue;
                    }
                    if (!is_valid_department(buf))
                    {
                        printf("输入的科室无效！请重新输入。\n");
                        continue;
                    }
                }
                strncpy(drug->department, buf, sizeof(drug->department) - 1);
                drug->department[sizeof(drug->department) - 1] = '\0';
                if (save_drugs_to_file(head) != 0)
                    printf("保存药品信息失败！\n");
                else
                    printf("更新成功！\n");
                wait_enter();
                break;
            }
            break;
        }
    }
    free_drugs(head);
    clear_screen();
}

/* 查询药品信息 */
void query_drug()
{
    Drug *head = load_drugs_from_file();
    if (!head)
    {
        printf("暂无药品数据！\n");
        wait_enter();
        clear_screen();
        return;
    }

    char buf[MAX_INPUT_LEN];
    while (1)
    {
        clear_screen();
        printf("===== 查询药品 =====\n");
        printf("1. 按药品ID查询\n");
        printf("2. 按名称模糊查询(通用/商品/别名)\n");
        printf("3. 按适用科室查询\n");
        printf("0. 返回\n");
        printf("请输入查询方式: ");
        safe_input(buf, sizeof(buf));

        if (!validate_choice(buf, 3))
        {
            printf("输入有误，请重新选择！\n");
            wait_enter();
            continue;
        }

        int select = atoi(buf);
        if (select == 0)
            break;

        char query[MAX_NAME_LEN];
        if (select == 3)
        {
            print_department_hint();
            printf("请输入科室名称(回车 = '通用' | 输入0返回): ");
            safe_input(query, sizeof(query));
            if (strcmp(query, "0") == 0)
                continue;
            if (query[0] == '\0')
                strcpy(query, "通用");
        }
        else
        {
            printf("请输入查询关键字(输入0返回): ");
            safe_input(query, sizeof(query));
            if (strcmp(query, "0") == 0)
                continue;
            if (query[0] == '\0')
            {
                printf("关键字不能为空！\n");
                wait_enter();
                continue;
            }
        }

        int id_w, gn_w, tn_w, al_w, pr_w, st_w, dept_w;
        calc_drug_width(head, &id_w, &gn_w, &tn_w, &al_w, &pr_w, &st_w, &dept_w);

        /* 先统计 */
        int found = 0;
        for (Drug *cur = head; cur; cur = cur->next)
        {
            if (select == 1 && strstr(cur->id, query) != NULL)
                found = 1;
            else if (select == 2 &&
                     (strstr(cur->generic_name, query) || strstr(cur->trade_name, query) || strstr(cur->alias, query)))
                found = 1;
            else if (select == 3 && strstr(cur->department, query) != NULL)
                found = 1;
            if (found)
                break; /* 找到一个就够了 */
        }

        if (!found)
        {
            printf("未找到匹配的药品。\n");
        }
        else
        {
            print_drug_header(id_w, gn_w, tn_w, al_w, pr_w, st_w, dept_w);
            for (Drug *cur = head; cur; cur = cur->next)
            {
                if (select == 1 && strstr(cur->id, query) != NULL)
                    print_drug(cur, id_w, gn_w, tn_w, al_w, pr_w, st_w, dept_w);
                else if (select == 2 && (strstr(cur->generic_name, query) || strstr(cur->trade_name, query) ||
                                         strstr(cur->alias, query)))
                    print_drug(cur, id_w, gn_w, tn_w, al_w, pr_w, st_w, dept_w);
                else if (select == 3 && strstr(cur->department, query) != NULL)
                    print_drug(cur, id_w, gn_w, tn_w, al_w, pr_w, st_w, dept_w);
            }
            print_drug_line(id_w, gn_w, tn_w, al_w, pr_w, st_w, dept_w);
        }
        wait_enter();
    }
    free_drugs(head);
    clear_screen();
}

/* 显示所有药品 */
void show_all_drugs()
{
    Drug *head = load_drugs_from_file();
    if (!head)
    {
        printf("暂无药品数据！\n");
        wait_enter();
        clear_screen();
        return;
    }

    int page_size = PAGE_SIZE;
    int total = count_drugs(head);
    int total_pages = (total + page_size - 1) / page_size;
    int current_page = 1;

    int id_w, gn_w, tn_w, al_w, pr_w, st_w, dept_w;
    calc_drug_width(head, &id_w, &gn_w, &tn_w, &al_w, &pr_w, &st_w, &dept_w);

    while (1)
    {
        clear_screen();
        printf("===== 药品库列表（第 %d/%d 页）=====\n", current_page, total_pages);
        print_drug_header(id_w, gn_w, tn_w, al_w, pr_w, st_w, dept_w);

        int start = (current_page - 1) * page_size;
        Drug *cur = get_nth_drug(head, start);

        for (int i = 0; i < page_size && cur; i++)
        {
            print_drug(cur, id_w, gn_w, tn_w, al_w, pr_w, st_w, dept_w);
            cur = cur->next;
        }
        print_drug_line(id_w, gn_w, tn_w, al_w, pr_w, st_w, dept_w);

        printf("\n[n]下一页  [p]上一页  [q]退出\n> ");
        char buf[MAX_INPUT_LEN];
        safe_input(buf, sizeof(buf));

        if (strcmp(buf, "n") == 0 || strcmp(buf, "N") == 0)
        {
            if (current_page < total_pages)
                current_page++;
        }
        else if (strcmp(buf, "p") == 0 || strcmp(buf, "P") == 0)
        {
            if (current_page > 1)
                current_page--;
        }
        else if (strcmp(buf, "q") == 0 || strcmp(buf, "Q") == 0)
            break;
        else
        {
            printf("输入无效！\n");
            wait_enter();
        }
    }
    free_drugs(head);
    clear_screen();
}

/* 添加药房 */
void add_pharmacy()
{
    Pharmacy *head = load_pharmacies_from_file();
    Pharmacy *new_node = (Pharmacy *)malloc(sizeof(Pharmacy));
    if (!new_node)
    {
        printf("内存分配失败！\n");
        free_pharmacies(head);
        wait_enter();
        return;
    }

    int id_num = generate_next_pharmacy_id(head);
    snprintf(new_node->id, sizeof(new_node->id), "PH%04d", id_num);

    while (1)
    {
        printf("请输入药房名称(汉字 | 输入0返回): ");
        safe_input(new_node->name, sizeof(new_node->name));
        if (strcmp(new_node->name, "0") == 0)
        {
            free(new_node);
            free_pharmacies(head);
            return;
        }
        if (new_node->name[0] == '\0')
        {
            printf("药房名称不能为空！\n");
            continue;
        }
        if (!is_all_chinese_utf8(new_node->name))
        {
            printf("药房名称只能包含汉字！\n");
            continue;
        }
        break;
    }

    while (1)
    {
        printf("请输入药房位置(汉字 | 输入0返回): ");
        safe_input(new_node->location, sizeof(new_node->location));
        if (strcmp(new_node->location, "0") == 0)
        {
            free(new_node);
            free_pharmacies(head);
            return;
        }
        if (new_node->location[0] == '\0')
        {
            printf("位置不能为空！\n");
            continue;
        }
        if (!is_all_chinese_utf8(new_node->location))
        {
            printf("位置只能包含汉字！\n");
            continue;
        }
        break;
    }

    new_node->next = NULL;
    if (!head)
        head = new_node;
    else
    {
        Pharmacy *tail = head;
        while (tail->next)
            tail = tail->next;
        tail->next = new_node;
    }

    if (save_pharmacies_to_file(head) != 0)
        printf("保存药房信息失败！\n");
    else
        printf("药房添加成功！分配ID: %s\n", new_node->id);
    free_pharmacies(head);
    wait_enter();
    clear_screen();
}

/*
 * 删除药房
 * 级联操作: 同时清除该药房在 pharmacy_drugs 表中的所有库存关联记录
 * 涉及两张表联动: pharmacies + pharmacy_drugs
 * 保存失败时回滚
 */
void delete_pharmacy()
{
    char id[MAX_ID_LEN];
    printf("请输入要删除的药房ID(输入0返回): ");
    safe_input(id, sizeof(id));
    if (strcmp(id, "0") == 0)
    {
        clear_screen();
        return;
    }
    if (id[0] == '\0')
    {
        printf("药房ID不能为空！\n");
        wait_enter();
        clear_screen();
        return;
    }

    Pharmacy *head = load_pharmacies_from_file();
    Pharmacy *prev = NULL, *cur = head;

    while (cur)
    {
        if (strcmp(cur->id, id) == 0)
        {
            int id_w, nm_w, loc_w;
            calc_pharmacy_width(head, &id_w, &nm_w, &loc_w);
            clear_screen();
            printf("找到药房:\n");
            print_pharmacy_header(id_w, nm_w, loc_w);
            print_pharmacy(cur, id_w, nm_w, loc_w);
            print_pharmacy_line(id_w, nm_w, loc_w);

            char confirm[16];
            printf("\n确认删除该药房及其名下的所有药品记录吗？(y/n): ");
            safe_input(confirm, sizeof(confirm));
            if (strcmp(confirm, "y") != 0 && strcmp(confirm, "Y") != 0)
            {
                printf("已取消删除。\n");
                free_pharmacies(head);
                wait_enter();
                clear_screen();
                return;
            }

            PharmacyDrug *pd_head = load_pharmacy_drugs_from_file();
            PharmacyDrug *pd_prev = NULL, *pd_cur = pd_head;
            PharmacyDrug *removed_pd_head = NULL, *removed_pd_tail = NULL; // 暂存被删库存关联，失败时直接恢复
            while (pd_cur)
            {
                if (strcmp(pd_cur->pharmacy_id, id) == 0)
                {
                    PharmacyDrug *tmp = pd_cur;
                    if (pd_prev)
                        pd_prev->next = pd_cur->next;
                    else
                        pd_head = pd_cur->next;
                    pd_cur = pd_cur->next;
                    tmp->next = NULL;
                    if (!removed_pd_head)
                        removed_pd_head = removed_pd_tail = tmp;
                    else
                    {
                        removed_pd_tail->next = tmp;
                        removed_pd_tail = tmp;
                    }
                }
                else
                {
                    pd_prev = pd_cur;
                    pd_cur = pd_cur->next;
                }
            }
            Pharmacy *deleted_pharmacy = cur; // 主药房节点延迟释放，等待双文件提交成功
            if (prev)
                prev->next = cur->next;
            else
                head = cur->next;
            deleted_pharmacy->next = NULL;

            /* 先保存关联表，再保存主药房表 */
            int s_pd = save_pharmacy_drugs_to_file(pd_head);
            int s_p = save_pharmacies_to_file(head);
            if (s_pd == 0 && s_p == 0)
            {
                while (removed_pd_head)
                {
                    PharmacyDrug *tmp = removed_pd_head;
                    removed_pd_head = removed_pd_head->next;
                    free(tmp);
                }
                free(deleted_pharmacy);
                printf("药房删除成功！\n");
            }
            else
            {
                /* 回滚药房链 */
                if (prev)
                {
                    deleted_pharmacy->next = prev->next;
                    prev->next = deleted_pharmacy;
                }
                else
                {
                    deleted_pharmacy->next = head;
                    head = deleted_pharmacy;
                }

                /* 回滚库存关联链 */
                if (removed_pd_head)
                {
                    removed_pd_tail->next = pd_head;
                    pd_head = removed_pd_head;
                }

                /* 将回滚后的状态持久化 */
                int rb1 = save_pharmacy_drugs_to_file(pd_head);
                int rb2 = save_pharmacies_to_file(head);
                printf("删除失败：保存文件失败。");
                if (rb1 == 0 && rb2 == 0)
                    printf("已回滚。\n");
                else
                    printf("且回滚失败，请立即检查数据文件。\n");
            }

            free_pharmacy_drugs(pd_head);
            free_pharmacies(head);
            wait_enter();
            clear_screen();
            return;
        }
        prev = cur;
        cur = cur->next;
    }
    printf("未找到该药房！\n");
    free_pharmacies(head);
    wait_enter();
    clear_screen();
}

/* 查询药房信息 */
void query_pharmacy()
{
    Pharmacy *head = load_pharmacies_from_file();
    if (!head)
    {
        printf("暂无药房数据！\n");
        wait_enter();
        clear_screen();
        return;
    }

    char query[MAX_NAME_LEN];
    printf("请输入查询关键字(ID或名称，输入0返回): ");
    safe_input(query, sizeof(query));
    if (strcmp(query, "0") == 0)
    {
        free_pharmacies(head);
        clear_screen();
        return;
    }
    if (query[0] == '\0')
    {
        printf("关键字不能为空！\n");
        free_pharmacies(head);
        wait_enter();
        clear_screen();
        return;
    }

    int id_w, nm_w, loc_w;
    calc_pharmacy_width(head, &id_w, &nm_w, &loc_w);

    /* 先统计 */
    int found = 0;
    for (Pharmacy *cur = head; cur; cur = cur->next)
    {
        if (strstr(cur->id, query) != NULL || strstr(cur->name, query) != NULL)
        {
            found = 1;
            break;
        }
    }

    if (!found)
    {
        printf("未找到匹配的药房。\n");
    }
    else
    {
        print_pharmacy_header(id_w, nm_w, loc_w);
        for (Pharmacy *cur = head; cur; cur = cur->next)
        {
            if (strstr(cur->id, query) != NULL || strstr(cur->name, query) != NULL)
                print_pharmacy(cur, id_w, nm_w, loc_w);
        }
        print_pharmacy_line(id_w, nm_w, loc_w);
    }
    free_pharmacies(head);
    wait_enter();
    clear_screen();
}

/* 显示所有药房 */
void show_all_pharmacies()
{
    Pharmacy *head = load_pharmacies_from_file();
    if (!head)
    {
        printf("暂无药房数据！\n");
        wait_enter();
        clear_screen();
        return;
    }

    int page_size = PAGE_SIZE;
    int total = count_pharmacies(head);
    int total_pages = (total + page_size - 1) / page_size;
    int current_page = 1;

    int id_w, nm_w, loc_w;
    calc_pharmacy_width(head, &id_w, &nm_w, &loc_w);

    while (1)
    {
        clear_screen();
        printf("===== 药房列表（第 %d/%d 页）=====\n", current_page, total_pages);
        print_pharmacy_header(id_w, nm_w, loc_w);

        int start = (current_page - 1) * page_size;
        Pharmacy *cur = get_nth_pharmacy(head, start);

        for (int i = 0; i < page_size && cur; i++)
        {
            print_pharmacy(cur, id_w, nm_w, loc_w);
            cur = cur->next;
        }
        print_pharmacy_line(id_w, nm_w, loc_w);

        printf("\n[n]下一页  [p]上一页  [q]退出\n> ");
        char buf[MAX_INPUT_LEN];
        safe_input(buf, sizeof(buf));

        if (strcmp(buf, "n") == 0 || strcmp(buf, "N") == 0)
        {
            if (current_page < total_pages)
                current_page++;
        }
        else if (strcmp(buf, "p") == 0 || strcmp(buf, "P") == 0)
        {
            if (current_page > 1)
                current_page--;
        }
        else if (strcmp(buf, "q") == 0 || strcmp(buf, "Q") == 0)
            break;
        else
        {
            printf("输入无效！\n");
            wait_enter();
        }
    }
    free_pharmacies(head);
    clear_screen();
}

/*
 * 药房药品管理
 */

/*
 * 药房入库
 * 涉及两张表联动: pharmacy_drugs(药房库存) + drugs(药品总库存)
 * 若药房已有该药品则追加数量, 否则新建关联记录
 * 保存失败时回滚两张表到操作前状态
 */
void stock_in_pharmacy()
{
    Pharmacy *p_head = load_pharmacies_from_file();
    Drug *d_head = load_drugs_from_file();
    PharmacyDrug *pd_head = load_pharmacy_drugs_from_file();

    if (!p_head || !d_head)
    {
        printf("基础数据不足(缺少药房或药品数据)！\n");
        free_pharmacies(p_head);
        free_drugs(d_head);
        free_pharmacy_drugs(pd_head);
        wait_enter();
        return;
    }

    char p_id[MAX_ID_LEN], d_id[MAX_ID_LEN], qty_str[32];

    print_pharmacy_hint();
    printf("请输入目标药房ID(输入0返回): ");
    safe_input(p_id, sizeof(p_id));
    if (strcmp(p_id, "0") == 0)
        goto cleanup;
    if (p_id[0] == '\0')
    {
        printf("药房ID不能为空！\n");
        wait_enter();
        goto cleanup;
    }

    Pharmacy *target_p = find_pharmacy_by_id(p_head, p_id);
    if (!target_p)
    {
        printf("未找到该药房！\n");
        wait_enter();
        goto cleanup;
    }

    printf("请输入要入库的药品ID(输入0返回): ");
    safe_input(d_id, sizeof(d_id));
    if (strcmp(d_id, "0") == 0)
        goto cleanup;
    if (d_id[0] == '\0')
    {
        printf("药品ID不能为空！\n");
        wait_enter();
        goto cleanup;
    }

    Drug *target_d = find_drug_by_id(d_head, d_id);
    if (!target_d)
    {
        printf("未找到该药品！\n");
        wait_enter();
        goto cleanup;
    }

    printf("请输入入库数量: ");
    safe_input(qty_str, sizeof(qty_str));
    int add_qty = 0;
    if (!parse_nonnegative_int_strict(qty_str, &add_qty) || add_qty <= 0)
    {
        printf("输入无效！\n");
        wait_enter();
        goto cleanup;
    }

    int found = 0; // 该药在目标药房是否已有库存记录
    PharmacyDrug *target_pd = NULL;
    for (PharmacyDrug *pd = pd_head; pd; pd = pd->next)
    {
        if (strcmp(pd->drug_id, d_id) == 0 && strcmp(pd->pharmacy_id, p_id) == 0)
        {
            target_pd = pd;
            found = 1;
            break;
        }
    }

    if (!found)
    {
        PharmacyDrug *new_pd = (PharmacyDrug *)malloc(sizeof(PharmacyDrug));
        if (!new_pd)
        {
            printf("内存分配失败！\n");
            wait_enter();
            goto cleanup;
        }
        memset(new_pd, 0, sizeof(PharmacyDrug));
        strncpy(new_pd->pharmacy_id, p_id, sizeof(new_pd->pharmacy_id) - 1);
        new_pd->pharmacy_id[sizeof(new_pd->pharmacy_id) - 1] = '\0';
        strncpy(new_pd->drug_id, d_id, sizeof(new_pd->drug_id) - 1);
        new_pd->drug_id[sizeof(new_pd->drug_id) - 1] = '\0';
        new_pd->quantity = add_qty;
        new_pd->next = NULL;
        target_pd = new_pd;

        if (!pd_head)
            pd_head = new_pd;
        else
        {
            PharmacyDrug *tail = pd_head;
            while (tail->next)
                tail = tail->next;
            tail->next = new_pd;
        }
    }

    /* 第一步: 快照修改前的库存数值 */
    int old_stock = target_d->stock;
    int old_pd_qty = found ? target_pd->quantity : 0;

    /* 第二步: 修改内存中的库存 */
    if (found)
        target_pd->quantity += add_qty;
    target_d->stock += add_qty;

    /* 第三步: 顺序保存两张表 */
    int pd_save_rc = save_pharmacy_drugs_to_file(pd_head);
    int d_save_rc = save_drugs_to_file(d_head);

    if (pd_save_rc != 0 || d_save_rc != 0)
    {
        /* 第四步(失败回滚): 从快照恢复库存数值 */
        target_d->stock = old_stock;
        if (found)
        {
            target_pd->quantity = old_pd_qty;
        }
        else
        {
            /* 新建节点场景: 回滚时需把刚插入节点从链表摘除 */
            PharmacyDrug *cur = pd_head, *prev = NULL;
            while (cur)
            {
                if (cur == target_pd)
                {
                    if (prev)
                        prev->next = cur->next;
                    else
                        pd_head = cur->next;
                    free(cur);
                    break;
                }
                prev = cur;
                cur = cur->next;
            }
        }

        /* 第五步: 将恢复后的状态重新写入全部文件 */
        int rb_pd = save_pharmacy_drugs_to_file(pd_head);
        int rb_d = save_drugs_to_file(d_head);

        printf("入库失败：保存文件失败。");
        if (rb_pd == 0 && rb_d == 0)
            printf("已回滚。\n");
        else
            printf("且回滚失败，请立即检查数据文件。\n");
        wait_enter();
        goto cleanup;
    }

    printf("入库成功！药房 [%s] 新增药品 [%s] %d 件，最新总库存: %d\n", target_p->name, target_d->generic_name, add_qty,
           target_d->stock);
    wait_enter();

cleanup:
    free_pharmacies(p_head);
    free_drugs(d_head);
    free_pharmacy_drugs(pd_head);
    clear_screen();
}

/*
 * 处方发药(医生/管理员均可调用)
 * 业务流程: 选择处方 -> 选择药房 -> 校验库存 -> 扣减药房库存和药品总库存
 * 涉及两张表联动: pharmacy_drugs + drugs
 * 保存失败时回滚扣减操作
 */
void dispense_prescription_drug()
{
    if (!g_session.logged_in)
    {
        printf("系统错误：请先登录账号！\n");
        wait_enter();
        return;
    }

    /* 1. 严格校验：只有医生可以发药 */
    if (strcmp(g_session.role, "doctor") != 0)
    {
        printf("发药拒绝：只有医生本人具有处方发药权限！\n");
        wait_enter();
        clear_screen();
        return;
    }

    Doctor *doc_head = load_doctors_from_file();
    Doctor *me = find_doctor_by_d_id(doc_head, g_session.user_id);
    if (!me)
    {
        printf("系统错误：找不到当前医生的信息！\n");
        free_doctors(doc_head);
        wait_enter();
        clear_screen();
        return;
    }

    char my_dept[MAX_NAME_LEN];
    strcpy(my_dept, me->department);

    Drug *d_head = load_drugs_from_file();
    Prescription *pr_head = load_prescriptions_from_file();
    Pharmacy *p_head = load_pharmacies_from_file();
    PharmacyDrug *pd_head = load_pharmacy_drugs_from_file();
    Patient *patient_head = load_patients_from_file(); // 为打印处方准备

    clear_screen();
    printf("===== 处方发药业务 (当前医生: %s, 所属科室: %s) =====\n", me->name, my_dept);
    printf("\n以下是您开具的处方列表：\n");

    /* 2. 筛选并打印当前医生名下的处方 */
    int pr_w, visit_w, d_w, p_w, drug_w, dose_w, freq_w;
    calc_prescription_width(pr_head, patient_head, doc_head, d_head, &pr_w, &visit_w, &d_w, &p_w, &drug_w, &dose_w,
                            &freq_w);

    /* 先统计 */
    int pr_count = 0;
    for (Prescription *c = pr_head; c; c = c->next)
    {
        if (strcmp(c->d_id, g_session.user_id) == 0)
            pr_count++;
    }

    if (pr_count == 0)
    {
        printf("  提示：您当前名下没有任何处方记录。\n");
        wait_enter();
        goto cleanup;
    }

    print_prescription_header(pr_w, visit_w, d_w, p_w, drug_w, dose_w, freq_w);
    for (Prescription *c = pr_head; c; c = c->next)
    {
        if (strcmp(c->d_id, g_session.user_id) == 0)
            print_prescription(c, patient_head, doc_head, d_head, pr_w, visit_w, d_w, p_w, drug_w, dose_w, freq_w);
    }
    print_prescription_line(pr_w, visit_w, d_w, p_w, drug_w, dose_w, freq_w);

    /* 3. 输入待履行的处方ID */
    char pr_id[MAX_ID_LEN];
    printf("\n请输入需履行的处方ID(输入0返回): ");
    safe_input(pr_id, sizeof(pr_id));
    if (strcmp(pr_id, "0") == 0 || pr_id[0] == '\0')
        goto cleanup;

    /* 寻找处方 */
    Prescription *pr = NULL;
    for (Prescription *c = pr_head; c; c = c->next)
    {
        if (strcmp(c->pr_id, pr_id) == 0)
        {
            pr = c;
            break;
        }
    }

    if (!pr)
    {
        printf("系统提示：未查找到该处方记录！\n");
        wait_enter();
        goto cleanup;
    }

    /* 4. 二次拦截：强制校验该处方是否属于当前医生 */
    if (strcmp(pr->d_id, g_session.user_id) != 0)
    {
        printf("\n【拦截告警】发药拒绝！您只能履行自己开具的处方。\n");
        wait_enter();
        goto cleanup;
    }

    Drug *target_drug = find_drug_by_id(d_head, pr->drug_id);
    if (!target_drug)
    {
        printf("系统提示：该处方包含的药品不存在(ID: %s)！\n", pr->drug_id);
        wait_enter();
        goto cleanup;
    }

    /* 5. 科室权限校验拦截 */
    if (strcmp(target_drug->department, "通用") != 0 && strcmp(target_drug->department, my_dept) != 0)
    {
        printf("\n【拦截告警】发药拒绝！\n");
        printf("该药品 [%s] 专属科室为 [%s]，不属于您的科室 [%s] 或 通用药！\n", target_drug->generic_name,
               target_drug->department, my_dept);
        wait_enter();
        goto cleanup;
    }

    printf("\n>>> 处方核对成功：需发药品 [%s] (处方医嘱剂量: %s)\n", target_drug->generic_name, pr->dose);
    printf("系统查询到该药品可用库存分布如下：\n");

    int found_stock = 0;
    for (PharmacyDrug *pd = pd_head; pd; pd = pd->next)
    {
        if (strcmp(pd->drug_id, target_drug->id) == 0 && pd->quantity > 0)
        {
            Pharmacy *phy = find_pharmacy_by_id(p_head, pd->pharmacy_id);
            printf(" - 药房 [%s] %s : 当前剩余 %d 件\n", pd->pharmacy_id, phy ? phy->name : "未知区域", pd->quantity);
            found_stock = 1;
        }
    }

    if (!found_stock)
    {
        printf("\n抱歉，该药品在所有药房均无可用库存，无法发药！\n");
        wait_enter();
        goto cleanup;
    }

    /* 6. 执行出库扣减 */
    char phy_id[MAX_ID_LEN];
    print_pharmacy_hint();
    printf("\n请指定扣减库存的出库药房ID(输入0取消): ");
    safe_input(phy_id, sizeof(phy_id));
    if (strcmp(phy_id, "0") == 0 || phy_id[0] == '\0')
        goto cleanup;

    PharmacyDrug *target_pd = NULL;
    for (PharmacyDrug *pd = pd_head; pd; pd = pd->next)
    {
        if (strcmp(pd->pharmacy_id, phy_id) == 0 && strcmp(pd->drug_id, target_drug->id) == 0)
        {
            target_pd = pd;
            break;
        }
    }

    if (!target_pd)
    {
        printf("该药房没有此药品的库存记录！\n");
        wait_enter();
        goto cleanup;
    }

    char qty_str[32];
    printf("请输入确认发药扣减数量: ");
    safe_input(qty_str, sizeof(qty_str));
    int qty = 0;

    if (!parse_nonnegative_int_strict(qty_str, &qty) || qty <= 0)
    {
        printf("发药数量无效！\n");
        wait_enter();
        goto cleanup;
    }
    if (qty > target_pd->quantity)
    {
        printf("出库失败！该药房该药仅剩 %d 件，库存不足！\n", target_pd->quantity);
        wait_enter();
        goto cleanup;
    }
    /* 检查全局药品库存是否充足 */
    if (qty > target_drug->stock)
    {
        printf("出库失败！该药品全局库存仅剩 %d 件，库存不足！\n", target_drug->stock);
        wait_enter();
        goto cleanup;
    }

    /* 第一步: 快照扣减前的库存 */
    int old_pd_qty = target_pd->quantity;
    int old_stock = target_drug->stock;

    /* 第二步: 扣减药房库存和药品总库存 */
    target_pd->quantity -= qty;
    target_drug->stock -= qty;

    /* 第三步: 顺序保存两张表 */
    int pd_save_rc = save_pharmacy_drugs_to_file(pd_head);
    int d_save_rc = save_drugs_to_file(d_head);

    if (pd_save_rc == 0 && d_save_rc == 0)
        printf("\n处方发药成功！已从指定药房扣减库存。\n");
    else
    {
        /* 第四步(失败回滚): 从快照恢复库存并重新写入文件 */
        target_pd->quantity = old_pd_qty;
        target_drug->stock = old_stock;

        int rb_pd = save_pharmacy_drugs_to_file(pd_head);
        int rb_d = save_drugs_to_file(d_head);

        printf("发药失败：保存文件失败。");
        if (rb_pd == 0 && rb_d == 0)
            printf("已回滚。\n");
        else
            printf("且回滚失败，请立即检查数据文件。\n");
    }
    wait_enter();

cleanup:
    free_doctors(doc_head);
    free_drugs(d_head);
    free_prescriptions(pr_head);
    free_pharmacies(p_head);
    free_pharmacy_drugs(pd_head);
    if (patient_head)
        free_patients(patient_head);
    clear_screen();
}

/* 药房库存查询 */
void show_pharmacy_drugs()
{
    Pharmacy *p_head = load_pharmacies_from_file();
    Drug *d_head = load_drugs_from_file();
    PharmacyDrug *pd_head = load_pharmacy_drugs_from_file();

    if (!p_head || !d_head || !pd_head)
    {
        printf("数据不足无法查询药房库存！\n");
        free_pharmacies(p_head);
        free_drugs(d_head);
        free_pharmacy_drugs(pd_head);
        wait_enter();
        return;
    }

    char p_id[MAX_ID_LEN];
    print_pharmacy_hint();
    printf("请输入目标药房ID(输入0返回): ");
    safe_input(p_id, sizeof(p_id));
    if (strcmp(p_id, "0") == 0)
        goto cleanup;
    if (p_id[0] == '\0')
    {
        printf("药房ID不能为空！\n");
        wait_enter();
        goto cleanup;
    }

    Pharmacy *target_p = find_pharmacy_by_id(p_head, p_id);
    if (!target_p)
    {
        printf("未找到该药房！\n");
        wait_enter();
        goto cleanup;
    }

    int id_w, gn_w, tn_w, al_w, pr_w, st_w, dept_w;
    calc_drug_width(d_head, &id_w, &gn_w, &tn_w, &al_w, &pr_w, &st_w, &dept_w);

    clear_screen();
    printf("===== [%s - %s] 库存明细列表 =====\n", target_p->id, target_p->name);

    /* 先统计 */
    int found = 0;
    for (PharmacyDrug *pd = pd_head; pd; pd = pd->next)
    {
        if (strcmp(pd->pharmacy_id, p_id) == 0 && pd->quantity > 0)
        {
            found = 1;
            break;
        }
    }

    if (!found)
    {
        printf("该药房内暂无任何有库存的药品！\n");
    }
    else
    {
        print_drug_header(id_w, gn_w, tn_w, al_w, pr_w, st_w, dept_w);
        for (PharmacyDrug *pd = pd_head; pd; pd = pd->next)
        {
            if (strcmp(pd->pharmacy_id, p_id) == 0 && pd->quantity > 0)
            {
                Drug *drug = find_drug_by_id(d_head, pd->drug_id);
                if (drug)
                {
                    Drug temp = *drug;
                    temp.stock = pd->quantity;
                    print_drug(&temp, id_w, gn_w, tn_w, al_w, pr_w, st_w, dept_w);
                }
            }
        }
        print_drug_line(id_w, gn_w, tn_w, al_w, pr_w, st_w, dept_w);
    }
    wait_enter();

cleanup:
    free_pharmacies(p_head);
    free_drugs(d_head);
    free_pharmacy_drugs(pd_head);
    clear_screen();
}