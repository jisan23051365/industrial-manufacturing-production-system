/*
 * analytics.c
 * Production analytics and reporting.
 *
 * Reports:
 *   1. Inventory Value   – total value of raw material stock
 *   2. Finished Goods    – total value and margin of finished product stock
 *   3. Production Summary – processes by status
 *   4. Material Consumption – materials consumed in completed processes
 *   5. Low Stock Report   – materials at or below reorder level
 *   6. Cost Analysis      – production cost vs selling price per product
 */

#include <stdio.h>
#include "analytics.h"
#include "manufacturing.h"
#include "raw_materials.h"
#include "finished_products.h"
#include "production.h"
#include "file_io.h"

/* ------------------------------------------------------------------ */
/* Load all data into the shared in-memory stores                      */
/* ------------------------------------------------------------------ */

static void reload_all(void)
{
    g_material_count = load_raw_materials(g_materials, MAX_RECORDS);
    g_product_count  = load_finished_products(g_products, MAX_RECORDS);
    g_process_count  = load_production_processes(g_processes, MAX_RECORDS);
}

/* ------------------------------------------------------------------ */
/* Report 1 – Raw Material Inventory Value                             */
/* ------------------------------------------------------------------ */

static void report_inventory_value(void)
{
    print_header("Raw Material Inventory Value");
    printf("  %-30s %10s %12s %14s\n",
           "Name", "Quantity", "Cost/Unit", "Total Value");
    print_separator();

    double grand_total = 0.0;
    int found = 0;
    for (int i = 0; i < g_material_count; i++) {
        if (!g_materials[i].active)
            continue;
        found = 1;
        RawMaterial *m = &g_materials[i];
        double val = m->quantity * m->cost_per_unit;
        grand_total += val;
        printf("  %-30s %10.2f %12.2f %14.2f\n",
               m->name, m->quantity, m->cost_per_unit, val);
    }
    if (!found)
        printf("  (no raw materials)\n");

    print_separator();
    printf("  %-30s %10s %12s %14.2f\n",
           "TOTAL", "", "", grand_total);
}

/* ------------------------------------------------------------------ */
/* Report 2 – Finished Goods Value & Margin                           */
/* ------------------------------------------------------------------ */

static void report_finished_goods(void)
{
    print_header("Finished Goods Inventory & Margin");
    printf("  %-30s %10s %12s %14s %14s\n",
           "Name", "Quantity", "Prod. Cost", "Sell Price", "Total Margin");
    print_separator();

    double total_value  = 0.0;
    double total_margin = 0.0;
    int found = 0;
    for (int i = 0; i < g_product_count; i++) {
        if (!g_products[i].active)
            continue;
        found = 1;
        FinishedProduct *p = &g_products[i];
        double value  = p->quantity * p->selling_price;
        double margin = p->quantity * (p->selling_price - p->production_cost);
        total_value  += value;
        total_margin += margin;
        printf("  %-30s %10.2f %12.2f %14.2f %14.2f\n",
               p->name, p->quantity, p->production_cost,
               p->selling_price, margin);
    }
    if (!found)
        printf("  (no finished products)\n");

    print_separator();
    printf("  %-30s %10s %12s %14.2f %14.2f\n",
           "TOTAL", "", "", total_value, total_margin);
}

/* ------------------------------------------------------------------ */
/* Report 3 – Production Summary by Status                            */
/* ------------------------------------------------------------------ */

static void report_production_summary(void)
{
    print_header("Production Summary");
    int counts[4] = {0, 0, 0, 0};
    for (int i = 0; i < g_process_count; i++) {
        if (!g_processes[i].active)
            continue;
        int s = (int)g_processes[i].status;
        if (s >= 0 && s < 4)
            counts[s]++;
    }
    printf("  %-20s : %d\n", "Planned",     counts[STATUS_PLANNED]);
    printf("  %-20s : %d\n", "In Progress",  counts[STATUS_IN_PROGRESS]);
    printf("  %-20s : %d\n", "Completed",    counts[STATUS_COMPLETED]);
    printf("  %-20s : %d\n", "Cancelled",    counts[STATUS_CANCELLED]);
    print_separator();
    printf("  %-20s : %d\n", "Total Active",
           counts[0] + counts[1] + counts[2] + counts[3]);
}

/* ------------------------------------------------------------------ */
/* Report 4 – Material Consumption (completed processes)              */
/* ------------------------------------------------------------------ */

static void report_material_consumption(void)
{
    print_header("Material Consumption Report (Completed Processes)");

    /* Aggregate consumption per material id */
    int   ids[MAX_RECORDS];
    double totals[MAX_RECORDS];
    int    cnt = 0;

    for (int i = 0; i < g_process_count; i++) {
        ProductionProcess *pr = &g_processes[i];
        if (!pr->active || pr->status != STATUS_COMPLETED)
            continue;
        for (int j = 0; j < pr->num_materials; j++) {
            int found = 0;
            for (int k = 0; k < cnt; k++) {
                if (ids[k] == pr->material_ids[j]) {
                    totals[k] += pr->quantities_required[j];
                    found = 1;
                    break;
                }
            }
            if (!found && cnt < MAX_RECORDS) {
                ids[cnt]    = pr->material_ids[j];
                totals[cnt] = pr->quantities_required[j];
                cnt++;
            }
        }
    }

    if (cnt == 0) {
        printf("  No completed processes found.\n");
        return;
    }

    printf("  %-5s %-30s %14s\n", "ID", "Material", "Total Consumed");
    print_separator();
    for (int k = 0; k < cnt; k++) {
        int midx = find_raw_material_by_id(ids[k]);
        const char *name = (midx >= 0) ? g_materials[midx].name : "Unknown";
        printf("  %-5d %-30s %14.2f\n", ids[k], name, totals[k]);
    }
}

/* ------------------------------------------------------------------ */
/* Report 5 – Low Stock                                               */
/* ------------------------------------------------------------------ */

static void report_low_stock(void)
{
    print_header("Low Stock Report");
    int found = 0;
    printf("  %-5s %-30s %12s %12s\n",
           "ID", "Name", "Quantity", "Reorder Lvl");
    print_separator();
    for (int i = 0; i < g_material_count; i++) {
        RawMaterial *m = &g_materials[i];
        if (!m->active)
            continue;
        if (m->quantity <= m->reorder_level) {
            printf("  %-5d %-30s %12.2f %12.2f  *** LOW ***\n",
                   m->id, m->name, m->quantity, m->reorder_level);
            found = 1;
        }
    }
    if (!found)
        printf("  All materials adequately stocked.\n");
}

/* ------------------------------------------------------------------ */
/* Report 6 – Cost Analysis                                           */
/* ------------------------------------------------------------------ */

static void report_cost_analysis(void)
{
    print_header("Cost Analysis – Finished Products");
    printf("  %-30s %12s %14s %12s\n",
           "Product", "Prod. Cost", "Sell Price", "Margin %%");
    print_separator();
    int found = 0;
    for (int i = 0; i < g_product_count; i++) {
        if (!g_products[i].active)
            continue;
        found = 1;
        FinishedProduct *p = &g_products[i];
        double margin_pct = 0.0;
        if (p->selling_price > 0.0)
            margin_pct = ((p->selling_price - p->production_cost)
                          / p->selling_price) * 100.0;
        printf("  %-30s %12.2f %14.2f %11.1f%%\n",
               p->name, p->production_cost, p->selling_price, margin_pct);
    }
    if (!found)
        printf("  (no finished products)\n");
}

/* ------------------------------------------------------------------ */
/* Menu                                                                */
/* ------------------------------------------------------------------ */

void analytics_menu(void)
{
    reload_all();
    int choice;
    do {
        print_header("Production Analytics");
        printf("  1. Raw Material Inventory Value\n");
        printf("  2. Finished Goods Value & Margin\n");
        printf("  3. Production Summary by Status\n");
        printf("  4. Material Consumption Report\n");
        printf("  5. Low Stock Report\n");
        printf("  6. Cost Analysis\n");
        printf("  0. Back\n");
        print_separator();
        choice = get_int_input("  Choice: ");

        switch (choice) {
        case 1: report_inventory_value();    break;
        case 2: report_finished_goods();     break;
        case 3: report_production_summary(); break;
        case 4: report_material_consumption(); break;
        case 5: report_low_stock();          break;
        case 6: report_cost_analysis();      break;
        case 0: break;
        default: printf("  Invalid option.\n");
        }
    } while (choice != 0);
}
