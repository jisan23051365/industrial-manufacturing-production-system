/*
 * finished_products.c
 * Finished product inventory management.
 *
 * Supports:
 *   - Add / view / edit / delete finished products
 *   - Stock top-up (used by the production module on process completion)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "finished_products.h"
#include "file_io.h"

/* ------------------------------------------------------------------ */
/* In-memory store                                                     */
/* ------------------------------------------------------------------ */

FinishedProduct g_products[MAX_RECORDS];
int             g_product_count = 0;

/* ------------------------------------------------------------------ */
/* Internal helpers                                                    */
/* ------------------------------------------------------------------ */

static int next_product_id(void)
{
    int max = 0;
    for (int i = 0; i < g_product_count; i++)
        if (g_products[i].id > max)
            max = g_products[i].id;
    return max + 1;
}

static void load(void)
{
    g_product_count = load_finished_products(g_products, MAX_RECORDS);
}

static void save(void)
{
    save_finished_products(g_products, g_product_count);
}

static int find_by_id(int id)
{
    for (int i = 0; i < g_product_count; i++)
        if (g_products[i].id == id && g_products[i].active)
            return i;
    return -1;
}

static void list_products(void)
{
    int found = 0;
    printf("\n%-5s %-30s %12s %-8s %14s %14s\n",
           "ID", "Name", "Quantity", "Unit", "Prod. Cost", "Selling Price");
    print_separator();
    for (int i = 0; i < g_product_count; i++) {
        if (!g_products[i].active)
            continue;
        found = 1;
        FinishedProduct *p = &g_products[i];
        printf("%-5d %-30s %12.2f %-8s %14.2f %14.2f\n",
               p->id, p->name, p->quantity, p->unit,
               p->production_cost, p->selling_price);
    }
    if (!found)
        printf("  (no records)\n");
}

/* ------------------------------------------------------------------ */
/* Public stock-add helper (called by production module)               */
/* ------------------------------------------------------------------ */

int add_finished_product_stock(int id, const char *name, double qty,
                               double production_cost)
{
    int idx = find_by_id(id);
    if (idx >= 0) {
        g_products[idx].quantity       += qty;
        g_products[idx].production_cost = production_cost;
        save();
        return idx;
    }
    /* Create new entry */
    if (g_product_count >= MAX_RECORDS)
        return -1;

    FinishedProduct p;
    memset(&p, 0, sizeof(p));
    p.id              = (id > 0) ? id : next_product_id();
    strncpy(p.name, name ? name : "Unknown", MAX_NAME_LEN - 1);
    p.quantity        = qty;
    strncpy(p.unit, "unit", MAX_UNIT_LEN - 1);
    p.production_cost = production_cost;
    p.selling_price   = 0.0;
    p.active          = 1;

    g_products[g_product_count++] = p;
    save();
    return g_product_count - 1;
}

/* ------------------------------------------------------------------ */
/* CRUD operations                                                     */
/* ------------------------------------------------------------------ */

static void add_product(void)
{
    if (g_product_count >= MAX_RECORDS) {
        printf("  Storage limit reached.\n");
        return;
    }

    FinishedProduct p;
    memset(&p, 0, sizeof(p));
    p.id     = next_product_id();
    p.active = 1;

    get_string_input("  Name              : ", p.name, MAX_NAME_LEN);
    p.quantity        = get_double_input("  Quantity          : ");
    get_string_input("  Unit (e.g. pcs)   : ", p.unit, MAX_UNIT_LEN);
    p.production_cost = get_double_input("  Production Cost   : ");
    p.selling_price   = get_double_input("  Selling Price     : ");

    g_products[g_product_count++] = p;
    save();
    printf("  Product '%s' added (ID %d).\n", p.name, p.id);
}

static void edit_product(void)
{
    int id = get_int_input("  Enter Product ID to edit: ");
    int idx = find_by_id(id);
    if (idx < 0) {
        printf("  Product ID %d not found.\n", id);
        return;
    }

    FinishedProduct *p = &g_products[idx];
    printf("  Editing '%s' (leave blank to keep current value)\n", p->name);

    char buf[MAX_NAME_LEN];

    get_string_input("  New Name          : ", buf, MAX_NAME_LEN);
    if (buf[0] != '\0')
        strncpy(p->name, buf, MAX_NAME_LEN - 1);

    printf("  Current Quantity: %.2f\n", p->quantity);
    get_string_input("  New Quantity      : ", buf, sizeof(buf));
    if (buf[0] != '\0')
        p->quantity = atof(buf);

    get_string_input("  New Unit          : ", buf, MAX_UNIT_LEN);
    if (buf[0] != '\0')
        strncpy(p->unit, buf, MAX_UNIT_LEN - 1);

    printf("  Current Production Cost: %.2f\n", p->production_cost);
    get_string_input("  New Production Cost: ", buf, sizeof(buf));
    if (buf[0] != '\0')
        p->production_cost = atof(buf);

    printf("  Current Selling Price: %.2f\n", p->selling_price);
    get_string_input("  New Selling Price  : ", buf, sizeof(buf));
    if (buf[0] != '\0')
        p->selling_price = atof(buf);

    save();
    printf("  Product updated.\n");
}

static void delete_product(void)
{
    int id = get_int_input("  Enter Product ID to delete: ");
    int idx = find_by_id(id);
    if (idx < 0) {
        printf("  Product ID %d not found.\n", id);
        return;
    }
    g_products[idx].active = 0;
    save();
    printf("  Product ID %d deleted.\n", id);
}

/* ------------------------------------------------------------------ */
/* Menu                                                                */
/* ------------------------------------------------------------------ */

void finished_products_menu(void)
{
    load();
    int choice;
    do {
        print_header("Finished Products Management");
        printf("  1. Add Finished Product\n");
        printf("  2. View All Finished Products\n");
        printf("  3. Edit Finished Product\n");
        printf("  4. Delete Finished Product\n");
        printf("  0. Back\n");
        print_separator();
        choice = get_int_input("  Choice: ");

        switch (choice) {
        case 1: add_product();    break;
        case 2: list_products();  break;
        case 3: edit_product();   break;
        case 4: delete_product(); break;
        case 0: break;
        default: printf("  Invalid option.\n");
        }
    } while (choice != 0);
}
