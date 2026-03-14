/*
 * production.c
 * Production process management.
 *
 * Supports:
 *   - Create / view / edit / delete production processes
 *   - Start production  – deducts required raw materials from inventory
 *   - Complete production – credits output quantity to finished products
 *   - Cancel production   – restores deducted materials
 */

#include <stdio.h>
#include <string.h>
#include "production.h"
#include "raw_materials.h"
#include "finished_products.h"
#include "file_io.h"

/* ------------------------------------------------------------------ */
/* In-memory store                                                     */
/* ------------------------------------------------------------------ */

ProductionProcess g_processes[MAX_RECORDS];
int               g_process_count = 0;

/* ------------------------------------------------------------------ */
/* Internal helpers                                                    */
/* ------------------------------------------------------------------ */

static int next_process_id(void)
{
    int max = 0;
    for (int i = 0; i < g_process_count; i++)
        if (g_processes[i].id > max)
            max = g_processes[i].id;
    return max + 1;
}

static void load(void)
{
    g_process_count = load_production_processes(g_processes, MAX_RECORDS);
    /* Also reload dependent stores so deductions are consistent */
    g_material_count = load_raw_materials(g_materials, MAX_RECORDS);
    g_product_count  = load_finished_products(g_products, MAX_RECORDS);
}

static void save_all(void)
{
    save_production_processes(g_processes, g_process_count);
    save_raw_materials(g_materials, g_material_count);
    save_finished_products(g_products, g_product_count);
}

static int find_by_id(int id)
{
    for (int i = 0; i < g_process_count; i++)
        if (g_processes[i].id == id && g_processes[i].active)
            return i;
    return -1;
}

static void print_process(const ProductionProcess *p)
{
    char sd[32], ed[32];
    format_date(p->start_date, sd, sizeof(sd));
    format_date(p->end_date,   ed, sizeof(ed));

    printf("\n  ID            : %d\n",   p->id);
    printf("  Name          : %s\n",    p->name);
    printf("  Status        : %s\n",    status_to_string(p->status));
    printf("  Start Date    : %s\n",    sd);
    printf("  End Date      : %s\n",    ed);
    printf("  Output Product: ID %d, Qty %.2f\n",
           p->output_product_id, p->output_quantity);
    printf("  Materials     :\n");
    for (int j = 0; j < p->num_materials; j++) {
        int midx = find_raw_material_by_id(p->material_ids[j]);
        const char *mname = (midx >= 0) ? g_materials[midx].name : "?";
        printf("    - ID %d (%s): %.2f\n",
               p->material_ids[j], mname,
               p->quantities_required[j]);
    }
}

/* ------------------------------------------------------------------ */
/* CRUD operations                                                     */
/* ------------------------------------------------------------------ */

static void create_process(void)
{
    if (g_process_count >= MAX_RECORDS) {
        printf("  Storage limit reached.\n");
        return;
    }

    ProductionProcess pr;
    memset(&pr, 0, sizeof(pr));
    pr.id     = next_process_id();
    pr.active = 1;
    pr.status = STATUS_PLANNED;

    get_string_input("  Process Name         : ", pr.name, MAX_NAME_LEN);

    printf("  Start Date\n");
    pr.start_date.day   = get_int_input("    Day   : ");
    pr.start_date.month = get_int_input("    Month : ");
    pr.start_date.year  = get_int_input("    Year  : ");

    printf("  Planned End Date\n");
    pr.end_date.day   = get_int_input("    Day   : ");
    pr.end_date.month = get_int_input("    Month : ");
    pr.end_date.year  = get_int_input("    Year  : ");

    /* Output product */
    printf("\n  Available Finished Products:\n");
    int found = 0;
    for (int i = 0; i < g_product_count; i++) {
        if (g_products[i].active) {
            printf("    ID %d – %s\n", g_products[i].id, g_products[i].name);
            found = 1;
        }
    }
    if (!found)
        printf("    (none – a new product ID will be assigned)\n");

    pr.output_product_id = get_int_input("  Output Product ID (0=new): ");
    pr.output_quantity   = get_double_input("  Output Quantity          : ");

    /* Required raw materials */
    printf("\n  Available Raw Materials:\n");
    for (int i = 0; i < g_material_count; i++)
        if (g_materials[i].active)
            printf("    ID %d – %s (stock: %.2f %s)\n",
                   g_materials[i].id, g_materials[i].name,
                   g_materials[i].quantity, g_materials[i].unit);

    pr.num_materials = get_int_input(
        "\n  How many raw materials required? (max 10): ");
    if (pr.num_materials > MAX_MATERIALS_PER_PROCESS)
        pr.num_materials = MAX_MATERIALS_PER_PROCESS;
    if (pr.num_materials < 0)
        pr.num_materials = 0;

    for (int j = 0; j < pr.num_materials; j++) {
        printf("  Material %d:\n", j + 1);
        pr.material_ids[j]          = get_int_input("    Material ID  : ");
        pr.quantities_required[j]   = get_double_input("    Qty Required : ");
    }

    g_processes[g_process_count++] = pr;
    save_production_processes(g_processes, g_process_count);
    printf("  Process '%s' created (ID %d, status: Planned).\n",
           pr.name, pr.id);
}

static void view_all_processes(void)
{
    int found = 0;
    printf("\n%-5s %-30s %-14s %-12s %-12s\n",
           "ID", "Name", "Status", "Start", "End");
    print_separator();
    for (int i = 0; i < g_process_count; i++) {
        if (!g_processes[i].active)
            continue;
        found = 1;
        ProductionProcess *p = &g_processes[i];
        char sd[32], ed[32];
        format_date(p->start_date, sd, sizeof(sd));
        format_date(p->end_date,   ed, sizeof(ed));
        printf("%-5d %-30s %-14s %-12s %-12s\n",
               p->id, p->name, status_to_string(p->status), sd, ed);
    }
    if (!found)
        printf("  (no records)\n");
}

static void view_process_details(void)
{
    int id = get_int_input("  Enter Process ID: ");
    int idx = find_by_id(id);
    if (idx < 0) {
        printf("  Process ID %d not found.\n", id);
        return;
    }
    print_process(&g_processes[idx]);
}

static void start_production(void)
{
    int id = get_int_input("  Enter Process ID to start: ");
    int idx = find_by_id(id);
    if (idx < 0) {
        printf("  Process ID %d not found.\n", id);
        return;
    }
    ProductionProcess *p = &g_processes[idx];
    if (p->status != STATUS_PLANNED) {
        printf("  Process is not in Planned status (current: %s).\n",
               status_to_string(p->status));
        return;
    }

    /* Check all materials are available */
    for (int j = 0; j < p->num_materials; j++) {
        int midx = find_raw_material_by_id(p->material_ids[j]);
        if (midx < 0) {
            printf("  Raw material ID %d not found.\n", p->material_ids[j]);
            return;
        }
        if (g_materials[midx].quantity < p->quantities_required[j]) {
            printf("  Insufficient stock of '%s' (have %.2f, need %.2f).\n",
                   g_materials[midx].name,
                   g_materials[midx].quantity,
                   p->quantities_required[j]);
            return;
        }
    }

    /* Deduct materials */
    for (int j = 0; j < p->num_materials; j++) {
        int midx = find_raw_material_by_id(p->material_ids[j]);
        g_materials[midx].quantity -= p->quantities_required[j];
    }

    p->status     = STATUS_IN_PROGRESS;
    p->start_date = get_current_date();
    save_all();
    printf("  Process '%s' started. Raw materials deducted.\n", p->name);
}

static void complete_production(void)
{
    int id = get_int_input("  Enter Process ID to complete: ");
    int idx = find_by_id(id);
    if (idx < 0) {
        printf("  Process ID %d not found.\n", id);
        return;
    }
    ProductionProcess *p = &g_processes[idx];
    if (p->status != STATUS_IN_PROGRESS) {
        printf("  Process is not In Progress (current: %s).\n",
               status_to_string(p->status));
        return;
    }

    /* Calculate production cost from consumed materials */
    double cost = 0.0;
    for (int j = 0; j < p->num_materials; j++) {
        int midx = find_raw_material_by_id(p->material_ids[j]);
        if (midx >= 0)
            cost += p->quantities_required[j] *
                    g_materials[midx].cost_per_unit;
    }

    /* Credit output product */
    char pname[MAX_NAME_LEN] = "Product";
    int pidx = -1;
    for (int i = 0; i < g_product_count; i++)
        if (g_products[i].id == p->output_product_id && g_products[i].active) {
            pidx = i;
            strncpy(pname, g_products[i].name, MAX_NAME_LEN - 1);
            break;
        }

    if (pidx >= 0) {
        g_products[pidx].quantity       += p->output_quantity;
        g_products[pidx].production_cost = (p->output_quantity > 0)
                                            ? cost / p->output_quantity : 0.0;
    } else {
        /* Auto-create the product if it doesn't exist */
        if (g_product_count < MAX_RECORDS) {
            FinishedProduct np;
            memset(&np, 0, sizeof(np));
            np.id              = (p->output_product_id > 0)
                                  ? p->output_product_id : g_product_count + 1;
            snprintf(np.name, MAX_NAME_LEN, "Product-%d", np.id);
            np.quantity        = p->output_quantity;
            strncpy(np.unit, "unit", MAX_UNIT_LEN - 1);
            np.production_cost = (p->output_quantity > 0)
                                  ? cost / p->output_quantity : 0.0;
            np.selling_price   = 0.0;
            np.active          = 1;
            g_products[g_product_count++] = np;
        }
    }

    p->status   = STATUS_COMPLETED;
    p->end_date = get_current_date();
    save_all();
    printf("  Process '%s' completed.\n  %.2f units of '%s' added to "
           "finished goods inventory.\n  Production cost: %.2f\n",
           p->name, p->output_quantity, pname, cost);
}

static void cancel_production(void)
{
    int id = get_int_input("  Enter Process ID to cancel: ");
    int idx = find_by_id(id);
    if (idx < 0) {
        printf("  Process ID %d not found.\n", id);
        return;
    }
    ProductionProcess *p = &g_processes[idx];
    if (p->status == STATUS_COMPLETED || p->status == STATUS_CANCELLED) {
        printf("  Cannot cancel a %s process.\n",
               status_to_string(p->status));
        return;
    }

    /* Restore materials if the process was already started */
    if (p->status == STATUS_IN_PROGRESS) {
        for (int j = 0; j < p->num_materials; j++) {
            int midx = find_raw_material_by_id(p->material_ids[j]);
            if (midx >= 0)
                g_materials[midx].quantity += p->quantities_required[j];
        }
        printf("  Raw materials restored.\n");
    }

    p->status = STATUS_CANCELLED;
    save_all();
    printf("  Process '%s' cancelled.\n", p->name);
}

static void delete_process(void)
{
    int id = get_int_input("  Enter Process ID to delete: ");
    int idx = find_by_id(id);
    if (idx < 0) {
        printf("  Process ID %d not found.\n", id);
        return;
    }
    if (g_processes[idx].status == STATUS_IN_PROGRESS) {
        printf("  Cannot delete an In-Progress process. Cancel it first.\n");
        return;
    }
    g_processes[idx].active = 0;
    save_production_processes(g_processes, g_process_count);
    printf("  Process ID %d deleted.\n", id);
}

/* ------------------------------------------------------------------ */
/* Menu                                                                */
/* ------------------------------------------------------------------ */

void production_menu(void)
{
    load();
    int choice;
    do {
        print_header("Production Process Management");
        printf("  1. Create Production Process\n");
        printf("  2. View All Processes\n");
        printf("  3. View Process Details\n");
        printf("  4. Start Production\n");
        printf("  5. Complete Production\n");
        printf("  6. Cancel Production\n");
        printf("  7. Delete Process\n");
        printf("  0. Back\n");
        print_separator();
        choice = get_int_input("  Choice: ");

        switch (choice) {
        case 1: create_process();        break;
        case 2: view_all_processes();    break;
        case 3: view_process_details();  break;
        case 4: start_production();      break;
        case 5: complete_production();   break;
        case 6: cancel_production();     break;
        case 7: delete_process();        break;
        case 0: break;
        default: printf("  Invalid option.\n");
        }
    } while (choice != 0);
}
