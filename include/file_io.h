/*
 * file_io.h
 * Binary file storage interface for the Manufacturing Production System.
 * Each entity type is persisted in its own .dat file under data/.
 */

#ifndef FILE_IO_H
#define FILE_IO_H

#include "manufacturing.h"

/* Create the data/ directory if it does not exist. */
void ensure_data_dir(void);

/* ------ Raw Materials -------------------------------------------- */
int load_raw_materials(RawMaterial *materials, int max_count);
int save_raw_materials(const RawMaterial *materials, int count);

/* ------ Production Processes ------------------------------------- */
int load_production_processes(ProductionProcess *processes, int max_count);
int save_production_processes(const ProductionProcess *processes, int count);

/* ------ Finished Products ---------------------------------------- */
int load_finished_products(FinishedProduct *products, int max_count);
int save_finished_products(const FinishedProduct *products, int count);

#endif /* FILE_IO_H */
