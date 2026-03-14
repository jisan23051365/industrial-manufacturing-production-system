/*
 * manufacturing.h
 * Common types, constants, and utility prototypes for the
 * Industrial Manufacturing Production System.
 */

#ifndef MANUFACTURING_H
#define MANUFACTURING_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ------------------------------------------------------------------ */
/* Constants                                                           */
/* ------------------------------------------------------------------ */

#define MAX_NAME_LEN              64
#define MAX_UNIT_LEN              16
#define MAX_DESCRIPTION_LEN      256
#define MAX_MATERIALS_PER_PROCESS 10
#define MAX_RECORDS               500

/* File paths */
#define DATA_DIR                  "data"
#define RAW_MATERIALS_FILE        "data/raw_materials.dat"
#define PRODUCTION_FILE           "data/production_processes.dat"
#define FINISHED_PRODUCTS_FILE    "data/finished_products.dat"

/* ------------------------------------------------------------------ */
/* Date                                                                */
/* ------------------------------------------------------------------ */

typedef struct {
    int day;
    int month;
    int year;
} Date;

/* ------------------------------------------------------------------ */
/* Production process status                                           */
/* ------------------------------------------------------------------ */

typedef enum {
    STATUS_PLANNED      = 0,
    STATUS_IN_PROGRESS  = 1,
    STATUS_COMPLETED    = 2,
    STATUS_CANCELLED    = 3
} ProcessStatus;

/* ------------------------------------------------------------------ */
/* Core data structures                                                */
/* ------------------------------------------------------------------ */

typedef struct {
    int    id;
    char   name[MAX_NAME_LEN];
    double quantity;
    char   unit[MAX_UNIT_LEN];
    double reorder_level;
    double cost_per_unit;
    int    active; /* 1 = active, 0 = deleted */
} RawMaterial;

typedef struct {
    int           id;
    char          name[MAX_NAME_LEN];
    ProcessStatus status;
    Date          start_date;
    Date          end_date;           /* planned / actual completion */
    int           material_ids[MAX_MATERIALS_PER_PROCESS];
    double        quantities_required[MAX_MATERIALS_PER_PROCESS];
    int           num_materials;
    int           output_product_id;  /* FinishedProduct id produced */
    double        output_quantity;
    int           active;
} ProductionProcess;

typedef struct {
    int    id;
    char   name[MAX_NAME_LEN];
    double quantity;
    char   unit[MAX_UNIT_LEN];
    double production_cost;
    double selling_price;
    int    active;
} FinishedProduct;

/* ------------------------------------------------------------------ */
/* Utility prototypes (implemented in main.c)                          */
/* ------------------------------------------------------------------ */

void  clear_input_buffer(void);
void  print_separator(void);
void  print_header(const char *title);
Date  get_current_date(void);
void  format_date(Date d, char *buf, int buf_size);
int   get_int_input(const char *prompt);
double get_double_input(const char *prompt);
void  get_string_input(const char *prompt, char *buf, int max_len);
const char *status_to_string(ProcessStatus s);

#endif /* MANUFACTURING_H */
