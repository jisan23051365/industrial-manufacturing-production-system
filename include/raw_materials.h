/*
 * raw_materials.h
 * Raw material inventory management interface.
 */

#ifndef RAW_MATERIALS_H
#define RAW_MATERIALS_H

#include "manufacturing.h"

void raw_materials_menu(void);

/* CRUD helpers (used internally and by production module) */
int  find_raw_material_by_id(int id);        /* returns index or -1 */
void list_raw_materials(void);

/*
 * Deduct `qty` units from material at `index`.
 * Returns 0 on success, -1 if insufficient stock.
 */
int  deduct_raw_material(int index, double qty);

/* Expose the in-memory store so other modules can read it */
extern RawMaterial g_materials[];
extern int         g_material_count;

#endif /* RAW_MATERIALS_H */
