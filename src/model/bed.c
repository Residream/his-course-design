/*
 * 床位模块
 */
#include "model/bed.h"
#include "core/utils.h"
#include <limits.h>

/*
 * 床位基础功能
 */

/* 从文件中加载床位数据，文件格式: bed_id|ward_id|bed_no|status(0空闲/1占用) */
Bed *load_beds_from_file(void)
{
    FILE *fp = fopen(BEDS_FILE, "r");
    if (!fp)
        return NULL;

    Bed *head = NULL, *tail = NULL;
    char line[MAX_LINE_LEN];

    while (fgets(line, sizeof(line), fp))
    {
        trim_newline(line);
        if (!line[0])
            continue;

        Bed *node = (Bed *)malloc(sizeof(Bed));
        if (!node)
        {
            fclose(fp);
            free_beds(head);
            return NULL;
        }
        memset(node, 0, sizeof(Bed));

        /* 获取bed_id */
        char *token = strtok(line, "|");
        if (!token)
        {
            free(node);
            continue;
        }
        strncpy(node->bed_id, token, sizeof(node->bed_id) - 1);

        /* 获取ward_id */
        token = strtok(NULL, "|");
        if (!token)
        {
            free(node);
            continue;
        }
        strncpy(node->ward_id, token, sizeof(node->ward_id) - 1);

        /* 获取bed_no */
        token = strtok(NULL, "|");
        if (!token)
        {
            free(node);
            continue;
        }
        char *endptr = NULL;
        long bed_no_val = strtol(token, &endptr, 10); // 使用strtol把字符串转换成十进制，endptr会指向未解析部分
        if (token[0] == '\0' || *endptr != '\0' || bed_no_val <= 0 ||
            bed_no_val > INT_MAX) // 字符串为空、没有完整解析、非正数、超过int范围都视为无效
        {
            free(node);
            continue;
        }
        node->bed_no = (int)bed_no_val;

        /* 获取status */
        token = strtok(NULL, "|");
        if (!token)
        {
            free(node);
            continue;
        }
        endptr = NULL;
        long status_val = strtol(token, &endptr, 10); // 使用strtol把字符串转换成十进制，endptr会指向未解析部分
        if (token[0] == '\0' || *endptr != '\0' ||
            (status_val != 0 && status_val != 1)) // 字符串为空、没有完整解析、非0非1都视为无效
        {
            free(node);
            continue;
        }
        node->status = (int)status_val;

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

/* 将床位数据保存到文件 */
int save_beds_to_file(Bed *head)
{
    char tmp_path[MAX_LINE_LEN];
    FILE *fp = safe_fopen_tmp(BEDS_FILE, tmp_path, sizeof(tmp_path)); // 打开临时文件写入
    if (!fp)
        return -1;

    for (Bed *cur = head; cur; cur = cur->next)
        fprintf(fp, "%s|%s|%d|%d\n", cur->bed_id, cur->ward_id, cur->bed_no, cur->status);

    return safe_fclose_commit(fp, tmp_path, BEDS_FILE); // 关闭临时文件并原子替换目标文件，成功返回0，失败返回-1
}

/* 释放床位链表 */
void free_beds(Bed *head)
{
    while (head)
    {
        Bed *tmp = head;
        head = head->next;
        free(tmp);
    }
}

/* 按ID查找床位 */
Bed *find_bed_by_b_id(Bed *head, const char *bed_id)
{
    for (Bed *cur = head; cur; cur = cur->next)
        if (strcmp(cur->bed_id, bed_id) == 0)
            return cur;
    return NULL;
}

/* 查找首个空闲床位 */
Bed *find_first_free_bed(Bed *head)
{
    for (Bed *cur = head; cur; cur = cur->next)
        if (cur->status == 0)
            return cur;
    return NULL;
}

/* 查找病房内首个空闲床位 */
Bed *find_first_free_bed_in_ward(Bed *head, const char *ward_id)
{
    for (Bed *cur = head; cur; cur = cur->next)
        if (cur->status == 0 && strcmp(cur->ward_id, ward_id) == 0)
            return cur;
    return NULL;
}

/* 生成下一个床位ID(Bxxxx) */
int generate_next_bed_id(Bed *head)
{
    int max_id = 0;
    for (Bed *cur = head; cur; cur = cur->next)
    {
        const char *id = cur->bed_id;
        if (id[0] == 'B')
        {
            int valid = 1;
            for (int i = 1; id[i] != '\0'; i++)
            {
                if (id[i] < '0' || id[i] > '9')
                {
                    valid = 0;
                    break;
                }
            }
            if (valid && id[1] != '\0')
            {
                int n = atoi(id + 1);
                if (n > max_id)
                    max_id = n;
            }
        }
    }
    return max_id + 1;
}

/* 统计床位数量 */
int count_beds(Bed *head)
{
    int c = 0;
    while (head)
    {
        c++;
        head = head->next;
    }
    return c;
}

/* 统计空闲床位数 */
int count_free_beds(Bed *head)
{
    int c = 0;
    for (Bed *cur = head; cur; cur = cur->next)
        if (cur->status == 0) // 空闲
            c++;
    return c;
}

/* 打印床位信息行 */
void print_bed(Bed *b, Ward *w_head, int id_w, int ward_w, int no_w, int st_w)
{
    char no[16];
    snprintf(no, sizeof(no), "%d", b->bed_no);

    Ward *w = find_ward_by_w_id(w_head, b->ward_id);
    const char *ward_name = w ? w->name : b->ward_id;

    printf("| ");
    print_align(b->bed_id, id_w);
    printf(" | ");
    print_align(ward_name, ward_w);
    printf(" | ");
    print_align(no, no_w);
    printf(" | ");
    print_align(b->status == 0 ? "空闲" : "已占用", st_w);
    printf(" |\n");
}

/* 打印空闲床位信息行 */
void print_free_bed(Bed *b, Ward *w_head, int id_w, int ward_w, int no_w, int st_w)
{
    if (b->status != 0)
        return;
    print_bed(b, w_head, id_w, ward_w, no_w, st_w);
}

/* 打印床位表头 */
void print_bed_header(int id_w, int ward_w, int no_w, int st_w)
{
    print_bed_line(id_w, ward_w, no_w, st_w);
    printf("| ");
    print_align("床位ID", id_w);
    printf(" | ");
    print_align("病房", ward_w);
    printf(" | ");
    print_align("床号", no_w);
    printf(" | ");
    print_align("状态", st_w);
    printf(" |\n");
    print_bed_line(id_w, ward_w, no_w, st_w);
}

/* 打印床位分隔线 */
void print_bed_line(int id_w, int ward_w, int no_w, int st_w)
{
    printf("+");
    for (int i = 0; i < id_w + 2; i++)
        printf("-");
    printf("+");
    for (int i = 0; i < ward_w + 2; i++)
        printf("-");
    printf("+");
    for (int i = 0; i < no_w + 2; i++)
        printf("-");
    printf("+");
    for (int i = 0; i < st_w + 2; i++)
        printf("-");
    printf("+\n");
}

/* 计算床位表格列宽 */
void calc_bed_width(Bed *b_head, Ward *w_head, int *id_w, int *ward_w, int *no_w, int *st_w)
{
    *id_w = str_width("床位ID");
    *ward_w = str_width("病房");
    *no_w = str_width("床号");
    *st_w = str_width("状态");

    for (Bed *b = b_head; b; b = b->next)
    {
        int width;
        char no[16];
        snprintf(no, sizeof(no), "%d", b->bed_no);

        Ward *w = find_ward_by_w_id(w_head, b->ward_id);
        const char *ward_name = w ? w->name : b->ward_id;

        /* 计算各列宽度，更新最大宽度 */
        width = str_width(b->bed_id);
        if (width > *id_w)
            *id_w = width;
        width = str_width(ward_name);
        if (width > *ward_w)
            *ward_w = width;
        width = str_width(no);
        if (width > *no_w)
            *no_w = width;
        width = str_width(b->status == 0 ? "空闲" : "已占用");
        if (width > *st_w)
            *st_w = width;
    }
}

Bed *get_nth_bed(Bed *head, int n)
{
    Bed *cur = head;
    for (int i = 0; i < n && cur; i++)
        cur = cur->next;
    return cur;
}

/*
 *床位功能函数
 */

/* 创建床位 */
Bed create_bed(const char *bed_id, const char *ward_id, int bed_no, int status)
{
    Bed b;
    memset(&b, 0, sizeof(Bed));
    strncpy(b.bed_id, bed_id, sizeof(b.bed_id) - 1);
    strncpy(b.ward_id, ward_id, sizeof(b.ward_id) - 1);
    b.bed_no = bed_no;
    b.status = status;
    b.next = NULL;
    return b;
}

/* 尾插床位 */
void append_bed(Bed **head, Bed *new_bed)
{
    if (!head || !new_bed)
        return;
    new_bed->next = NULL;
    if (!*head)
    {
        *head = new_bed;
        return;
    }

    Bed *cur = *head;
    while (cur->next)
        cur = cur->next;
    cur->next = new_bed;
}

/*
 * 床位业务函数
 */

/*
 * 添加床位(管理员功能)
 * 支持批量添加: 输入数量后自动编号(从当前最大床位号+1起)
 * 涉及两张表联动: beds + wards(更新容量)
 * 批量添加中途失败时, 回滚已添加的床位节点并恢复病房容量
 */
void add_bed()
{
    Ward *w_head = load_wards_from_file();
    Bed *b_head = load_beds_from_file();

    if (!w_head)
    {
        printf("暂无病房数据，请先添加病房！\n");
        if (b_head)
            free_beds(b_head);
        wait_enter();
        clear_screen();
        return;
    }

    char ward_id[MAX_ID_LEN];

    while (1)
    {
        clear_screen();
        printf("===== 可用病房列表 =====\n");
        int id_w, name_w, type_w, dept_w, cap_w, occ_w;
        calc_ward_width(w_head, &id_w, &name_w, &type_w, &dept_w, &cap_w, &occ_w);
        print_ward_header(id_w, name_w, type_w, dept_w, cap_w, occ_w);
        for (Ward *w = w_head; w; w = w->next)
        {
            print_ward(w, id_w, name_w, type_w, dept_w, cap_w, occ_w);
        }
        print_ward_line(id_w, name_w, type_w, dept_w, cap_w, occ_w);

        printf("请输入要添加床位的病房ID(输入0返回): ");
        safe_input(ward_id, sizeof(ward_id));

        if (strcmp(ward_id, "0") == 0)
        {
            break;
        }
        if (ward_id[0] == '\0')
        {
            printf("输入错误！病房ID不能为空，请重新输入。\n");
            wait_enter();
            continue;
        }

        Ward *ward = find_ward_by_w_id(w_head, ward_id);
        if (!ward)
        {
            printf("病房ID不存在，请重新输入！\n");
            wait_enter();
            continue;
        }

        /* 获取床位数量 */
        int add_count = 0;
        char count_str[16];
        while (1)
        {
            printf("请输入要为该病房添加的床位数量(输入0返回): ");
            safe_input(count_str, sizeof(count_str));

            if (strcmp(count_str, "0") == 0)
            {
                add_count = 0;
                break;
            }

            char *endptr = NULL;
            long cnt_val = strtol(count_str, &endptr, 10); // 使用strtol把字符串转换成十进制，endptr会指向未解析部分
            if (*endptr != '\0' || cnt_val <= 0 ||
                cnt_val > INT_MAX) // 字符串没有被完全解析、非正数、超过int范围都视为无效
            {
                printf("输入错误！添加数量必须是正整数，请重新输入。\n");
                continue;
            }
            add_count = (int)cnt_val;
            break;
        }

        if (add_count == 0)
        {
            break;
        }

        int max_bed_no = 0;
        for (Bed *b = b_head; b != NULL; b = b->next)
        {
            if (strcmp(b->ward_id, ward_id) == 0 && b->bed_no > max_bed_no)
            {
                max_bed_no = b->bed_no;
            }
        }

        int next_id_num = generate_next_bed_id(b_head);
        int success_count = 0;

        for (int i = 0; i < add_count; i++)
        {
            char bed_id[MAX_ID_LEN];
            snprintf(bed_id, sizeof(bed_id), "B%04d", next_id_num + i);
            int bed_no = max_bed_no + 1 + i;

            Bed *new_node = (Bed *)malloc(sizeof(Bed));
            if (!new_node)
            {
                printf("内存分配失败！\n");
                break;
            }

            *new_node = create_bed(bed_id, ward_id, bed_no, 0);
            append_bed(&b_head, new_node);
            success_count++;
        }

        if (success_count > 0)
        {
            int old_capacity = ward->capacity; // 病房原容量，保存失败时恢复
            ward->capacity += success_count;

            int s1 = save_beds_to_file(b_head);
            int s2 = save_wards_to_file(w_head);
            if (s1 != 0 || s2 != 0)
            {
                ward->capacity = old_capacity;

                int rollback_upper = max_bed_no + success_count; // 仅回滚本次批量新增区间的床位节点
                Bed *cur = b_head, *prev = NULL;
                while (cur)
                {
                    if (strcmp(cur->ward_id, ward_id) == 0 && cur->bed_no > max_bed_no && cur->bed_no <= rollback_upper)
                    {
                        Bed *tmp = cur;
                        if (prev)
                            prev->next = cur->next;
                        else
                            b_head = cur->next;
                        cur = cur->next;
                        free(tmp);
                    }
                    else
                    {
                        prev = cur;
                        cur = cur->next;
                    }
                }

                int rb1 = save_beds_to_file(b_head);
                int rb2 = save_wards_to_file(w_head);
                printf("添加床位失败：保存文件失败。");
                if (rb1 == 0 && rb2 == 0)
                    printf("已回滚。\n");
                else
                    printf("且回滚失败，请立即检查数据文件。\n");
            }
            else
            {
                printf("添加成功！添加的床位数量为 %d 张。\n", success_count);
            }
        }

        wait_enter();
    }

    free_wards(w_head);
    free_beds(b_head);
    clear_screen();
}

/*
 * 删除床位(管理员功能)
 * 安全检查: 拒绝删除已占用的床位
 * 涉及两张表联动: beds + wards(减少容量)
 * 保存失败时回滚
 */
void delete_bed()
{
    Ward *w_head = load_wards_from_file();
    Bed *b_head = load_beds_from_file();

    if (!w_head || !b_head)
    {
        printf("暂无病房或床位数据！\n");
        if (w_head)
            free_wards(w_head);
        if (b_head)
            free_beds(b_head);
        wait_enter();
        clear_screen();
        return;
    }

    char ward_id[MAX_ID_LEN];
    char bed_id[MAX_ID_LEN];

    while (1)
    {
        clear_screen();
        printf("===== 选择要操作的病房 =====\n");
        int id_w, name_w, type_w, dept_w, cap_w, occ_w;
        calc_ward_width(w_head, &id_w, &name_w, &type_w, &dept_w, &cap_w, &occ_w);
        print_ward_header(id_w, name_w, type_w, dept_w, cap_w, occ_w);
        for (Ward *w = w_head; w; w = w->next)
        {
            print_ward(w, id_w, name_w, type_w, dept_w, cap_w, occ_w);
        }
        print_ward_line(id_w, name_w, type_w, dept_w, cap_w, occ_w);

        printf("请输入要操作的病房ID(输入0返回): ");
        safe_input(ward_id, sizeof(ward_id));

        if (strcmp(ward_id, "0") == 0)
        {
            break;
        }
        if (ward_id[0] == '\0')
        {
            printf("输入错误！病房ID不能为空。\n");
            wait_enter();
            clear_screen();
            continue;
        }

        Ward *ward = find_ward_by_w_id(w_head, ward_id);
        if (!ward)
        {
            printf("病房ID不存在，请重新输入！\n");
            wait_enter();
            clear_screen();
            continue;
        }

        while (1)
        {
            /* 先统计可删除的空闲床位数量 */
            int free_count = 0;
            for (Bed *b = b_head; b; b = b->next)
            {
                if (strcmp(b->ward_id, ward_id) == 0 && b->status == 0)
                    free_count++;
            }

            if (free_count == 0)
            {
                printf("该病房当前没有可删除的空闲床位！\n");
                wait_enter();
                break;
            }

            /* 有床位才打印表格 */
            clear_screen();
            printf("===== [%s] 可删除的空闲床位 =====\n", ward->name);
            int bed_id_w, ward_name_w, no_w, st_w;
            calc_bed_width(b_head, w_head, &bed_id_w, &ward_name_w, &no_w, &st_w);
            print_bed_header(bed_id_w, ward_name_w, no_w, st_w);
            for (Bed *b = b_head; b; b = b->next)
            {
                if (strcmp(b->ward_id, ward_id) == 0 && b->status == 0)
                    print_bed(b, w_head, bed_id_w, ward_name_w, no_w, st_w);
            }
            print_bed_line(bed_id_w, ward_name_w, no_w, st_w);

            if (free_count == 0)
            {
                printf("该病房当前没有可删除的空闲床位！\n");
                wait_enter();
                break;
            }

            printf("请输入要删除的床位ID(输入0返回上一步): ");
            safe_input(bed_id, sizeof(bed_id));

            if (strcmp(bed_id, "0") == 0)
            {
                break;
            }
            if (bed_id[0] == '\0')
            {
                printf("输入错误！床位ID不能为空。\n");
                wait_enter();
                clear_screen();
                continue;
            }

            Bed *bed = find_bed_by_b_id(b_head, bed_id);
            if (!bed || strcmp(bed->ward_id, ward_id) != 0 || bed->status != 0)
            {
                printf("输入错误：床位ID不存在，或不属于该病房，或已被占用！\n");
                wait_enter();
                clear_screen();
                continue;
            }

            Bed *cur = b_head, *prev = NULL;
            Bed *removed_node = NULL; // 延迟释放，保存失败可插回链表
            Bed *removed_prev = NULL;
            while (cur)
            {
                if (strcmp(cur->bed_id, bed_id) == 0)
                {
                    removed_node = cur;
                    removed_prev = prev;
                    if (prev)
                        prev->next = cur->next;
                    else
                        b_head = cur->next;
                    removed_node->next = NULL;
                    break;
                }
                prev = cur;
                cur = cur->next;
            }

            if (!removed_node)
            {
                printf("操作失败：未找到目标床位。\n");
                wait_enter();
                clear_screen();
                continue;
            }

            int old_capacity = ward->capacity; // 删除前容量快照，保存失败时恢复
            if (ward->capacity > 0)
                ward->capacity -= 1;

            int s1 = save_beds_to_file(b_head);
            int s2 = save_wards_to_file(w_head);
            if (s1 != 0 || s2 != 0)
            {
                /* 先恢复链表节点，再恢复容量 */
                if (removed_prev)
                {
                    removed_node->next = removed_prev->next;
                    removed_prev->next = removed_node;
                }
                else
                {
                    removed_node->next = b_head;
                    b_head = removed_node;
                }
                ward->capacity = old_capacity;

                int rb1 = save_beds_to_file(b_head);
                int rb2 = save_wards_to_file(w_head);
                printf("删除床位失败：保存文件失败。");
                if (rb1 == 0 && rb2 == 0)
                    printf("已回滚。\n");
                else
                    printf("且回滚失败，请立即检查数据文件。\n");
            }
            else
            {
                free(removed_node);
                printf("床位删除成功！病房 [%s] 容量已更新为 %d 张。\n", ward->name, ward->capacity);
            }

            wait_enter();
            break;
        }
    }

    free_wards(w_head);
    free_beds(b_head);
    clear_screen();
}

/* 显示所有床位 */
void show_all_beds()
{
    Bed *b_head = load_beds_from_file();
    if (!b_head)
    {
        printf("暂无床位记录！\n");
        wait_enter();
        clear_screen();
        return;
    }

    Ward *w_head = load_wards_from_file();
    if (!w_head)
    {
        printf("加载病房数据失败！\n");
        free_beds(b_head);
        wait_enter();
        clear_screen();
        return;
    }

    int page_size = PAGE_SIZE;
    int total = count_beds(b_head);
    int total_pages = (total + page_size - 1) / page_size;

    int current_page = 1;

    int id_w, ward_w, no_w, st_w;
    calc_bed_width(b_head, w_head, &id_w, &ward_w, &no_w, &st_w);

    while (1)
    {
        clear_screen();

        printf("===== 所有物理床位记录 (第 %d/%d 页) =====\n", current_page, total_pages);

        print_bed_header(id_w, ward_w, no_w, st_w);

        int start = (current_page - 1) * page_size;
        Bed *b_cur = get_nth_bed(b_head, start);

        for (int i = 0; i < page_size && b_cur; i++)
        {
            print_bed(b_cur, w_head, id_w, ward_w, no_w, st_w);
            b_cur = b_cur->next;
        }

        print_bed_line(id_w, ward_w, no_w, st_w);

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
        {
            break;
        }
        else
        {
            printf("输入无效，请重新输入！\n");
            wait_enter();
        }
    }
    free_beds(b_head);
    free_wards(w_head);
    clear_screen();
}