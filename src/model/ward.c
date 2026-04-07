/*
 * 病房模块
 */
#include "model/ward.h"
#include "model/bed.h"
#include "core/utils.h"

/*
 *病房基础功能
 */

/* 从文件中加载病房数据 */
Ward *load_wards_from_file(void)
{
    FILE *fp = fopen(WARDS_FILE, "r");
    if (!fp)
        return NULL;

    Ward *head = NULL, *tail = NULL;
    char line[MAX_LINE_LEN];

    while (fgets(line, sizeof(line), fp))
    {
        trim_newline(line);
        if (!line[0])
            continue;

        Ward *node = (Ward *)malloc(sizeof(Ward));
        if (!node)
        {
            fclose(fp);
            free_wards(head);
            return NULL;
        }
        memset(node, 0, sizeof(Ward));

        char *token = strtok(line, "|"); // ward_id
        if (!token)
        {
            free(node);
            continue;
        }
        strncpy(node->ward_id, token, sizeof(node->ward_id) - 1);

        token = strtok(NULL, "|"); // name
        if (!token)
        {
            free(node);
            continue;
        }
        strncpy(node->name, token, sizeof(node->name) - 1);

        token = strtok(NULL, "|"); // capacity
        if (!token)
        {
            free(node);
            continue;
        }
        node->capacity = atoi(token);

        token = strtok(NULL, "|"); // occupied
        if (!token)
        {
            free(node);
            continue;
        }
        node->occupied = atoi(token);

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

/* 保存病房数据 */
int save_wards_to_file(Ward *head)
{
    char tmp_path[MAX_LINE_LEN];
    FILE *fp = safe_fopen_tmp(WARDS_FILE, tmp_path, sizeof(tmp_path));
    if (!fp)
        return -1;

    for (Ward *cur = head; cur; cur = cur->next)
        fprintf(fp, "%s|%s|%d|%d\n", cur->ward_id, cur->name, cur->capacity, cur->occupied);

    return safe_fclose_commit(fp, tmp_path, WARDS_FILE);
}

/* 释放病房链表 */
void free_wards(Ward *head)
{
    while (head)
    {
        Ward *tmp = head;
        head = head->next;
        free(tmp);
    }
}

/* 按ID查找病房 */
Ward *find_ward_by_w_id(Ward *head, const char *ward_id)
{
    for (Ward *cur = head; cur; cur = cur->next)
        if (strcmp(cur->ward_id, ward_id) == 0)
            return cur;
    return NULL;
}

/* 按名称查找病房 */
Ward *find_ward_by_name(Ward *head, const char *name)
{
    for (Ward *cur = head; cur; cur = cur->next)
        if (strcmp(cur->name, name) == 0)
            return cur;
    return NULL;
}

/* 生成下一个病房ID */
int generate_next_ward_id(Ward *head)
{
    int max_id = 0;
    for (Ward *cur = head; cur; cur = cur->next)
    {
        if (cur->ward_id[0] == 'W')
        {
            int n = atoi(cur->ward_id + 1);
            if (n > max_id)
                max_id = n;
        }
    }
    return max_id + 1;
}

/* 统计病房数量 */
int count_wards(Ward *head)
{
    int cnt = 0;
    while (head)
    {
        cnt++;
        head = head->next;
    }
    return cnt;
}

/* 获取第n个病房节点 */
Ward *get_nth_ward(Ward *head, int n)
{
    int i = 0;
    while (head && i < n)
    {
        head = head->next;
        i++;
    }
    return head;
}

/* 打印病房信息行 */
void print_ward(Ward *w, int id_w, int name_w, int cap_w, int occ_w)
{
    char cap[16], occ[16];
    snprintf(cap, sizeof(cap), "%d", w->capacity);
    snprintf(occ, sizeof(occ), "%d", w->occupied);

    printf("| ");
    print_align(w->ward_id, id_w);
    printf(" | ");
    print_align(w->name, name_w);
    printf(" | ");
    print_align(cap, cap_w);
    printf(" | ");
    print_align(occ, occ_w);
    printf(" |\n");
}

/* 打印病房表头 */
void print_ward_header(int id_w, int name_w, int cap_w, int occ_w)
{
    print_ward_line(id_w, name_w, cap_w, occ_w);
    printf("| ");
    print_align("病房ID", id_w);
    printf(" | ");
    print_align("病房名称", name_w);
    printf(" | ");
    print_align("床位总数", cap_w);
    printf(" | ");
    print_align("已占用", occ_w);
    printf(" |\n");
    print_ward_line(id_w, name_w, cap_w, occ_w);
}

/* 打印病房分隔线 */
void print_ward_line(int id_w, int name_w, int cap_w, int occ_w)
{
    printf("+");
    for (int i = 0; i < id_w + 2; i++)
        printf("-");
    printf("+");
    for (int i = 0; i < name_w + 2; i++)
        printf("-");
    printf("+");
    for (int i = 0; i < cap_w + 2; i++)
        printf("-");
    printf("+");
    for (int i = 0; i < occ_w + 2; i++)
        printf("-");
    printf("+\n");
}

/* 计算病房表格列宽 */
void calc_ward_width(Ward *head, int *id_w, int *name_w, int *cap_w, int *occ_w)
{
    *id_w = str_width("病房ID");
    *name_w = str_width("病房名称");
    *cap_w = str_width("床位总数");
    *occ_w = str_width("已占用");

    for (Ward *w = head; w; w = w->next)
    {
        int width;
        char cap[16], occ[16];
        snprintf(cap, sizeof(cap), "%d", w->capacity);
        snprintf(occ, sizeof(occ), "%d", w->occupied);

        width = str_width(w->ward_id);
        if (width > *id_w)
            *id_w = width;
        width = str_width(w->name);
        if (width > *name_w)
            *name_w = width;
        width = str_width(cap);
        if (width > *cap_w)
            *cap_w = width;
        width = str_width(occ);
        if (width > *occ_w)
            *occ_w = width;
    }
}

/*
 * 病房功能函数
 */

/* 创建病房 */
Ward create_ward(const char *ward_id, const char *name, int capacity, int occupied)
{
    Ward w;
    memset(&w, 0, sizeof(Ward));
    strncpy(w.ward_id, ward_id, sizeof(w.ward_id) - 1);
    strncpy(w.name, name, sizeof(w.name) - 1);
    w.capacity = capacity;
    w.occupied = occupied;
    w.next = NULL;
    return w;
}

/* 尾插病房 */
void append_ward(Ward **head, Ward *new_ward)
{
    if (!head || !new_ward)
        return;
    new_ward->next = NULL;
    if (!*head)
    {
        *head = new_ward;
        return;
    }

    Ward *cur = *head;
    while (cur->next)
        cur = cur->next;
    cur->next = new_ward;
}

/*
 * 病房业务函数
 */

/* 添加病房 */
void add_ward()
{
    char name[MAX_NAME_LEN];
    int capacity;

    /* 病房名称 */
    while (1)
    {
        printf("请输入病房名称(输入0返回): ");
        safe_input(name, sizeof(name));

        if (strcmp(name, "0") == 0)
            return;

        if (name[0] == '\0')
        {
            printf("输入错误！病房名称不能为空，请重新输入。\n");
            continue;
        }
        if (!is_all_chinese_utf8(name))
        {
            printf("输入错误！病房名称只能包含汉字，请重新输入。\n");
            continue;
        }
        break;
    }

    /* 床位总数 */
    while (1)
    {
        char cap_str[16];
        printf("请输入床位总数(输入0返回): ");
        safe_input(cap_str, sizeof(cap_str));

        if (strcmp(cap_str, "0") == 0)
            return;

        capacity = atoi(cap_str);
        if (capacity <= 0)
        {
            printf("输入错误！床位总数必须是正整数，请重新输入。\n");
            continue;
        }
        break;
    }

    Ward *w_head = load_wards_from_file();
    int next_id = generate_next_ward_id(w_head);
    char ward_id[MAX_ID_LEN];
    snprintf(ward_id, sizeof(ward_id), "W%04d", next_id);

    Ward *new_node = (Ward *)malloc(sizeof(Ward));
    if (!new_node)
    {
        printf("内存分配失败！\n");
        free_wards(w_head);
        wait_enter();
        return;
    }

    *new_node = create_ward(ward_id, name, capacity, 0);
    append_ward(&w_head, new_node);

    if (save_wards_to_file(w_head) != 0)
        printf("保存病房信息失败！\n");
    else
        printf("病房添加成功！添加的病房ID是: %s\n", ward_id);

    free_wards(w_head);
    wait_enter();
    clear_screen();
}

/* 删除病房 */
void delete_ward()
{
    char ward_id[MAX_ID_LEN];
    printf("请输入要删除的病房ID(输入0返回): ");
    safe_input(ward_id, sizeof(ward_id));
    if (strcmp(ward_id, "0") == 0)
    {
        clear_screen();
        return;
    }

    Ward *w_head = load_wards_from_file();
    Ward *current = w_head, *prev = NULL;
    while (current)
    {
        if (strcmp(current->ward_id, ward_id) == 0)
        {
            if (current->occupied > 0)
            {
                printf("无法删除！该病房有患者入住。\n");
                free_wards(w_head);
                wait_enter();
                clear_screen();
                return;
            }

            printf("找到病房: %s (%s)，确定要删除吗？(y/n): ", current->name, current->ward_id);
            char confirm[MAX_INPUT_LEN];
            safe_input(confirm, sizeof(confirm));
            if (strcmp(confirm, "n") == 0 || strcmp(confirm, "N") == 0) // 用户输入n/N取消删除
            {
                printf("已取消删除。\n");
                free_wards(w_head);
                wait_enter();
                clear_screen();
                return;
            }
            else if (strcmp(confirm, "y") != 0 &&
                     strcmp(confirm, "Y") != 0) // 用户输入非y/Y/n/N的其他内容，视为无效输入并取消删除
            {
                printf("输入无效，已取消删除。\n");
                free_wards(w_head);
                wait_enter();
                clear_screen();
                return;
            }
            else // 用户输入y/Y确认删除
            {
                // 检查是否有关联的床位记录
                Bed *b_head = load_beds_from_file();
                int has_bed = 0;
                for (Bed *b = b_head; b; b = b->next)
                {
                    if (strcmp(b->ward_id, ward_id) == 0)
                    {
                        has_bed = 1;
                        break;
                    }
                }
                free_beds(b_head);

                if (has_bed)
                {
                    printf("无法删除！该病房下仍有关联的床位记录，请先删除相关床位。\n");
                    free_wards(w_head);
                    wait_enter();
                    clear_screen();
                    return;
                }

                if (prev)
                    prev->next = current->next;
                else
                    w_head = current->next;

                free(current);

                if (save_wards_to_file(w_head) != 0)
                    printf("保存病房信息失败！\n");
                else
                    printf("病房删除成功！\n");

                free_wards(w_head);
                wait_enter();
                clear_screen();
                return;
            }
        }
        prev = current;
        current = current->next;
    }
    printf("未找到指定ID的病房。\n");
    free_wards(w_head);
    wait_enter();
    clear_screen();
}

/* 显示所有病房 */
void show_all_wards()
{
    Ward *w_head = load_wards_from_file();
    if (!w_head)
    {
        printf("暂无病房数据！\n");
        wait_enter();
        clear_screen();
        return;
    }
    Ward *current = w_head;
    printf("所有病房信息:\n");
    int id_w, name_w, cap_w, occ_w;
    calc_ward_width(w_head, &id_w, &name_w, &cap_w, &occ_w);
    print_ward_header(id_w, name_w, cap_w, occ_w);
    while (current)
    {
        print_ward(current, id_w, name_w, cap_w, occ_w);
        current = current->next;
    }
    print_ward_line(id_w, name_w, cap_w, occ_w);
    free_wards(w_head);
    wait_enter();
    clear_screen();
}