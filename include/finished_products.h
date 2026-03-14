/*
 * finished_products.h
 * Finished product inventory management interface.
 */

#ifndef FINISHED_PRODUCTS_H
#define FINISHED_PRODUCTS_H

#include "manufacturing.h"

void finished_products_menu(void);

/*
 * Add `qty` units to the finished product with the given `id`.
 * If no product with that id exists, a new record is created with
 * `name` and `production_cost`.  Returns the product index.
 */
int add_finished_product_stock(int id, const char *name, double qty,
                               double production_cost);

/* Expose the in-memory store so other modules can read it */
extern FinishedProduct g_products[];
extern int             g_product_count;

#endif /* FINISHED_PRODUCTS_H */
