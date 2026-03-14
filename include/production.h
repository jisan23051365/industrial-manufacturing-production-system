/*
 * production.h
 * Production process management interface.
 */

#ifndef PRODUCTION_H
#define PRODUCTION_H

#include "manufacturing.h"

void production_menu(void);

/* Expose the in-memory store so other modules can read it */
extern ProductionProcess g_processes[];
extern int               g_process_count;

#endif /* PRODUCTION_H */
