/*
 * raw_materials.c
 * Raw material inventory management.
 *
 * Supports:
 *   - Add / view / edit / delete raw materials
 *   - Low-stock alert (quantity <= reorder_level)
 *   - Stock deduction used by the production module
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "raw_materials.h"
#include "file_io.h"

/* ------------------------------------------------------------------ */
/* In-memory store                                                     */
/* ------------------------------------------------------------------ */

RawMaterial g_materials[MAX_RECORDS];
int         g_material_count = 0;

/* ------------------------------------------------------------------ */
/* Internal helpers                                                    */
/* ------------------------------------------------------------------ */

static int next_material_id(void)
{
    int max = 0;
    for (int i = 0; i < g_material_count; i++)
        if (g_materials[i].id > max)
            max = g_materials[i].id;
    return max + 1;
}

static void load(void)
{
    g_material_count = load_raw_materials(g_materials, MAX_RECORDS);
}

static void save(void)
{
    save_raw_materials(g_materials, g_material_count);
}

/* ------------------------------------------------------------------ */
/* Public helpers                                                      */
/* ------------------------------------------------------------------ */

int find_raw_material_by_id(int id)
{
    for (int i = 0; i < g_material_count; i++)
        if (g_materials[i].id == id && g_materials[i].active)
            return i;
    return -1;
}

int deduct_raw_material(int index, double qty)
{
    if (index < 0 || index >= g_material_count)
        return -1;
    if (g_materials[index].quantity < qty)
        return -1;
    g_materials[index].quantity -= qty;
    save();
    return 0;
}

void list_raw_materials(void)
{
    int found = 0;
    printf("\n%-5s %-30s %12s %-8s %12s %12s\n",
           "ID", "Name", "Quantity", "Unit", "Reorder Lvl", "Cost/Unit");
    print_separator();
    for (int i = 0; i < g_material_count; i++) {
        if (!g_materials[i].active)
            continue;
        found = 1;
        RawMaterial *m = &g_materials[i];
        printf("%-5d %-30s %12.2f %-8s %12.2f %12.2f\n",
               m->id, m->name, m->quantity, m->unit,
               m->reorder_level, m->cost_per_unit);
    }
    if (!found)
        printf("  (no records)\n");
}

/* ------------------------------------------------------------------ */
/* CRUD operations                                                     */
/* ------------------------------------------------------------------ */

static void add_material(void)
{
    if (g_material_count >= MAX_RECORDS) {
        printf("  Storage limit reached.\n");
        return;
    }

    RawMaterial m;
    memset(&m, 0, sizeof(m));
    m.id     = next_material_id();
    m.active = 1;

    get_string_input("  Name           : ", m.name, MAX_NAME_LEN);
    m.quantity      = get_double_input("  Quantity       : ");
    get_string_input("  Unit (e.g. kg) : ", m.unit, MAX_UNIT_LEN);
    m.reorder_level = get_double_input("  Reorder Level  : ");
    m.cost_per_unit = get_double_input("  Cost per Unit  : ");

    g_materials[g_material_count++] = m;
    save();
    printf("  Raw material '%s' added (ID %d).\n", m.name, m.id);
}

static void edit_material(void)
{
    int id = get_int_input("  Enter Material ID to edit: ");
    int idx = find_raw_material_by_id(id);
    if (idx < 0) {
        printf("  Material ID %d not found.\n", id);
        return;
    }

    RawMaterial *m = &g_materials[idx];
    printf("  Editing '%s' (leave blank to keep current value)\n", m->name);

    char buf[MAX_NAME_LEN];

    get_string_input("  New Name       : ", buf, MAX_NAME_LEN);
    if (buf[0] != '\0')
        strncpy(m->name, buf, MAX_NAME_LEN - 1);

    printf("  Current Quantity: %.2f\n", m->quantity);
    get_string_input("  New Quantity   : ", buf, sizeof(buf));
    if (buf[0] != '\0')
        m->quantity = atof(buf);

    get_string_input("  New Unit       : ", buf, MAX_UNIT_LEN);
    if (buf[0] != '\0')
        strncpy(m->unit, buf, MAX_UNIT_LEN - 1);

    printf("  Current Reorder Level: %.2f\n", m->reorder_level);
    get_string_input("  New Reorder Lvl: ", buf, sizeof(buf));
    if (buf[0] != '\0')
        m->reorder_level = atof(buf);

    printf("  Current Cost/Unit: %.2f\n", m->cost_per_unit);
    get_string_input("  New Cost/Unit  : ", buf, sizeof(buf));
    if (buf[0] != '\0')
        m->cost_per_unit = atof(buf);

    save();
    printf("  Material updated.\n");
}

static void delete_material(void)
{
    int id = get_int_input("  Enter Material ID to delete: ");
    int idx = find_raw_material_by_id(id);
    if (idx < 0) {
        printf("  Material ID %d not found.\n", id);
        return;
    }
    g_materials[idx].active = 0;
    save();
    printf("  Material ID %d deleted.\n", id);
}

static void low_stock_report(void)
{
    int found = 0;
    printf("\n  --- Low Stock Alert ---\n");
    printf("  %-5s %-30s %12s %12s\n",
           "ID", "Name", "Quantity", "Reorder Lvl");
    print_separator();
    for (int i = 0; i < g_material_count; i++) {
        RawMaterial *m = &g_materials[i];
        if (!m->active)
            continue;
        if (m->quantity <= m->reorder_level) {
            printf("  %-5d %-30s %12.2f %12.2f  <<< LOW\n",
                   m->id, m->name, m->quantity, m->reorder_level);
            found = 1;
        }
    }
    if (!found)
        printf("  All materials are adequately stocked.\n");
}

/* ------------------------------------------------------------------ */
/* Menu                                                                */
/* ------------------------------------------------------------------ */

void raw_materials_menu(void)
{
    load();
    int choice;
    do {
        print_header("Raw Materials Management");
        printf("  1. Add Raw Material\n");
        printf("  2. View All Raw Materials\n");
        printf("  3. Edit Raw Material\n");
        printf("  4. Delete Raw Material\n");
        printf("  5. Low Stock Alert\n");
        printf("  0. Back\n");
        print_separator();
        choice = get_int_input("  Choice: ");

        switch (choice) {
        case 1: add_material();     break;
        case 2: list_raw_materials(); break;
        case 3: edit_material();    break;
        case 4: delete_material();  break;
        case 5: low_stock_report(); break;
        case 0: break;
        default: printf("  Invalid option.\n");
        }
    } while (choice != 0);
}
