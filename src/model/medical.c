/*

 * 医疗记录模块
   */
#include "model/medical.h"
#include "core/session.h"
#include "core/utils.h"
#include "model/bed.h"
#include "model/doctor.h"
#include "model/drug.h"
#include "model/exam.h"
#include "model/hospitalization.h"
#include "model/patient.h"
#include "model/prescription.h"
#include "model/registration.h"
#include "model/visit.h"
#include "model/ward.h"

/*

 * 医疗记录系统功能
   */

/*

 * 全程查询
   */
/* 患者就诊全程查阅 */
void query_patients_medical_records(void)
{
#define SEC_COUNT 5

    /* 一次性加载所有链表 */
    Patient *p_head = load_patients_from_file();
    Doctor *d_head = load_doctors_from_file();
    Registration *reg_head = load_registrations_from_file();
    Visit *v_head = load_visits_from_file();
    Exam *e_head = load_exams_from_file();
    Hospitalization *h_head = load_hospitalizations_from_file();
    Ward *ward_head = load_wards_from_file();
    Bed *bed_head = load_beds_from_file();
    Prescription *pr_head = load_prescriptions_from_file();
    Drug *drug_head = load_drugs_from_file();

    char p_id[MAX_ID_LEN];

    printf("请输入患者ID (输入0返回): ");
    safe_input(p_id, sizeof(p_id));

    if (strcmp(p_id, "0") == 0)
        goto cleanup;
    if (p_id[0] == '\0')
    {
        printf("患者ID不能为空！\n");
        wait_enter();
        goto cleanup;
    }

    {
        Patient *patient = find_patient_by_p_id(p_head, p_id);
        if (!patient)
        {
            printf("未找到该患者ID！\n");
            wait_enter();
            goto cleanup;
        }

        /* 检查该患者是否有任何挂号记录 */
        Registration *first_reg = find_registration_by_p_id(reg_head, p_id);
        if (!first_reg)
        {
            printf("该患者暂无任何就诊记录！\n");
            wait_enter();
            goto cleanup;
        }

        /* 做医生权限校验 */
        if (g_session.logged_in && strcmp(g_session.role, "doctor") == 0)
        {
            int has_visit = 0;
            for (Registration *r = first_reg; r; r = find_registration_by_p_id(r->next, p_id))
            {
                if (strcmp(r->d_id, g_session.user_id) == 0)
                {
                    has_visit = 1;
                    break;
                }
            }
            if (!has_visit)
            {
                printf("您没有权限查看该患者的医疗记录！\n");
                wait_enter();
                goto cleanup;
            }
        }

        clear_screen();

        /* 预计算全部列宽 */
        int id_w, name_w, gen_w, age_w;
        calc_patient_width(p_head, &id_w, &name_w, &gen_w, &age_w);

        int reg_w, rp_w, rd_w, rdept_w, rwhen_w, rst_w;
        calc_registration_width(reg_head, p_head, d_head, &reg_w, &rp_w, &rd_w, &rdept_w, &rwhen_w, &rst_w);

        int vis_w, vp_w, vd_w, vdept_w, vwhen_w, vst_w, vdiag_w;
        calc_visit_width(v_head, reg_head, p_head, d_head, &vis_w, &vp_w, &vd_w, &vdept_w, &vwhen_w, &vst_w, &vdiag_w);

        int exam_w, ev_w, ep_w, ed_w, edept_w, item_w, result_w;
        calc_exam_width(e_head, v_head, reg_head, p_head, d_head, &exam_w, &ev_w, &ep_w, &ed_w, &edept_w, &item_w,
                        &result_w);

        int hosp_w, hv_w, hp_w, ward_w, bed_w, in_w, out_w, hst_w;
        calc_hospitalization_width(h_head, v_head, reg_head, p_head, ward_head, bed_head, &hosp_w, &hv_w, &hp_w,
                                   &ward_w, &bed_w, &in_w, &out_w, &hst_w);

        int pr_w, prv_w, prd_w, prp_w, drug_w, dose_w, freq_w;
        calc_prescription_width(pr_head, p_head, d_head, drug_head, &pr_w, &prv_w, &prd_w, &prp_w, &drug_w, &dose_w,
                                &freq_w);

        /* 统计各分类记录条数 */
        int sec_totals[SEC_COUNT] = {0};

        for (Registration *r = find_registration_by_p_id(reg_head, p_id); r;
             r = find_registration_by_p_id(r->next, p_id))
            sec_totals[0]++;

        for (Visit *v = find_visit_by_p_id(v_head, reg_head, p_id); v; v = find_visit_by_p_id(v->next, reg_head, p_id))
            sec_totals[1]++;

        for (Exam *e = find_exam_by_p_id(e_head, v_head, reg_head, p_id); e;
             e = find_exam_by_p_id(e->next, v_head, reg_head, p_id))
            sec_totals[2]++;

        for (Hospitalization *h = find_hospitalization_by_p_id(h_head, p_id); h;
             h = find_hospitalization_by_p_id(h->next, p_id))
            sec_totals[3]++;

        for (Prescription *pr = find_prescription_by_p_id(pr_head, p_id); pr;
             pr = find_prescription_by_p_id(pr->next, p_id))
            sec_totals[4]++;

        const char *sec_names[SEC_COUNT] = {"挂号信息", "看诊信息", "检查信息", "住院信息", "处方信息"};

        int cur_sec = 0;
        int cur_page = 1;

        /* 分类分页浏览循环 */
        while (1)
        {
            clear_screen();

            printf("===== 患者信息 =====\n");
            print_patient_header(id_w, name_w, gen_w, age_w);
            print_patient(patient, id_w, name_w, gen_w, age_w);
            print_patient_line(id_w, name_w, gen_w, age_w);

            int total = sec_totals[cur_sec];
            int total_pages = total > 0 ? (total + PAGE_SIZE - 1) / PAGE_SIZE : 1;
            if (cur_page > total_pages)
                cur_page = total_pages;

            printf("\n===== %s (第 %d/%d 页，共 %d 条) =====\n", sec_names[cur_sec], cur_page, total_pages, total);

            int start = (cur_page - 1) * PAGE_SIZE;

            switch (cur_sec)
            {
            case 0:
            {
                print_registration_header(reg_w, rp_w, rd_w, rdept_w, rwhen_w, rst_w);
                Registration *r = find_registration_by_p_id(reg_head, p_id);
                for (int i = 0; i < start && r; i++)
                    r = find_registration_by_p_id(r->next, p_id);
                for (int i = 0; i < PAGE_SIZE && r; i++)
                {
                    print_registration(r, p_head, d_head, reg_w, rp_w, rd_w, rdept_w, rwhen_w, rst_w);
                    r = find_registration_by_p_id(r->next, p_id);
                }
                print_registration_line(reg_w, rp_w, rd_w, rdept_w, rwhen_w, rst_w);
                break;
            }
            case 1:
            {
                if (total == 0)
                {
                    printf("\n暂无看诊记录！\n");
                    break;
                }
                print_visit_header(vis_w, vp_w, vd_w, vdept_w, vwhen_w, vst_w, vdiag_w);
                Visit *v = find_visit_by_p_id(v_head, reg_head, p_id);
                for (int i = 0; i < start && v; i++)
                    v = find_visit_by_p_id(v->next, reg_head, p_id);
                for (int i = 0; i < PAGE_SIZE && v; i++)
                {
                    print_visit(v, reg_head, p_head, d_head, vis_w, vp_w, vd_w, vdept_w, vwhen_w, vst_w, vdiag_w);
                    v = find_visit_by_p_id(v->next, reg_head, p_id);
                }
                print_visit_line(vis_w, vp_w, vd_w, vdept_w, vwhen_w, vst_w, vdiag_w);
                break;
            }
            case 2:
            {
                if (total == 0)
                {
                    printf("\n暂无检查记录！\n");
                    break;
                }
                print_exam_header(exam_w, ev_w, ep_w, ed_w, edept_w, item_w, result_w);
                Exam *e = find_exam_by_p_id(e_head, v_head, reg_head, p_id);
                for (int i = 0; i < start && e; i++)
                    e = find_exam_by_p_id(e->next, v_head, reg_head, p_id);
                for (int i = 0; i < PAGE_SIZE && e; i++)
                {
                    print_exam(e, v_head, reg_head, p_head, d_head, exam_w, ev_w, ep_w, ed_w, edept_w, item_w,
                               result_w);
                    e = find_exam_by_p_id(e->next, v_head, reg_head, p_id);
                }
                print_exam_line(exam_w, ev_w, ep_w, ed_w, edept_w, item_w, result_w);
                break;
            }
            case 3:
            {
                if (total == 0)
                {
                    printf("\n暂无住院记录！\n");
                    break;
                }
                print_hospitalization_header(hosp_w, hv_w, hp_w, ward_w, bed_w, in_w, out_w, hst_w);
                Hospitalization *h = find_hospitalization_by_p_id(h_head, p_id);
                for (int i = 0; i < start && h; i++)
                    h = find_hospitalization_by_p_id(h->next, p_id);
                for (int i = 0; i < PAGE_SIZE && h; i++)
                {
                    print_hospitalization(h, v_head, reg_head, p_head, ward_head, bed_head, hosp_w, hv_w, hp_w, ward_w,
                                          bed_w, in_w, out_w, hst_w);
                    h = find_hospitalization_by_p_id(h->next, p_id);
                }
                print_hospitalization_line(hosp_w, hv_w, hp_w, ward_w, bed_w, in_w, out_w, hst_w);
                break;
            }
            case 4:
            {
                if (total == 0)
                {
                    printf("\n暂无处方记录！\n");
                    break;
                }
                print_prescription_header(pr_w, prv_w, prd_w, prp_w, drug_w, dose_w, freq_w);
                Prescription *pr = find_prescription_by_p_id(pr_head, p_id);
                for (int i = 0; i < start && pr; i++)
                    pr = find_prescription_by_p_id(pr->next, p_id);
                for (int i = 0; i < PAGE_SIZE && pr; i++)
                {
                    print_prescription(pr, p_head, d_head, drug_head, pr_w, prv_w, prd_w, prp_w, drug_w, dose_w,
                                       freq_w);
                    pr = find_prescription_by_p_id(pr->next, p_id);
                }
                print_prescription_line(pr_w, prv_w, prd_w, prp_w, drug_w, dose_w, freq_w);
                break;
            }
            }

            printf("\n当前类: ");
            for (int i = 0; i < SEC_COUNT; i++)
                printf(i == cur_sec ? "[%s] " : " %s  ", sec_names[i]);

            printf("\n[n]下一页  [p]上一页  [nn]下一类  [pp]上一类  [q]退出\n> ");

            char buf[MAX_INPUT_LEN];
            safe_input(buf, sizeof(buf));

            if (strcmp(buf, "n") == 0 || strcmp(buf, "N") == 0)
            {
                if (cur_page < total_pages)
                    cur_page++;
            }
            else if (strcmp(buf, "p") == 0 || strcmp(buf, "P") == 0)
            {
                if (cur_page > 1)
                    cur_page--;
            }
            else if (strcmp(buf, "nn") == 0 || strcmp(buf, "NN") == 0)
            {
                if (cur_sec < SEC_COUNT - 1)
                {
                    cur_sec++;
                    cur_page = 1;
                }
            }
            else if (strcmp(buf, "pp") == 0 || strcmp(buf, "PP") == 0)
            {
                if (cur_sec > 0)
                {
                    cur_sec--;
                    cur_page = 1;
                }
            }
            else if (strcmp(buf, "q") == 0 || strcmp(buf, "Q") == 0)
            {
                goto cleanup;
            }
            else
            {
                printf("输入无效，请重新输入！\n");
                wait_enter();
            }
        }
    }

cleanup:
    free_patients(p_head);
    free_doctors(d_head);
    free_registrations(reg_head);
    free_visits(v_head);
    free_exams(e_head);
    free_hospitalizations(h_head);
    free_wards(ward_head);
    free_beds(bed_head);
    free_prescriptions(pr_head);
    free_drugs(drug_head);

#undef SEC_COUNT
}

/*
 *分类打印
 */
/* 分类打印医疗记录 */
void print_medical_records_by_category(void)
{
#define SEC_COUNT 5

    /* 一次性加载所有链表 */
    Patient *p_head = load_patients_from_file();
    Doctor *d_head = load_doctors_from_file();
    Registration *reg_head = load_registrations_from_file();
    Visit *v_head = load_visits_from_file();
    Exam *e_head = load_exams_from_file();
    Hospitalization *h_head = load_hospitalizations_from_file();
    Ward *ward_head = load_wards_from_file();
    Bed *bed_head = load_beds_from_file();
    Prescription *pr_head = load_prescriptions_from_file();
    Drug *drug_head = load_drugs_from_file();

    int is_doctor = g_session.logged_in && strcmp(g_session.role, "doctor") == 0;

    /* 预计算全部列宽 */
    int id_w, name_w, gen_w, age_w;
    calc_patient_width(p_head, &id_w, &name_w, &gen_w, &age_w);

    int reg_w, rp_w, rd_w, rdept_w, rwhen_w, rst_w;
    calc_registration_width(reg_head, p_head, d_head, &reg_w, &rp_w, &rd_w, &rdept_w, &rwhen_w, &rst_w);

    int vis_w, vp_w, vd_w, vdept_w, vwhen_w, vst_w, vdiag_w;
    calc_visit_width(v_head, reg_head, p_head, d_head, &vis_w, &vp_w, &vd_w, &vdept_w, &vwhen_w, &vst_w, &vdiag_w);

    int exam_w, ev_w, ep_w, ed_w, edept_w, item_w, result_w;
    calc_exam_width(e_head, v_head, reg_head, p_head, d_head, &exam_w, &ev_w, &ep_w, &ed_w, &edept_w, &item_w,
                    &result_w);

    int hosp_w, hv_w, hp_w, ward_w, bed_w, in_w, out_w, hst_w;
    calc_hospitalization_width(h_head, v_head, reg_head, p_head, ward_head, bed_head, &hosp_w, &hv_w, &hp_w, &ward_w,
                               &bed_w, &in_w, &out_w, &hst_w);

    int pr_w, prv_w, prd_w, prp_w, drug_w, dose_w, freq_w;
    calc_prescription_width(pr_head, p_head, d_head, drug_head, &pr_w, &prv_w, &prd_w, &prp_w, &drug_w, &dose_w,
                            &freq_w);

    /* 统计各分类记录条数 */
    int sec_totals[SEC_COUNT] = {0};

    if (!is_doctor)
    {
        sec_totals[0] = count_registrations(reg_head);
        sec_totals[1] = count_visits(v_head);
        sec_totals[2] = count_exams(e_head);
        sec_totals[3] = count_hospitalizations(h_head);
        sec_totals[4] = count_prescriptions(pr_head);
    }
    else
    {
        sec_totals[0] = count_registrations_for_doctor(reg_head, g_session.user_id);
        sec_totals[1] = count_visits_for_doctor(v_head, reg_head, g_session.user_id);
        sec_totals[2] = count_exams_for_doctor(e_head, v_head, reg_head, g_session.user_id);
        sec_totals[3] = count_hospitalizations_for_doctor(h_head, v_head, reg_head, g_session.user_id);
        sec_totals[4] = count_prescriptions_for_doctor(pr_head, g_session.user_id);
    }

    const char *sec_names[SEC_COUNT] = {"挂号信息", "看诊信息", "检查信息", "住院信息", "处方信息"};

    int cur_sec = 0;
    int cur_page = 1;

    {
        while (1)
        {
            clear_screen();

            int total = sec_totals[cur_sec];
            int total_pages = total > 0 ? (total + PAGE_SIZE - 1) / PAGE_SIZE : 1;
            if (cur_page > total_pages)
                cur_page = total_pages;

            printf("===== %s (第 %d/%d 页，共 %d 条) =====\n", sec_names[cur_sec], cur_page,
                   sec_totals[cur_sec] > 0 ? (sec_totals[cur_sec] + PAGE_SIZE - 1) / PAGE_SIZE : 1,
                   sec_totals[cur_sec]);

            int start = (cur_page - 1) * PAGE_SIZE;

            switch (cur_sec)
            {
            case 0:
            {
                if (total == 0)
                {
                    printf("\n暂无挂号记录！\n");
                    break;
                }
                print_registration_header(reg_w, rp_w, rd_w, rdept_w, rwhen_w, rst_w);
                Registration *r = is_doctor ? get_nth_registration_for_doctor(reg_head, g_session.user_id, start)
                                            : get_nth_registration(reg_head, start);
                for (int i = 0; i < PAGE_SIZE && r; i++)
                {
                    print_registration(r, p_head, d_head, reg_w, rp_w, rd_w, rdept_w, rwhen_w, rst_w);
                    r = is_doctor ? get_nth_registration_for_doctor(reg_head, g_session.user_id, start + i + 1)
                                  : get_nth_registration(reg_head, start + i + 1);
                }
                print_registration_line(reg_w, rp_w, rd_w, rdept_w, rwhen_w, rst_w);
                break;
            }
            case 1:
            {
                if (total == 0)
                {
                    printf("\n暂无看诊记录！\n");
                    break;
                }
                print_visit_header(vis_w, vp_w, vd_w, vdept_w, vwhen_w, vst_w, vdiag_w);
                Visit *v = is_doctor ? get_nth_visit_for_doctor(v_head, reg_head, g_session.user_id, start)
                                     : get_nth_visit(v_head, start);
                for (int i = 0; i < PAGE_SIZE && v; i++)
                {
                    print_visit(v, reg_head, p_head, d_head, vis_w, vp_w, vd_w, vdept_w, vwhen_w, vst_w, vdiag_w);
                    v = is_doctor ? get_nth_visit_for_doctor(v_head, reg_head, g_session.user_id, start + i + 1)
                                  : get_nth_visit(v_head, start + i + 1);
                }
                print_visit_line(vis_w, vp_w, vd_w, vdept_w, vwhen_w, vst_w, vdiag_w);
                break;
            }
            case 2:
            {
                if (total == 0)
                {
                    printf("\n暂无检查记录！\n");
                    break;
                }
                print_exam_header(exam_w, ev_w, ep_w, ed_w, edept_w, item_w, result_w);
                Exam *e = is_doctor ? get_nth_exam_for_doctor(e_head, v_head, reg_head, g_session.user_id, start)
                                    : get_nth_exam(e_head, start);
                for (int i = 0; i < PAGE_SIZE && e; i++)
                {
                    print_exam(e, v_head, reg_head, p_head, d_head, exam_w, ev_w, ep_w, ed_w, edept_w, item_w,
                               result_w);
                    e = is_doctor
                            ? get_nth_exam_for_doctor(e_head, v_head, reg_head, g_session.user_id, start + i + 1)
                            : get_nth_exam(e_head, start + i + 1);
                }
                print_exam_line(exam_w, ev_w, ep_w, ed_w, edept_w, item_w, result_w);
                break;
            }
            case 3:
            {
                if (total == 0)
                {
                    printf("\n暂无住院记录！\n");
                    break;
                }
                print_hospitalization_header(hosp_w, hv_w, hp_w, ward_w, bed_w, in_w, out_w, hst_w);
                Hospitalization *h =
                    is_doctor ? get_nth_hospitalization_for_doctor(h_head, v_head, reg_head, g_session.user_id, start)
                              : get_nth_hospitalization(h_head, start);
                for (int i = 0; i < PAGE_SIZE && h; i++)
                {
                    print_hospitalization(h, v_head, reg_head, p_head, ward_head, bed_head, hosp_w, hv_w, hp_w, ward_w,
                                          bed_w, in_w, out_w, hst_w);
                    h = is_doctor ? get_nth_hospitalization_for_doctor(h_head, v_head, reg_head, g_session.user_id,
                                                                       start + i + 1)
                                  : get_nth_hospitalization(h_head, start + i + 1);
                }
                print_hospitalization_line(hosp_w, hv_w, hp_w, ward_w, bed_w, in_w, out_w, hst_w);
                break;
            }
            case 4:
            {
                if (total == 0)
                {
                    printf("\n暂无处方记录！\n");
                    break;
                }
                print_prescription_header(pr_w, prv_w, prd_w, prp_w, drug_w, dose_w, freq_w);
                Prescription *pr = is_doctor ? get_nth_prescription_for_doctor(pr_head, g_session.user_id, start)
                                             : get_nth_prescription(pr_head, start);
                for (int i = 0; i < PAGE_SIZE && pr; i++)
                {
                    print_prescription(pr, p_head, d_head, drug_head, pr_w, prv_w, prd_w, prp_w, drug_w, dose_w,
                                       freq_w);
                    pr = is_doctor ? get_nth_prescription_for_doctor(pr_head, g_session.user_id, start + i + 1)
                                   : get_nth_prescription(pr_head, start + i + 1);
                }
                print_prescription_line(pr_w, prv_w, prd_w, prp_w, drug_w, dose_w, freq_w);
                break;
            }
            }

            printf("\n当前类: ");
            for (int i = 0; i < SEC_COUNT; i++)
                printf(i == cur_sec ? "[%s] " : " %s  ", sec_names[i]);

            printf("\n[n]下一页  [p]上一页  [nn]下一类  [pp]上一类  [q]退出\n> ");

            char buf[MAX_INPUT_LEN];
            safe_input(buf, sizeof(buf));
            if (strcmp(buf, "n") == 0 || strcmp(buf, "N") == 0)
            {
                if (cur_page < total_pages)
                    cur_page++;
            }
            else if (strcmp(buf, "p") == 0 || strcmp(buf, "P") == 0)
            {
                if (cur_page > 1)
                    cur_page--;
            }
            else if (strcmp(buf, "nn") == 0 || strcmp(buf, "NN") == 0)
            {
                if (cur_sec < SEC_COUNT - 1)
                {
                    cur_sec++;
                    cur_page = 1;
                }
            }
            else if (strcmp(buf, "pp") == 0 || strcmp(buf, "PP") == 0)
            {
                if (cur_sec > 0)
                {
                    cur_sec--;
                    cur_page = 1;
                }
            }
            else if (strcmp(buf, "q") == 0 || strcmp(buf, "Q") == 0)
            {
                goto cleanup;
            }
            else
            {
                printf("输入无效，请重新输入！\n");
                wait_enter();
            }
        }
    }

cleanup:
    free_patients(p_head);
    free_doctors(d_head);
    free_registrations(reg_head);
    free_visits(v_head);
    free_exams(e_head);
    free_hospitalizations(h_head);
    free_wards(ward_head);
    free_beds(bed_head);
    free_prescriptions(pr_head);
    free_drugs(drug_head);

#undef SEC_COUNT
}

/*
 * 分类查询
 */

/* 挂号记录查询 */
void query_registrations(void)
{
    Patient *p_head = load_patients_from_file();
    Doctor *d_head = load_doctors_from_file();
    Registration *reg_head = load_registrations_from_file();

    int is_doctor = g_session.logged_in && strcmp(g_session.role, "doctor") == 0;
    int is_admin = g_session.logged_in && strcmp(g_session.role, "admin") == 0;

    /* 非 admin/doctor 不应进入本函数 */
    if (!is_admin && !is_doctor)
    {
        printf("您没有权限访问本功能！\n");
        wait_enter();
        goto cleanup;
    }

    char buf[MAX_INPUT_LEN];
    int select;

    while (1)
    {
        clear_screen();
        printf("===== 查询%s挂号记录 =====\n", is_doctor ? "名下" : "");
        printf("1. 按挂号ID精确查询\n");
        printf("2. 按患者姓名模糊查询\n");
        printf("3. 按医生姓名模糊查询\n");
        printf("4. 按状态查询\n");
        printf("0. 返回\n");

        printf("请输入您的选择: ");
        safe_input(buf, sizeof(buf));

        if (strcmp(buf, "0") == 0)
            goto cleanup;

        if (!validate_choice(buf, 4))
        {
            printf("输入有误，请重新选择！\n");
            wait_enter();
            clear_screen();
            continue;
        }

        select = atoi(buf);

        Registration **results = NULL;
        int hit = 0;

        switch (select)
        {
        case 1:
        {
            char key[MAX_ID_LEN];
            printf("请输入挂号ID(输入0返回): ");
            safe_input(key, sizeof(key));
            if (strcmp(key, "0") == 0)
                continue;
            if (key[0] == '\0')
            {
                printf("挂号ID不能为空！\n");
                wait_enter();
                continue;
            }
            Registration *r = find_registration_by_r_id(reg_head, key);
            if (r && (is_admin || (is_doctor && strcmp(r->d_id, g_session.user_id) == 0)))
            {
                results = (Registration **)malloc(sizeof(Registration *));
                if (!results)
                {
                    printf("内存分配失败！\n");
                    wait_enter();
                    continue;
                }
                results[hit++] = r;
            }
            break;
        }
        case 2:
        {
            char key[MAX_NAME_LEN];
            printf("请输入患者姓名(输入0返回): ");
            safe_input(key, sizeof(key));
            if (strcmp(key, "0") == 0)
                continue;
            if (key[0] == '\0')
            {
                printf("患者姓名不能为空！\n");
                wait_enter();
                continue;
            }
            int total_regs = count_registrations(reg_head);
            if (total_regs == 0)
                break;
            results = (Registration **)malloc(sizeof(Registration *) * total_regs);
            if (!results)
            {
                printf("内存分配失败！\n");
                wait_enter();
                continue;
            }
            for (Registration *r = reg_head; r; r = r->next)
            {
                if (is_admin || (is_doctor && strcmp(r->d_id, g_session.user_id) == 0))
                {
                    const char *p_name = get_patient_name_by_p_id(p_head, r->p_id);
                    if (contains_substr(p_name, key))
                        results[hit++] = r;
                }
            }
            break;
        }
        case 3:
        {
            char key[MAX_NAME_LEN];
            printf("请输入医生姓名(输入0返回): ");
            safe_input(key, sizeof(key));
            if (strcmp(key, "0") == 0)
                continue;
            if (key[0] == '\0')
            {
                printf("医生姓名不能为空！\n");
                wait_enter();
                continue;
            }
            int total_regs = count_registrations(reg_head);
            if (total_regs == 0)
                break;
            results = (Registration **)malloc(sizeof(Registration *) * total_regs);
            if (!results)
            {
                printf("内存分配失败！\n");
                wait_enter();
                continue;
            }
            for (Registration *r = reg_head; r; r = r->next)
            {
                if (is_admin || (is_doctor && strcmp(r->d_id, g_session.user_id) == 0))
                {
                    Doctor *doc = find_doctor_by_d_id(d_head, r->d_id);
                    const char *d_name = doc ? doc->name : r->d_id;
                    if (contains_substr(d_name, key))
                        results[hit++] = r;
                }
            }
            break;
        }
        case 4:
        {
            char sbuf[MAX_INPUT_LEN];
            int sselect;

            printf("状态选项: ");
            for (int i = 0; i < REG_STATUS_COUNT; i++)
                printf("[%d]%s  ", i, REG_STATUS_TEXT[i]);

            printf("\n请输入您的选择(输入q返回): ");
            safe_input(sbuf, sizeof(sbuf));

            if (strcmp(sbuf, "q") == 0 || strcmp(sbuf, "Q") == 0)
                continue;

            if (!validate_choice(sbuf, REG_STATUS_COUNT - 1))
            {
                printf("输入有误，请重新选择！\n");
                wait_enter();
                continue;
            }

            sselect = atoi(sbuf);

            int total_regs = count_registrations(reg_head);
            if (total_regs == 0)
                break;
            results = (Registration **)malloc(sizeof(Registration *) * total_regs);
            if (!results)
            {
                printf("内存分配失败！\n");
                wait_enter();
                continue;
            }
            for (Registration *r = reg_head; r; r = r->next)
            {
                if (is_admin || (is_doctor && strcmp(r->d_id, g_session.user_id) == 0))
                {
                    if (r->status == sselect)
                        results[hit++] = r;
                }
            }
            break;
        }
        default:
            printf("未知错误: %d\n", select);
            wait_enter();
            continue;
        }

        if (hit == 0)
        {
            printf("未找到匹配记录！\n");
            wait_enter();
            free(results);
            continue;
        }

        /* 分页浏览结果 */
        int reg_w, p_w, d_w, dept_w, when_w, st_w;
        calc_registration_width(reg_head, p_head, d_head, &reg_w, &p_w, &d_w, &dept_w, &when_w, &st_w);

        int cur_page = 1;
        int total_pages = (hit + PAGE_SIZE - 1) / PAGE_SIZE;

        while (1)
        {
            clear_screen();
            printf("===== 查询结果 (共 %d 条, 第 %d/%d 页) =====\n", hit, cur_page, total_pages);
            print_registration_header(reg_w, p_w, d_w, dept_w, when_w, st_w);

            int start = (cur_page - 1) * PAGE_SIZE;
            for (int i = 0; i < PAGE_SIZE && start + i < hit; i++)
                print_registration(results[start + i], p_head, d_head, reg_w, p_w, d_w, dept_w, when_w, st_w);
            print_registration_line(reg_w, p_w, d_w, dept_w, when_w, st_w);

            printf("\n[n]下一页  [p]上一页  [q]退出\n> ");
            char op[MAX_INPUT_LEN];
            safe_input(op, sizeof(op));
            if (strcmp(op, "n") == 0 || strcmp(op, "N") == 0)
            {
                if (cur_page < total_pages)
                    cur_page++;
            }
            else if (strcmp(op, "p") == 0 || strcmp(op, "P") == 0)
            {
                if (cur_page > 1)
                    cur_page--;
            }
            else if (strcmp(op, "q") == 0 || strcmp(op, "Q") == 0)
            {
                break;
            }
            else
            {
                printf("输入无效，请重新输入！\n");
                wait_enter();
            }
        }
        free(results);
    }

cleanup:
    free_patients(p_head);
    free_doctors(d_head);
    free_registrations(reg_head);
}

/* 看诊记录查询 */
void query_visits(void)
{
    Patient *p_head = load_patients_from_file();
    Doctor *d_head = load_doctors_from_file();
    Registration *reg_head = load_registrations_from_file();
    Visit *v_head = load_visits_from_file();

    int is_doctor = g_session.logged_in && strcmp(g_session.role, "doctor") == 0;
    int is_admin = g_session.logged_in && strcmp(g_session.role, "admin") == 0;

    /* 非 admin/doctor 不应进入本函数 */
    if (!is_admin && !is_doctor)
    {
        printf("您没有权限访问本功能！\n");
        wait_enter();
        goto cleanup;
    }

    char buf[MAX_INPUT_LEN];
    int select;

    while (1)
    {
        clear_screen();
        printf("===== 查询%s看诊记录 =====\n", is_doctor ? "名下" : "");
        printf("1. 按看诊ID精确查询\n");
        printf("2. 按患者姓名模糊查询\n");
        printf("3. 按医生姓名模糊查询\n");
        printf("4. 按状态查询\n");
        printf("0. 返回\n");

        printf("请输入您的选择: ");
        safe_input(buf, sizeof(buf));

        if (strcmp(buf, "0") == 0)
            goto cleanup;

        if (!validate_choice(buf, 4))
        {
            printf("输入有误，请重新选择！\n");
            wait_enter();
            clear_screen();
            continue;
        }

        select = atoi(buf);

        Visit **results = NULL;
        int hit = 0;

        switch (select)
        {
        case 1:
        {
            char key[MAX_ID_LEN];
            printf("请输入看诊ID(输入0返回): ");
            safe_input(key, sizeof(key));
            if (strcmp(key, "0") == 0)
                continue;
            if (key[0] == '\0')
            {
                printf("看诊ID不能为空！\n");
                wait_enter();
                continue;
            }
            Visit *v = find_visit_by_v_id(v_head, key);
            if (v)
            {
                /* 看诊归属判定: v -> reg -> d_id */
                Registration *r = find_registration_by_r_id(reg_head, v->reg_id);
                if (is_admin || (is_doctor && r && strcmp(r->d_id, g_session.user_id) == 0))
                {
                    results = (Visit **)malloc(sizeof(Visit *));
                    if (!results)
                    {
                        printf("内存分配失败！\n");
                        wait_enter();
                        continue;
                    }
                    results[hit++] = v;
                }
            }
            break;
        }
        case 2:
        {
            char key[MAX_NAME_LEN];
            printf("请输入患者姓名(输入0返回): ");
            safe_input(key, sizeof(key));
            if (strcmp(key, "0") == 0)
                continue;
            if (key[0] == '\0')
            {
                printf("患者姓名不能为空！\n");
                wait_enter();
                continue;
            }
            int total_v = count_visits(v_head);
            if (total_v == 0)
                break;
            results = (Visit **)malloc(sizeof(Visit *) * total_v);
            if (!results)
            {
                printf("内存分配失败！\n");
                wait_enter();
                continue;
            }
            for (Visit *v = v_head; v; v = v->next)
            {
                Registration *r = find_registration_by_r_id(reg_head, v->reg_id);
                if (!r)
                    continue;
                if (is_admin || (is_doctor && strcmp(r->d_id, g_session.user_id) == 0))
                {
                    const char *p_name = get_patient_name_by_p_id(p_head, r->p_id);
                    if (contains_substr(p_name, key))
                        results[hit++] = v;
                }
            }
            break;
        }
        case 3:
        {
            char key[MAX_NAME_LEN];
            printf("请输入医生姓名(输入0返回): ");
            safe_input(key, sizeof(key));
            if (strcmp(key, "0") == 0)
                continue;
            if (key[0] == '\0')
            {
                printf("医生姓名不能为空！\n");
                wait_enter();
                continue;
            }
            int total_v = count_visits(v_head);
            if (total_v == 0)
                break;
            results = (Visit **)malloc(sizeof(Visit *) * total_v);
            if (!results)
            {
                printf("内存分配失败！\n");
                wait_enter();
                continue;
            }
            for (Visit *v = v_head; v; v = v->next)
            {
                Registration *r = find_registration_by_r_id(reg_head, v->reg_id);
                if (!r)
                    continue;
                if (is_admin || (is_doctor && strcmp(r->d_id, g_session.user_id) == 0))
                {
                    Doctor *doc = find_doctor_by_d_id(d_head, r->d_id);
                    const char *d_name = doc ? doc->name : r->d_id;
                    if (contains_substr(d_name, key))
                        results[hit++] = v;
                }
            }
            break;
        }
        case 4:
        {
            char sbuf[MAX_INPUT_LEN];
            int sselect;

            printf("状态选项: ");
            for (int i = 0; i < VISIT_STATUS_COUNT; i++)
                printf("[%d]%s  ", i, VISIT_STATUS_TEXT[i]);

            printf("\n请输入您的选择(输入q返回): ");
            safe_input(sbuf, sizeof(sbuf));

            if (strcmp(sbuf, "q") == 0 || strcmp(sbuf, "Q") == 0)
                continue;

            if (!validate_choice(sbuf, VISIT_STATUS_COUNT - 1))
            {
                printf("输入有误，请重新选择！\n");
                wait_enter();
                continue;
            }

            sselect = atoi(sbuf);

            int total_v = count_visits(v_head);
            if (total_v == 0)
                break;
            results = (Visit **)malloc(sizeof(Visit *) * total_v);
            if (!results)
            {
                printf("内存分配失败！\n");
                wait_enter();
                continue;
            }
            for (Visit *v = v_head; v; v = v->next)
            {
                Registration *r = find_registration_by_r_id(reg_head, v->reg_id);
                if (!r)
                    continue;
                if (is_admin || (is_doctor && strcmp(r->d_id, g_session.user_id) == 0))
                {
                    if (v->status == sselect)
                        results[hit++] = v;
                }
            }
            break;
        }
        default:
            printf("未知错误: %d\n", select);
            wait_enter();
            continue;
        }

        if (hit == 0)
        {
            printf("未找到匹配记录！\n");
            wait_enter();
            free(results);
            continue;
        }

        /* 分页浏览结果 */
        int vis_w, vp_w, vd_w, vdept_w, vwhen_w, vst_w, vdiag_w;
        calc_visit_width(v_head, reg_head, p_head, d_head, &vis_w, &vp_w, &vd_w, &vdept_w, &vwhen_w, &vst_w, &vdiag_w);

        int cur_page = 1;
        int total_pages = (hit + PAGE_SIZE - 1) / PAGE_SIZE;

        while (1)
        {
            clear_screen();
            printf("===== 查询结果 (共 %d 条, 第 %d/%d 页) =====\n", hit, cur_page, total_pages);
            print_visit_header(vis_w, vp_w, vd_w, vdept_w, vwhen_w, vst_w, vdiag_w);

            int start = (cur_page - 1) * PAGE_SIZE;
            for (int i = 0; i < PAGE_SIZE && start + i < hit; i++)
                print_visit(results[start + i], reg_head, p_head, d_head, vis_w, vp_w, vd_w, vdept_w, vwhen_w, vst_w,
                            vdiag_w);
            print_visit_line(vis_w, vp_w, vd_w, vdept_w, vwhen_w, vst_w, vdiag_w);

            printf("\n[n]下一页  [p]上一页  [q]退出\n> ");
            char op[MAX_INPUT_LEN];
            safe_input(op, sizeof(op));
            if (strcmp(op, "n") == 0 || strcmp(op, "N") == 0)
            {
                if (cur_page < total_pages)
                    cur_page++;
            }
            else if (strcmp(op, "p") == 0 || strcmp(op, "P") == 0)
            {
                if (cur_page > 1)
                    cur_page--;
            }
            else if (strcmp(op, "q") == 0 || strcmp(op, "Q") == 0)
            {
                break;
            }
            else
            {
                printf("输入无效，请重新输入！\n");
                wait_enter();
            }
        }
        free(results);
    }

cleanup:
    free_patients(p_head);
    free_doctors(d_head);
    free_registrations(reg_head);
    free_visits(v_head);
}

/* 检查记录查询 */
void query_exams(void)
{
    Patient *p_head = load_patients_from_file();
    Doctor *d_head = load_doctors_from_file();
    Registration *reg_head = load_registrations_from_file();
    Visit *v_head = load_visits_from_file();
    Exam *e_head = load_exams_from_file();

    int is_doctor = g_session.logged_in && strcmp(g_session.role, "doctor") == 0;
    int is_admin = g_session.logged_in && strcmp(g_session.role, "admin") == 0;

    /* 非 admin/doctor 不应进入本函数 */
    if (!is_admin && !is_doctor)
    {
        printf("您没有权限访问本功能！\n");
        wait_enter();
        goto cleanup;
    }

    char buf[MAX_INPUT_LEN];
    int select;

    while (1)
    {
        clear_screen();
        printf("===== 查询%s检查记录 =====\n", is_doctor ? "名下" : "");
        printf("1. 按检查ID精确查询\n");
        printf("2. 按患者姓名模糊查询\n");
        printf("3. 按医生姓名模糊查询\n");
        printf("4. 按检查项目模糊查询\n");
        printf("0. 返回\n");

        printf("请输入您的选择: ");
        safe_input(buf, sizeof(buf));

        if (strcmp(buf, "0") == 0)
            goto cleanup;

        if (!validate_choice(buf, 4))
        {
            printf("输入有误，请重新选择！\n");
            wait_enter();
            clear_screen();
            continue;
        }

        select = atoi(buf);

        Exam **results = NULL;
        int hit = 0;

        switch (select)
        {
        case 1:
        {
            char key[MAX_ID_LEN];
            printf("请输入检查ID(输入0返回): ");
            safe_input(key, sizeof(key));
            if (strcmp(key, "0") == 0)
                continue;
            if (key[0] == '\0')
            {
                printf("检查ID不能为空！\n");
                wait_enter();
                continue;
            }
            Exam *e = find_exam_by_e_id(e_head, key);
            if (e)
            {
                /* 检查归属判定: e -> visit -> reg -> d_id */
                Visit *v = find_visit_by_v_id(v_head, e->visit_id);
                Registration *r = v ? find_registration_by_r_id(reg_head, v->reg_id) : NULL;
                if (is_admin || (is_doctor && r && strcmp(r->d_id, g_session.user_id) == 0))
                {
                    results = (Exam **)malloc(sizeof(Exam *));
                    if (!results)
                    {
                        printf("内存分配失败！\n");
                        wait_enter();
                        continue;
                    }
                    results[hit++] = e;
                }
            }
            break;
        }
        case 2:
        {
            char key[MAX_NAME_LEN];
            printf("请输入患者姓名(输入0返回): ");
            safe_input(key, sizeof(key));
            if (strcmp(key, "0") == 0)
                continue;
            if (key[0] == '\0')
            {
                printf("患者姓名不能为空！\n");
                wait_enter();
                continue;
            }
            int total_e = count_exams(e_head);
            if (total_e == 0)
                break;
            results = (Exam **)malloc(sizeof(Exam *) * total_e);
            if (!results)
            {
                printf("内存分配失败！\n");
                wait_enter();
                continue;
            }
            for (Exam *e = e_head; e; e = e->next)
            {
                Visit *v = find_visit_by_v_id(v_head, e->visit_id);
                if (!v)
                    continue;
                Registration *r = find_registration_by_r_id(reg_head, v->reg_id);
                if (!r)
                    continue;
                if (is_admin || (is_doctor && strcmp(r->d_id, g_session.user_id) == 0))
                {
                    const char *p_name = get_patient_name_by_p_id(p_head, r->p_id);
                    if (contains_substr(p_name, key))
                        results[hit++] = e;
                }
            }
            break;
        }
        case 3:
        {
            char key[MAX_NAME_LEN];
            printf("请输入医生姓名(输入0返回): ");
            safe_input(key, sizeof(key));
            if (strcmp(key, "0") == 0)
                continue;
            if (key[0] == '\0')
            {
                printf("医生姓名不能为空！\n");
                wait_enter();
                continue;
            }
            int total_e = count_exams(e_head);
            if (total_e == 0)
                break;
            results = (Exam **)malloc(sizeof(Exam *) * total_e);
            if (!results)
            {
                printf("内存分配失败！\n");
                wait_enter();
                continue;
            }
            for (Exam *e = e_head; e; e = e->next)
            {
                Visit *v = find_visit_by_v_id(v_head, e->visit_id);
                if (!v)
                    continue;
                Registration *r = find_registration_by_r_id(reg_head, v->reg_id);
                if (!r)
                    continue;
                if (is_admin || (is_doctor && strcmp(r->d_id, g_session.user_id) == 0))
                {
                    Doctor *doc = find_doctor_by_d_id(d_head, r->d_id);
                    const char *d_name = doc ? doc->name : r->d_id;
                    if (contains_substr(d_name, key))
                        results[hit++] = e;
                }
            }
            break;
        }
        case 4:
        {
            /* 检查记录无 status, 第 4 项改为按检查项目模糊 */
            char key[MAX_INPUT_LEN];
            printf("请输入检查项目关键字(输入0返回): ");
            safe_input(key, sizeof(key));
            if (strcmp(key, "0") == 0)
                continue;
            if (key[0] == '\0')
            {
                printf("检查项目不能为空！\n");
                wait_enter();
                continue;
            }
            int total_e = count_exams(e_head);
            if (total_e == 0)
                break;
            results = (Exam **)malloc(sizeof(Exam *) * total_e);
            if (!results)
            {
                printf("内存分配失败！\n");
                wait_enter();
                continue;
            }
            for (Exam *e = e_head; e; e = e->next)
            {
                Visit *v = find_visit_by_v_id(v_head, e->visit_id);
                if (!v)
                    continue;
                Registration *r = find_registration_by_r_id(reg_head, v->reg_id);
                if (!r)
                    continue;
                if (is_admin || (is_doctor && strcmp(r->d_id, g_session.user_id) == 0))
                {
                    if (contains_substr(e->item, key))
                        results[hit++] = e;
                }
            }
            break;
        }
        default:
            printf("未知错误: %d\n", select);
            wait_enter();
            continue;
        }

        if (hit == 0)
        {
            printf("未找到匹配记录！\n");
            wait_enter();
            free(results);
            continue;
        }

        /* 分页浏览结果 */
        int exam_w, ev_w, ep_w, ed_w, edept_w, item_w, result_w;
        calc_exam_width(e_head, v_head, reg_head, p_head, d_head, &exam_w, &ev_w, &ep_w, &ed_w, &edept_w, &item_w,
                        &result_w);

        int cur_page = 1;
        int total_pages = (hit + PAGE_SIZE - 1) / PAGE_SIZE;

        while (1)
        {
            clear_screen();
            printf("===== 查询结果 (共 %d 条, 第 %d/%d 页) =====\n", hit, cur_page, total_pages);
            print_exam_header(exam_w, ev_w, ep_w, ed_w, edept_w, item_w, result_w);

            int start = (cur_page - 1) * PAGE_SIZE;
            for (int i = 0; i < PAGE_SIZE && start + i < hit; i++)
                print_exam(results[start + i], v_head, reg_head, p_head, d_head, exam_w, ev_w, ep_w, ed_w, edept_w,
                           item_w, result_w);
            print_exam_line(exam_w, ev_w, ep_w, ed_w, edept_w, item_w, result_w);

            printf("\n[n]下一页  [p]上一页  [q]退出\n> ");
            char op[MAX_INPUT_LEN];
            safe_input(op, sizeof(op));
            if (strcmp(op, "n") == 0 || strcmp(op, "N") == 0)
            {
                if (cur_page < total_pages)
                    cur_page++;
            }
            else if (strcmp(op, "p") == 0 || strcmp(op, "P") == 0)
            {
                if (cur_page > 1)
                    cur_page--;
            }
            else if (strcmp(op, "q") == 0 || strcmp(op, "Q") == 0)
            {
                break;
            }
            else
            {
                printf("输入无效，请重新输入！\n");
                wait_enter();
            }
        }
        free(results);
    }

cleanup:
    free_patients(p_head);
    free_doctors(d_head);
    free_registrations(reg_head);
    free_visits(v_head);
    free_exams(e_head);
}

/* 住院记录查询 */
void query_hospitalizations(void)
{
    Patient *p_head = load_patients_from_file();
    Doctor *d_head = load_doctors_from_file();
    Registration *reg_head = load_registrations_from_file();
    Visit *v_head = load_visits_from_file();
    Hospitalization *h_head = load_hospitalizations_from_file();
    Ward *ward_head = load_wards_from_file();
    Bed *bed_head = load_beds_from_file();

    int is_doctor = g_session.logged_in && strcmp(g_session.role, "doctor") == 0;
    int is_admin = g_session.logged_in && strcmp(g_session.role, "admin") == 0;

    /* 非 admin/doctor 不应进入本函数 */
    if (!is_admin && !is_doctor)
    {
        printf("您没有权限访问本功能！\n");
        wait_enter();
        goto cleanup;
    }

    char buf[MAX_INPUT_LEN];
    int select;

    while (1)
    {
        clear_screen();
        printf("===== 查询%s住院记录 =====\n", is_doctor ? "名下" : "");
        printf("1. 按住院ID精确查询\n");
        printf("2. 按患者姓名模糊查询\n");
        printf("3. 按医生姓名模糊查询\n");
        printf("4. 按状态查询\n");
        printf("0. 返回\n");

        printf("请输入您的选择: ");
        safe_input(buf, sizeof(buf));

        if (strcmp(buf, "0") == 0)
            goto cleanup;

        if (!validate_choice(buf, 4))
        {
            printf("输入有误，请重新选择！\n");
            wait_enter();
            clear_screen();
            continue;
        }

        select = atoi(buf);

        Hospitalization **results = NULL;
        int hit = 0;

        switch (select)
        {
        case 1:
        {
            char key[MAX_ID_LEN];
            printf("请输入住院ID(输入0返回): ");
            safe_input(key, sizeof(key));
            if (strcmp(key, "0") == 0)
                continue;
            if (key[0] == '\0')
            {
                printf("住院ID不能为空！\n");
                wait_enter();
                continue;
            }
            Hospitalization *h = find_hospitalization_by_h_id(h_head, key);
            if (h)
            {
                /* 住院归属判定: h -> visit -> reg -> d_id */
                Visit *v = find_visit_by_v_id(v_head, h->visit_id);
                Registration *r = v ? find_registration_by_r_id(reg_head, v->reg_id) : NULL;
                if (is_admin || (is_doctor && r && strcmp(r->d_id, g_session.user_id) == 0))
                {
                    results = (Hospitalization **)malloc(sizeof(Hospitalization *));
                    if (!results)
                    {
                        printf("内存分配失败！\n");
                        wait_enter();
                        continue;
                    }
                    results[hit++] = h;
                }
            }
            break;
        }
        case 2:
        {
            char key[MAX_NAME_LEN];
            printf("请输入患者姓名(输入0返回): ");
            safe_input(key, sizeof(key));
            if (strcmp(key, "0") == 0)
                continue;
            if (key[0] == '\0')
            {
                printf("患者姓名不能为空！\n");
                wait_enter();
                continue;
            }
            int total_h = count_hospitalizations(h_head);
            if (total_h == 0)
                break;
            results = (Hospitalization **)malloc(sizeof(Hospitalization *) * total_h);
            if (!results)
            {
                printf("内存分配失败！\n");
                wait_enter();
                continue;
            }
            for (Hospitalization *h = h_head; h; h = h->next)
            {
                Visit *v = find_visit_by_v_id(v_head, h->visit_id);
                Registration *r = v ? find_registration_by_r_id(reg_head, v->reg_id) : NULL;
                if (is_admin || (is_doctor && r && strcmp(r->d_id, g_session.user_id) == 0))
                {
                    /* 住院表本身有 p_id, 直接用 */
                    const char *p_name = get_patient_name_by_p_id(p_head, h->p_id);
                    if (contains_substr(p_name, key))
                        results[hit++] = h;
                }
            }
            break;
        }
        case 3:
        {
            char key[MAX_NAME_LEN];
            printf("请输入医生姓名(输入0返回): ");
            safe_input(key, sizeof(key));
            if (strcmp(key, "0") == 0)
                continue;
            if (key[0] == '\0')
            {
                printf("医生姓名不能为空！\n");
                wait_enter();
                continue;
            }
            int total_h = count_hospitalizations(h_head);
            if (total_h == 0)
                break;
            results = (Hospitalization **)malloc(sizeof(Hospitalization *) * total_h);
            if (!results)
            {
                printf("内存分配失败！\n");
                wait_enter();
                continue;
            }
            for (Hospitalization *h = h_head; h; h = h->next)
            {
                Visit *v = find_visit_by_v_id(v_head, h->visit_id);
                if (!v)
                    continue;
                Registration *r = find_registration_by_r_id(reg_head, v->reg_id);
                if (!r)
                    continue;
                if (is_admin || (is_doctor && strcmp(r->d_id, g_session.user_id) == 0))
                {
                    Doctor *doc = find_doctor_by_d_id(d_head, r->d_id);
                    const char *d_name = doc ? doc->name : r->d_id;
                    if (contains_substr(d_name, key))
                        results[hit++] = h;
                }
            }
            break;
        }
        case 4:
        {
            char sbuf[MAX_INPUT_LEN];
            int sselect;

            printf("状态选项: [0]住院中  [1]已出院");

            printf("\n请输入您的选择(输入q返回): ");
            safe_input(sbuf, sizeof(sbuf));

            if (strcmp(sbuf, "q") == 0 || strcmp(sbuf, "Q") == 0)
                continue;

            if (!validate_choice(sbuf, HOSP_STATUS_COUNT - 1))
            {
                printf("输入有误，请重新选择！\n");
                wait_enter();
                continue;
            }

            sselect = atoi(sbuf);

            int total_h = count_hospitalizations(h_head);
            if (total_h == 0)
                break;
            results = (Hospitalization **)malloc(sizeof(Hospitalization *) * total_h);
            if (!results)
            {
                printf("内存分配失败！\n");
                wait_enter();
                continue;
            }
            for (Hospitalization *h = h_head; h; h = h->next)
            {
                Visit *v = find_visit_by_v_id(v_head, h->visit_id);
                Registration *r = v ? find_registration_by_r_id(reg_head, v->reg_id) : NULL;
                if (is_admin || (is_doctor && r && strcmp(r->d_id, g_session.user_id) == 0))
                {
                    if (h->status == sselect)
                        results[hit++] = h;
                }
            }
            break;
        }
        default:
            printf("未知错误: %d\n", select);
            wait_enter();
            continue;
        }

        if (hit == 0)
        {
            printf("未找到匹配记录！\n");
            wait_enter();
            free(results);
            continue;
        }

        /* 分页浏览结果 */
        int hosp_w, hv_w, hp_w, ward_w, bed_w, in_w, out_w, hst_w;
        calc_hospitalization_width(h_head, v_head, reg_head, p_head, ward_head, bed_head, &hosp_w, &hv_w, &hp_w,
                                   &ward_w, &bed_w, &in_w, &out_w, &hst_w);

        int cur_page = 1;
        int total_pages = (hit + PAGE_SIZE - 1) / PAGE_SIZE;

        while (1)
        {
            clear_screen();
            printf("===== 查询结果 (共 %d 条, 第 %d/%d 页) =====\n", hit, cur_page, total_pages);
            print_hospitalization_header(hosp_w, hv_w, hp_w, ward_w, bed_w, in_w, out_w, hst_w);

            int start = (cur_page - 1) * PAGE_SIZE;
            for (int i = 0; i < PAGE_SIZE && start + i < hit; i++)
                print_hospitalization(results[start + i], v_head, reg_head, p_head, ward_head, bed_head, hosp_w, hv_w,
                                      hp_w, ward_w, bed_w, in_w, out_w, hst_w);
            print_hospitalization_line(hosp_w, hv_w, hp_w, ward_w, bed_w, in_w, out_w, hst_w);

            printf("\n[n]下一页  [p]上一页  [q]退出\n> ");
            char op[MAX_INPUT_LEN];
            safe_input(op, sizeof(op));
            if (strcmp(op, "n") == 0 || strcmp(op, "N") == 0)
            {
                if (cur_page < total_pages)
                    cur_page++;
            }
            else if (strcmp(op, "p") == 0 || strcmp(op, "P") == 0)
            {
                if (cur_page > 1)
                    cur_page--;
            }
            else if (strcmp(op, "q") == 0 || strcmp(op, "Q") == 0)
            {
                break;
            }
            else
            {
                printf("输入无效，请重新输入！\n");
                wait_enter();
            }
        }
        free(results);
    }

cleanup:
    free_patients(p_head);
    free_doctors(d_head);
    free_registrations(reg_head);
    free_visits(v_head);
    free_hospitalizations(h_head);
    free_wards(ward_head);
    free_beds(bed_head);
}

/* 处方记录查询 */
void query_prescriptions(void)
{
    Patient *p_head = load_patients_from_file();
    Doctor *d_head = load_doctors_from_file();
    Prescription *pr_head = load_prescriptions_from_file();
    Drug *drug_head = load_drugs_from_file();

    int is_doctor = g_session.logged_in && strcmp(g_session.role, "doctor") == 0;
    int is_admin = g_session.logged_in && strcmp(g_session.role, "admin") == 0;

    /* 非 admin/doctor 不应进入本函数 */
    if (!is_admin && !is_doctor)
    {
        printf("您没有权限访问本功能！\n");
        wait_enter();
        goto cleanup;
    }

    char buf[MAX_INPUT_LEN];
    int select;

    while (1)
    {
        clear_screen();
        printf("===== 查询%s处方记录 =====\n", is_doctor ? "名下" : "");
        printf("1. 按处方ID精确查询\n");
        printf("2. 按患者姓名模糊查询\n");
        printf("3. 按医生姓名模糊查询\n");
        printf("4. 按药品名称模糊查询\n");
        printf("0. 返回\n");

        printf("请输入您的选择: ");
        safe_input(buf, sizeof(buf));

        if (strcmp(buf, "0") == 0)
            goto cleanup;

        if (!validate_choice(buf, 4))
        {
            printf("输入有误，请重新选择！\n");
            wait_enter();
            clear_screen();
            continue;
        }

        select = atoi(buf);

        Prescription **results = NULL;
        int hit = 0;

        switch (select)
        {
        case 1:
        {
            char key[MAX_ID_LEN];
            printf("请输入处方ID(输入0返回): ");
            safe_input(key, sizeof(key));
            if (strcmp(key, "0") == 0)
                continue;
            if (key[0] == '\0')
            {
                printf("处方ID不能为空！\n");
                wait_enter();
                continue;
            }
            Prescription *pr = find_prescription_by_pr_id(pr_head, key);
            /* 处方表自带 d_id, 直接判定 */
            if (pr && (is_admin || (is_doctor && strcmp(pr->d_id, g_session.user_id) == 0)))
            {
                results = (Prescription **)malloc(sizeof(Prescription *));
                if (!results)
                {
                    printf("内存分配失败！\n");
                    wait_enter();
                    continue;
                }
                results[hit++] = pr;
            }
            break;
        }
        case 2:
        {
            char key[MAX_NAME_LEN];
            printf("请输入患者姓名(输入0返回): ");
            safe_input(key, sizeof(key));
            if (strcmp(key, "0") == 0)
                continue;
            if (key[0] == '\0')
            {
                printf("患者姓名不能为空！\n");
                wait_enter();
                continue;
            }
            int total_pr = count_prescriptions(pr_head);
            if (total_pr == 0)
                break;
            results = (Prescription **)malloc(sizeof(Prescription *) * total_pr);
            if (!results)
            {
                printf("内存分配失败！\n");
                wait_enter();
                continue;
            }
            for (Prescription *pr = pr_head; pr; pr = pr->next)
            {
                if (is_admin || (is_doctor && strcmp(pr->d_id, g_session.user_id) == 0))
                {
                    const char *p_name = get_patient_name_by_p_id(p_head, pr->p_id);
                    if (contains_substr(p_name, key))
                        results[hit++] = pr;
                }
            }
            break;
        }
        case 3:
        {
            char key[MAX_NAME_LEN];
            printf("请输入医生姓名(输入0返回): ");
            safe_input(key, sizeof(key));
            if (strcmp(key, "0") == 0)
                continue;
            if (key[0] == '\0')
            {
                printf("医生姓名不能为空！\n");
                wait_enter();
                continue;
            }
            int total_pr = count_prescriptions(pr_head);
            if (total_pr == 0)
                break;
            results = (Prescription **)malloc(sizeof(Prescription *) * total_pr);
            if (!results)
            {
                printf("内存分配失败！\n");
                wait_enter();
                continue;
            }
            for (Prescription *pr = pr_head; pr; pr = pr->next)
            {
                if (is_admin || (is_doctor && strcmp(pr->d_id, g_session.user_id) == 0))
                {
                    Doctor *doc = find_doctor_by_d_id(d_head, pr->d_id);
                    const char *d_name = doc ? doc->name : pr->d_id;
                    if (contains_substr(d_name, key))
                        results[hit++] = pr;
                }
            }
            break;
        }
        case 4:
        {
            /* 处方无 status, 第 4 项改为按药品名称模糊 */
            char key[MAX_NAME_LEN];
            printf("请输入药品名称(输入0返回): ");
            safe_input(key, sizeof(key));
            if (strcmp(key, "0") == 0)
                continue;
            if (key[0] == '\0')
            {
                printf("药品名称不能为空！\n");
                wait_enter();
                continue;
            }
            int total_pr = count_prescriptions(pr_head);
            if (total_pr == 0)
                break;
            results = (Prescription **)malloc(sizeof(Prescription *) * total_pr);
            if (!results)
            {
                printf("内存分配失败！\n");
                wait_enter();
                continue;
            }
            for (Prescription *pr = pr_head; pr; pr = pr->next)
            {
                if (is_admin || (is_doctor && strcmp(pr->d_id, g_session.user_id) == 0))
                {
                    Drug *drug = find_drug_by_id(drug_head, pr->drug_id);
                    const char *drug_name = drug ? drug->trade_name : pr->drug_id;
                    if (contains_substr(drug_name, key))
                        results[hit++] = pr;
                }
            }
            break;
        }
        default:
            printf("未知错误: %d\n", select);
            wait_enter();
            continue;
        }

        if (hit == 0)
        {
            printf("未找到匹配记录！\n");
            wait_enter();
            free(results);
            continue;
        }

        /* 分页浏览结果 */
        int pr_w, prv_w, prd_w, prp_w, drug_w, dose_w, freq_w;
        calc_prescription_width(pr_head, p_head, d_head, drug_head, &pr_w, &prv_w, &prd_w, &prp_w, &drug_w, &dose_w,
                                &freq_w);

        int cur_page = 1;
        int total_pages = (hit + PAGE_SIZE - 1) / PAGE_SIZE;

        while (1)
        {
            clear_screen();
            printf("===== 查询结果 (共 %d 条, 第 %d/%d 页) =====\n", hit, cur_page, total_pages);
            print_prescription_header(pr_w, prv_w, prd_w, prp_w, drug_w, dose_w, freq_w);

            int start = (cur_page - 1) * PAGE_SIZE;
            for (int i = 0; i < PAGE_SIZE && start + i < hit; i++)
                print_prescription(results[start + i], p_head, d_head, drug_head, pr_w, prv_w, prd_w, prp_w, drug_w,
                                   dose_w, freq_w);
            print_prescription_line(pr_w, prv_w, prd_w, prp_w, drug_w, dose_w, freq_w);

            printf("\n[n]下一页  [p]上一页  [q]退出\n> ");
            char op[MAX_INPUT_LEN];
            safe_input(op, sizeof(op));
            if (strcmp(op, "n") == 0 || strcmp(op, "N") == 0)
            {
                if (cur_page < total_pages)
                    cur_page++;
            }
            else if (strcmp(op, "p") == 0 || strcmp(op, "P") == 0)
            {
                if (cur_page > 1)
                    cur_page--;
            }
            else if (strcmp(op, "q") == 0 || strcmp(op, "Q") == 0)
            {
                break;
            }
            else
            {
                printf("输入无效，请重新输入！\n");
                wait_enter();
            }
        }
        free(results);
    }

cleanup:
    free_patients(p_head);
    free_doctors(d_head);
    free_prescriptions(pr_head);
    free_drugs(drug_head);
}

/*
 * 分类删除
 */

/* 挂号记录删除 */
void delete_registration(void)
{
    char reg_id[MAX_ID_LEN];
    printf("请输入要删除的挂号ID(输入0返回): ");
    safe_input(reg_id, sizeof(reg_id));
    if (strcmp(reg_id, "0") == 0)
    {
        clear_screen();
        return;
    }

    if (reg_id[0] == '\0')
    {
        printf("输入错误！挂号ID不能为空。\n");
        wait_enter();
        clear_screen();
        return;
    }

    /* 加载所有相关链表 */
    Patient *p_head = load_patients_from_file();
    Doctor *d_head = load_doctors_from_file();
    Registration *reg_head = load_registrations_from_file();
    Visit *v_head = load_visits_from_file();
    Exam *e_head = load_exams_from_file();
    Hospitalization *h_head = load_hospitalizations_from_file();
    Ward *ward_head = load_wards_from_file();
    Bed *bed_head = load_beds_from_file();
    Prescription *pr_head = load_prescriptions_from_file();

    Registration *target = find_registration_by_r_id(reg_head, reg_id);
    if (!target)
    {
        printf("未找到指定ID的挂号！\n");
        free_patients(p_head);
        free_doctors(d_head);
        free_registrations(reg_head);
        free_visits(v_head);
        free_exams(e_head);
        free_hospitalizations(h_head);
        free_wards(ward_head);
        free_beds(bed_head);
        free_prescriptions(pr_head);
        wait_enter();
        clear_screen();
        return;
    }

    /* 显示待删除挂号 */
    int reg_w, rp_w, rd_w, rdept_w, rwhen_w, rst_w;
    calc_registration_width(reg_head, p_head, d_head, &reg_w, &rp_w, &rd_w, &rdept_w, &rwhen_w, &rst_w);
    printf("\n待删除挂号:\n");
    print_registration_header(reg_w, rp_w, rd_w, rdept_w, rwhen_w, rst_w);
    print_registration(target, p_head, d_head, reg_w, rp_w, rd_w, rdept_w, rwhen_w, rst_w);
    print_registration_line(reg_w, rp_w, rd_w, rdept_w, rwhen_w, rst_w);

    /* 探查下游级联数量 */
    int cas_v = 0, cas_e = 0, cas_h = 0, cas_pr = 0;
    for (Visit *v = v_head; v; v = v->next)
    {
        if (strcmp(v->reg_id, reg_id) != 0)
            continue;
        cas_v++;
        for (Exam *e = e_head; e; e = e->next)
            if (strcmp(e->visit_id, v->visit_id) == 0)
                cas_e++;
        for (Hospitalization *h = h_head; h; h = h->next)
            if (strcmp(h->visit_id, v->visit_id) == 0)
                cas_h++;
        for (Prescription *pr = pr_head; pr; pr = pr->next)
            if (strcmp(pr->visit_id, v->visit_id) == 0)
                cas_pr++;
    }

    if (cas_v + cas_e + cas_h + cas_pr > 0)
    {
        printf("\n⚠ 级联影响:\n");
        printf("  - 看诊记录: %d 条\n", cas_v);
        printf("  - 检查记录: %d 条\n", cas_e);
        printf("  - 住院记录: %d 条 (将自动释放对应床位)\n", cas_h);
        printf("  - 处方记录: %d 条\n", cas_pr);
        printf("以上记录将一并删除。\n");
    }
    else
    {
        printf("\n该挂号无任何关联记录。\n");
    }

    printf("确定要删除吗？(y/n): ");
    char confirm[MAX_INPUT_LEN];
    safe_input(confirm, sizeof(confirm));
    if (strcmp(confirm, "n") == 0 || strcmp(confirm, "N") == 0)
    {
        printf("已取消删除。\n");
        free_patients(p_head);
        free_doctors(d_head);
        free_registrations(reg_head);
        free_visits(v_head);
        free_exams(e_head);
        free_hospitalizations(h_head);
        free_wards(ward_head);
        free_beds(bed_head);
        free_prescriptions(pr_head);
        wait_enter();
        clear_screen();
        return;
    }
    else if (strcmp(confirm, "y") != 0 && strcmp(confirm, "Y") != 0)
    {
        printf("输入无效，已取消删除。\n");
        free_patients(p_head);
        free_doctors(d_head);
        free_registrations(reg_head);
        free_visits(v_head);
        free_exams(e_head);
        free_hospitalizations(h_head);
        free_wards(ward_head);
        free_beds(bed_head);
        free_prescriptions(pr_head);
        wait_enter();
        clear_screen();
        return;
    }

    /*
     * 执行级联删除
     * 顺序: 先收集 visit_id -> 删 exam / prescription / hospitalization (同时释放床位)
     *       -> 删 visit -> 删 registration 自身
     */

    /* 先收集该挂号下所有 visit_id, 因为后面 visit 节点会被释放 */
    char (*v_ids)[MAX_ID_LEN] = NULL;
    int v_cnt = 0;
    if (cas_v > 0)
    {
        v_ids = (char (*)[MAX_ID_LEN])malloc(sizeof(char[MAX_ID_LEN]) * cas_v);
        if (!v_ids)
        {
            printf("内存分配失败，删除中止！\n");
            free_patients(p_head);
            free_doctors(d_head);
            free_registrations(reg_head);
            free_visits(v_head);
            free_exams(e_head);
            free_hospitalizations(h_head);
            free_wards(ward_head);
            free_beds(bed_head);
            free_prescriptions(pr_head);
            wait_enter();
            clear_screen();
            return;
        }
        for (Visit *v = v_head; v; v = v->next)
        {
            if (strcmp(v->reg_id, reg_id) == 0)
            {
                strncpy(v_ids[v_cnt], v->visit_id, MAX_ID_LEN - 1);
                v_ids[v_cnt][MAX_ID_LEN - 1] = '\0';
                v_cnt++;
            }
        }
    }

    /* 删 exam */
    for (int i = 0; i < v_cnt; i++)
    {
        Exam *e = e_head;
        while (e)
        {
            Exam *next = e->next;
            if (strcmp(e->visit_id, v_ids[i]) == 0)
                exam_remove(&e_head, e);
            e = next;
        }
    }

    /* 删 prescription */
    for (int i = 0; i < v_cnt; i++)
    {
        Prescription *pr = pr_head;
        while (pr)
        {
            Prescription *next = pr->next;
            if (strcmp(pr->visit_id, v_ids[i]) == 0)
                prescription_remove(&pr_head, pr);
            pr = next;
        }
    }

    /* 删 hospitalization (同时释放床位) */
    for (int i = 0; i < v_cnt; i++)
    {
        Hospitalization *h = h_head;
        while (h)
        {
            Hospitalization *next = h->next;
            if (strcmp(h->visit_id, v_ids[i]) == 0)
            {
                if (h->status == HOSP_STATUS_ONGOING)
                {
                    Bed *b = find_bed_by_b_id(bed_head, h->bed_id);
                    Ward *w = find_ward_by_w_id(ward_head, h->ward_id);
                    if (b)
                        b->status = BED_STATUS_FREE;
                    if (w && w->occupied > 0)
                        w->occupied -= 1;
                }
                hospitalization_remove(&h_head, h);
            }
            h = next;
        }
    }

    /* 删 visit */
    for (int i = 0; i < v_cnt; i++)
    {
        Visit *v = find_visit_by_v_id(v_head, v_ids[i]);
        if (v)
            visit_remove(&v_head, v);
    }

    /* 删 registration 自身 */
    registration_remove(&reg_head, target);

    free(v_ids);

    /* 落盘: 顺序保存所有受影响的表 */
    int err = 0;
    if (save_registrations_to_file(reg_head) != 0)
        err++;
    if (save_visits_to_file(v_head) != 0)
        err++;
    if (save_exams_to_file(e_head) != 0)
        err++;
    if (save_hospitalizations_to_file(h_head) != 0)
        err++;
    if (save_beds_to_file(bed_head) != 0)
        err++;
    if (save_wards_to_file(ward_head) != 0)
        err++;
    if (save_prescriptions_to_file(pr_head) != 0)
        err++;

    if (err == 0)
        printf("删除成功！\n");
    else
        printf("删除完成，但有 %d 个文件保存失败，请检查数据文件！\n", err);

    free_patients(p_head);
    free_doctors(d_head);
    free_registrations(reg_head);
    free_visits(v_head);
    free_exams(e_head);
    free_hospitalizations(h_head);
    free_wards(ward_head);
    free_beds(bed_head);
    free_prescriptions(pr_head);
    wait_enter();
    clear_screen();
}

/* 看诊记录删除 */
void delete_visit(void)
{
    char v_id[MAX_ID_LEN];
    printf("请输入要删除的看诊ID(输入0返回): ");
    safe_input(v_id, sizeof(v_id));
    if (strcmp(v_id, "0") == 0)
    {
        clear_screen();
        return;
    }

    if (v_id[0] == '\0')
    {
        printf("输入错误！看诊ID不能为空。\n");
        wait_enter();
        clear_screen();
        return;
    }

    /* 加载所有相关链表 */
    Patient *p_head = load_patients_from_file();
    Doctor *d_head = load_doctors_from_file();
    Registration *reg_head = load_registrations_from_file();
    Visit *v_head = load_visits_from_file();
    Exam *e_head = load_exams_from_file();
    Hospitalization *h_head = load_hospitalizations_from_file();
    Ward *ward_head = load_wards_from_file();
    Bed *bed_head = load_beds_from_file();
    Prescription *pr_head = load_prescriptions_from_file();

    Visit *target = find_visit_by_v_id(v_head, v_id);
    if (!target)
    {
        printf("未找到指定ID的看诊！\n");
        free_patients(p_head);
        free_doctors(d_head);
        free_registrations(reg_head);
        free_visits(v_head);
        free_exams(e_head);
        free_hospitalizations(h_head);
        free_wards(ward_head);
        free_beds(bed_head);
        free_prescriptions(pr_head);
        wait_enter();
        clear_screen();
        return;
    }

    /* 显示待删除看诊 */
    int vis_w, vp_w, vd_w, vdept_w, vwhen_w, vst_w, vdiag_w;
    calc_visit_width(v_head, reg_head, p_head, d_head, &vis_w, &vp_w, &vd_w, &vdept_w, &vwhen_w, &vst_w, &vdiag_w);
    printf("\n待删除看诊:\n");
    print_visit_header(vis_w, vp_w, vd_w, vdept_w, vwhen_w, vst_w, vdiag_w);
    print_visit(target, reg_head, p_head, d_head, vis_w, vp_w, vd_w, vdept_w, vwhen_w, vst_w, vdiag_w);
    print_visit_line(vis_w, vp_w, vd_w, vdept_w, vwhen_w, vst_w, vdiag_w);

    /* 探查下游级联数量 */
    int cas_e = 0, cas_h = 0, cas_pr = 0;
    for (Exam *e = e_head; e; e = e->next)
        if (strcmp(e->visit_id, v_id) == 0)
            cas_e++;
    for (Hospitalization *h = h_head; h; h = h->next)
        if (strcmp(h->visit_id, v_id) == 0)
            cas_h++;
    for (Prescription *pr = pr_head; pr; pr = pr->next)
        if (strcmp(pr->visit_id, v_id) == 0)
            cas_pr++;

    if (cas_e + cas_h + cas_pr > 0)
    {
        printf("\n⚠ 级联影响:\n");
        printf("  - 检查记录: %d 条\n", cas_e);
        printf("  - 住院记录: %d 条 (将自动释放对应床位)\n", cas_h);
        printf("  - 处方记录: %d 条\n", cas_pr);
        printf("以上记录将一并删除。\n");
    }
    else
    {
        printf("\n该看诊无任何关联记录。\n");
    }

    printf("确定要删除吗？(y/n): ");
    char confirm[MAX_INPUT_LEN];
    safe_input(confirm, sizeof(confirm));
    if (strcmp(confirm, "n") == 0 || strcmp(confirm, "N") == 0)
    {
        printf("已取消删除。\n");
        free_patients(p_head);
        free_doctors(d_head);
        free_registrations(reg_head);
        free_visits(v_head);
        free_exams(e_head);
        free_hospitalizations(h_head);
        free_wards(ward_head);
        free_beds(bed_head);
        free_prescriptions(pr_head);
        wait_enter();
        clear_screen();
        return;
    }
    else if (strcmp(confirm, "y") != 0 && strcmp(confirm, "Y") != 0)
    {
        printf("输入无效，已取消删除。\n");
        free_patients(p_head);
        free_doctors(d_head);
        free_registrations(reg_head);
        free_visits(v_head);
        free_exams(e_head);
        free_hospitalizations(h_head);
        free_wards(ward_head);
        free_beds(bed_head);
        free_prescriptions(pr_head);
        wait_enter();
        clear_screen();
        return;
    }

    /*
     * 执行级联删除
     * 顺序: 删 exam / prescription / hospitalization (同时释放床位) -> 删 visit 自身
     */

    /* 删 exam */
    {
        Exam *e = e_head;
        while (e)
        {
            Exam *next = e->next;
            if (strcmp(e->visit_id, v_id) == 0)
                exam_remove(&e_head, e);
            e = next;
        }
    }

    /* 删 prescription */
    {
        Prescription *pr = pr_head;
        while (pr)
        {
            Prescription *next = pr->next;
            if (strcmp(pr->visit_id, v_id) == 0)
                prescription_remove(&pr_head, pr);
            pr = next;
        }
    }

    /* 删 hospitalization (同时释放床位) */
    {
        Hospitalization *h = h_head;
        while (h)
        {
            Hospitalization *next = h->next;
            if (strcmp(h->visit_id, v_id) == 0)
            {
                if (h->status == HOSP_STATUS_ONGOING)
                {
                    Bed *b = find_bed_by_b_id(bed_head, h->bed_id);
                    Ward *w = find_ward_by_w_id(ward_head, h->ward_id);
                    if (b)
                        b->status = BED_STATUS_FREE;
                    if (w && w->occupied > 0)
                        w->occupied -= 1;
                }
                hospitalization_remove(&h_head, h);
            }
            h = next;
        }
    }

    /* 删 visit 自身 */
    visit_remove(&v_head, target);

    /* 落盘: 顺序保存所有受影响的表 */
    int err = 0;
    if (save_visits_to_file(v_head) != 0)
        err++;
    if (save_exams_to_file(e_head) != 0)
        err++;
    if (save_hospitalizations_to_file(h_head) != 0)
        err++;
    if (save_beds_to_file(bed_head) != 0)
        err++;
    if (save_wards_to_file(ward_head) != 0)
        err++;
    if (save_prescriptions_to_file(pr_head) != 0)
        err++;

    if (err == 0)
        printf("删除成功！\n");
    else
        printf("删除完成，但有 %d 个文件保存失败，请检查数据文件！\n", err);

    free_patients(p_head);
    free_doctors(d_head);
    free_registrations(reg_head);
    free_visits(v_head);
    free_exams(e_head);
    free_hospitalizations(h_head);
    free_wards(ward_head);
    free_beds(bed_head);
    free_prescriptions(pr_head);
    wait_enter();
    clear_screen();
}

/* 检查记录删除 */
void delete_exam(void)
{
    char e_id[MAX_ID_LEN];
    printf("请输入要删除的检查ID(输入0返回): ");
    safe_input(e_id, sizeof(e_id));
    if (strcmp(e_id, "0") == 0)
    {
        clear_screen();
        return;
    }

    if (e_id[0] == '\0')
    {
        printf("输入错误！检查ID不能为空。\n");
        wait_enter();
        clear_screen();
        return;
    }

    Patient *p_head = load_patients_from_file();
    Doctor *d_head = load_doctors_from_file();
    Registration *reg_head = load_registrations_from_file();
    Visit *v_head = load_visits_from_file();
    Exam *e_head = load_exams_from_file();

    Exam *target = find_exam_by_e_id(e_head, e_id);
    if (!target)
    {
        printf("未找到指定ID的检查！\n");
        free_patients(p_head);
        free_doctors(d_head);
        free_registrations(reg_head);
        free_visits(v_head);
        free_exams(e_head);
        wait_enter();
        clear_screen();
        return;
    }

    /* 显示待删除检查 */
    int exam_w, ev_w, ep_w, ed_w, edept_w, item_w, result_w;
    calc_exam_width(e_head, v_head, reg_head, p_head, d_head, &exam_w, &ev_w, &ep_w, &ed_w, &edept_w, &item_w,
                    &result_w);
    printf("\n待删除检查:\n");
    print_exam_header(exam_w, ev_w, ep_w, ed_w, edept_w, item_w, result_w);
    print_exam(target, v_head, reg_head, p_head, d_head, exam_w, ev_w, ep_w, ed_w, edept_w, item_w, result_w);
    print_exam_line(exam_w, ev_w, ep_w, ed_w, edept_w, item_w, result_w);

    printf("确定要删除吗？(y/n): ");
    char confirm[MAX_INPUT_LEN];
    safe_input(confirm, sizeof(confirm));
    if (strcmp(confirm, "n") == 0 || strcmp(confirm, "N") == 0)
    {
        printf("已取消删除。\n");
        free_patients(p_head);
        free_doctors(d_head);
        free_registrations(reg_head);
        free_visits(v_head);
        free_exams(e_head);
        wait_enter();
        clear_screen();
        return;
    }
    else if (strcmp(confirm, "y") != 0 && strcmp(confirm, "Y") != 0)
    {
        printf("输入无效，已取消删除。\n");
        free_patients(p_head);
        free_doctors(d_head);
        free_registrations(reg_head);
        free_visits(v_head);
        free_exams(e_head);
        wait_enter();
        clear_screen();
        return;
    }

    /* 执行删除 */
    exam_remove(&e_head, target);

    if (save_exams_to_file(e_head) != 0)
        printf("删除失败：保存检查文件失败！\n");
    else
        printf("删除成功！\n");

    free_patients(p_head);
    free_doctors(d_head);
    free_registrations(reg_head);
    free_visits(v_head);
    free_exams(e_head);
    wait_enter();
    clear_screen();
}

/* 住院记录删除 */
void delete_hospitalization(void)
{
    char h_id[MAX_ID_LEN];
    printf("请输入要删除的住院ID(输入0返回): ");
    safe_input(h_id, sizeof(h_id));
    if (strcmp(h_id, "0") == 0)
    {
        clear_screen();
        return;
    }

    if (h_id[0] == '\0')
    {
        printf("输入错误！住院ID不能为空。\n");
        wait_enter();
        clear_screen();
        return;
    }

    Patient *p_head = load_patients_from_file();
    Registration *reg_head = load_registrations_from_file();
    Visit *v_head = load_visits_from_file();
    Hospitalization *h_head = load_hospitalizations_from_file();
    Ward *ward_head = load_wards_from_file();
    Bed *bed_head = load_beds_from_file();

    Hospitalization *target = find_hospitalization_by_h_id(h_head, h_id);
    if (!target)
    {
        printf("未找到指定ID的住院记录！\n");
        free_patients(p_head);
        free_registrations(reg_head);
        free_visits(v_head);
        free_hospitalizations(h_head);
        free_wards(ward_head);
        free_beds(bed_head);
        wait_enter();
        clear_screen();
        return;
    }

    /* 显示待删除住院 */
    int hosp_w, hv_w, hp_w, ward_w, bed_w, in_w, out_w, hst_w;
    calc_hospitalization_width(h_head, v_head, reg_head, p_head, ward_head, bed_head, &hosp_w, &hv_w, &hp_w, &ward_w,
                               &bed_w, &in_w, &out_w, &hst_w);
    printf("\n待删除住院:\n");
    print_hospitalization_header(hosp_w, hv_w, hp_w, ward_w, bed_w, in_w, out_w, hst_w);
    print_hospitalization(target, v_head, reg_head, p_head, ward_head, bed_head, hosp_w, hv_w, hp_w, ward_w, bed_w,
                          in_w, out_w, hst_w);
    print_hospitalization_line(hosp_w, hv_w, hp_w, ward_w, bed_w, in_w, out_w, hst_w);

    if (target->status == HOSP_STATUS_ONGOING)
        printf("\n注意: 该患者仍在住院中, 删除后将自动释放床位 %s。\n", target->bed_id);

    printf("确定要删除吗？(y/n): ");
    char confirm[MAX_INPUT_LEN];
    safe_input(confirm, sizeof(confirm));
    if (strcmp(confirm, "n") == 0 || strcmp(confirm, "N") == 0)
    {
        printf("已取消删除。\n");
        free_patients(p_head);
        free_registrations(reg_head);
        free_visits(v_head);
        free_hospitalizations(h_head);
        free_wards(ward_head);
        free_beds(bed_head);
        wait_enter();
        clear_screen();
        return;
    }
    else if (strcmp(confirm, "y") != 0 && strcmp(confirm, "Y") != 0)
    {
        printf("输入无效，已取消删除。\n");
        free_patients(p_head);
        free_registrations(reg_head);
        free_visits(v_head);
        free_hospitalizations(h_head);
        free_wards(ward_head);
        free_beds(bed_head);
        wait_enter();
        clear_screen();
        return;
    }

    /* 释放床位与病房占用(仅住院中时需要) */
    if (target->status == HOSP_STATUS_ONGOING)
    {
        Bed *b = find_bed_by_b_id(bed_head, target->bed_id);
        Ward *w = find_ward_by_w_id(ward_head, target->ward_id);
        if (b)
            b->status = BED_STATUS_FREE;
        if (w && w->occupied > 0)
            w->occupied -= 1;
    }

    /* 执行删除 */
    hospitalization_remove(&h_head, target);

    /* 落盘 */
    int err = 0;
    if (save_hospitalizations_to_file(h_head) != 0)
        err++;
    if (save_beds_to_file(bed_head) != 0)
        err++;
    if (save_wards_to_file(ward_head) != 0)
        err++;

    if (err == 0)
        printf("删除成功！\n");
    else
        printf("删除完成，但有 %d 个文件保存失败，请检查数据文件！\n", err);

    free_patients(p_head);
    free_registrations(reg_head);
    free_visits(v_head);
    free_hospitalizations(h_head);
    free_wards(ward_head);
    free_beds(bed_head);
    wait_enter();
    clear_screen();
}

/* 处方记录删除 */
void delete_prescription(void)
{
    char pr_id[MAX_ID_LEN];
    printf("请输入要删除的处方ID(输入0返回): ");
    safe_input(pr_id, sizeof(pr_id));
    if (strcmp(pr_id, "0") == 0)
    {
        clear_screen();
        return;
    }

    if (pr_id[0] == '\0')
    {
        printf("输入错误！处方ID不能为空。\n");
        wait_enter();
        clear_screen();
        return;
    }

    Patient *p_head = load_patients_from_file();
    Doctor *d_head = load_doctors_from_file();
    Prescription *pr_head = load_prescriptions_from_file();
    Drug *drug_head = load_drugs_from_file();

    Prescription *target = find_prescription_by_pr_id(pr_head, pr_id);
    if (!target)
    {
        printf("未找到指定ID的处方！\n");
        free_patients(p_head);
        free_doctors(d_head);
        free_prescriptions(pr_head);
        free_drugs(drug_head);
        wait_enter();
        clear_screen();
        return;
    }

    /* 显示待删除处方 */
    int pr_w, prv_w, prd_w, prp_w, drug_w, dose_w, freq_w;
    calc_prescription_width(pr_head, p_head, d_head, drug_head, &pr_w, &prv_w, &prd_w, &prp_w, &drug_w, &dose_w,
                            &freq_w);
    printf("\n待删除处方:\n");
    print_prescription_header(pr_w, prv_w, prd_w, prp_w, drug_w, dose_w, freq_w);
    print_prescription(target, p_head, d_head, drug_head, pr_w, prv_w, prd_w, prp_w, drug_w, dose_w, freq_w);
    print_prescription_line(pr_w, prv_w, prd_w, prp_w, drug_w, dose_w, freq_w);

    printf("确定要删除吗？(y/n): ");
    char confirm[MAX_INPUT_LEN];
    safe_input(confirm, sizeof(confirm));
    if (strcmp(confirm, "n") == 0 || strcmp(confirm, "N") == 0)
    {
        printf("已取消删除。\n");
        free_patients(p_head);
        free_doctors(d_head);
        free_prescriptions(pr_head);
        free_drugs(drug_head);
        wait_enter();
        clear_screen();
        return;
    }
    else if (strcmp(confirm, "y") != 0 && strcmp(confirm, "Y") != 0)
    {
        printf("输入无效，已取消删除。\n");
        free_patients(p_head);
        free_doctors(d_head);
        free_prescriptions(pr_head);
        free_drugs(drug_head);
        wait_enter();
        clear_screen();
        return;
    }

    /* 执行删除 */
    prescription_remove(&pr_head, target);

    if (save_prescriptions_to_file(pr_head) != 0)
        printf("删除失败：保存处方文件失败！\n");
    else
        printf("删除成功！\n");

    free_patients(p_head);
    free_doctors(d_head);
    free_prescriptions(pr_head);
    free_drugs(drug_head);
    wait_enter();
    clear_screen();
}