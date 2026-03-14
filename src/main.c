/*
 * main.c
 * Entry point and shared utilities for the
 * Industrial Manufacturing Production System.
 *
 * Usage: ./manufacturing_system
 *
 * All data is persisted under the data/ directory using binary .dat files.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "manufacturing.h"
#include "file_io.h"
#include "raw_materials.h"
#include "production.h"
#include "finished_products.h"
#include "analytics.h"

/* ------------------------------------------------------------------ */
/* Utility implementations                                             */
/* ------------------------------------------------------------------ */

void clear_input_buffer(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

void print_separator(void)
{
    printf("  ");
    for (int i = 0; i < 76; i++)
        putchar('-');
    putchar('\n');
}

void print_header(const char *title)
{
    printf("\n");
    print_separator();
    printf("  %s\n", title);
    print_separator();
}

Date get_current_date(void)
{
    time_t t  = time(NULL);
    struct tm *tm_info = localtime(&t);
    Date d;
    d.day   = tm_info->tm_mday;
    d.month = tm_info->tm_mon + 1;
    d.year  = tm_info->tm_year + 1900;
    return d;
}

void format_date(Date d, char *buf, int buf_size)
{
    if (d.day == 0 && d.month == 0 && d.year == 0)
        snprintf(buf, (size_t)buf_size, "N/A");
    else
        snprintf(buf, (size_t)buf_size, "%02d/%02d/%04d",
                 d.day, d.month, d.year);
}

const char *status_to_string(ProcessStatus s)
{
    switch (s) {
    case STATUS_PLANNED:     return "Planned";
    case STATUS_IN_PROGRESS: return "In Progress";
    case STATUS_COMPLETED:   return "Completed";
    case STATUS_CANCELLED:   return "Cancelled";
    default:                 return "Unknown";
    }
}

int get_int_input(const char *prompt)
{
    char buf[64];
    printf("%s", prompt);
    if (!fgets(buf, sizeof(buf), stdin))
        return 0;
    return atoi(buf);
}

double get_double_input(const char *prompt)
{
    char buf[64];
    printf("%s", prompt);
    if (!fgets(buf, sizeof(buf), stdin))
        return 0.0;
    return atof(buf);
}

void get_string_input(const char *prompt, char *buf, int max_len)
{
    printf("%s", prompt);
    if (!fgets(buf, max_len, stdin))
        buf[0] = '\0';
    /* Strip trailing newline */
    size_t len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n')
        buf[len - 1] = '\0';
}

/* ------------------------------------------------------------------ */
/* Main menu                                                           */
/* ------------------------------------------------------------------ */

static void print_main_menu(void)
{
    print_header("Industrial Manufacturing Production System");
    printf("  1. Raw Materials Management\n");
    printf("  2. Production Process Management\n");
    printf("  3. Finished Products Management\n");
    printf("  4. Production Analytics\n");
    printf("  0. Exit\n");
    print_separator();
}

int main(void)
{
    ensure_data_dir();

    printf("\n  Welcome to the Industrial Manufacturing Production System\n");

    int choice;
    do {
        print_main_menu();
        choice = get_int_input("  Choice: ");

        switch (choice) {
        case 1: raw_materials_menu();    break;
        case 2: production_menu();       break;
        case 3: finished_products_menu(); break;
        case 4: analytics_menu();        break;
        case 0:
            printf("\n  Goodbye.\n\n");
            break;
        default:
            printf("  Invalid option. Please try again.\n");
        }
    } while (choice != 0);

    return 0;
}
