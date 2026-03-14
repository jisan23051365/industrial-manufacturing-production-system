/*
 * file_io.c
 * Binary file I/O for the Manufacturing Production System.
 *
 * Each entity is stored in a compact binary .dat file:
 *   - First 4 bytes: record count (int)
 *   - Remaining bytes: array of structs
 *
 * The entire active + soft-deleted record set is written on every save
 * so the in-memory index remains stable within a session.
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#include "file_io.h"

/* ------------------------------------------------------------------ */
/* Directory helpers                                                   */
/* ------------------------------------------------------------------ */

void ensure_data_dir(void)
{
#ifdef _WIN32
    mkdir(DATA_DIR);
#else
    mkdir(DATA_DIR, 0755);
#endif
}

/* ------------------------------------------------------------------ */
/* Generic helpers                                                     */
/* ------------------------------------------------------------------ */

static FILE *open_file(const char *path, const char *mode)
{
    FILE *fp = fopen(path, mode);
    return fp;
}

static int read_count(FILE *fp)
{
    int count = 0;
    if (fread(&count, sizeof(int), 1, fp) != 1)
        return 0;
    return count;
}

static int write_count(FILE *fp, int count)
{
    return (fwrite(&count, sizeof(int), 1, fp) == 1) ? 0 : -1;
}

/* ------------------------------------------------------------------ */
/* Raw Materials                                                       */
/* ------------------------------------------------------------------ */

int load_raw_materials(RawMaterial *materials, int max_count)
{
    FILE *fp = open_file(RAW_MATERIALS_FILE, "rb");
    if (!fp)
        return 0;

    int count = read_count(fp);
    if (count < 0 || count > max_count) {
        fprintf(stderr, "Warning: raw_materials.dat has invalid record count "
                        "(%d). File may be corrupt.\n", count);
        fclose(fp);
        return 0;
    }

    int loaded = (int)fread(materials, sizeof(RawMaterial), (size_t)count, fp);
    fclose(fp);
    return loaded;
}

int save_raw_materials(const RawMaterial *materials, int count)
{
    ensure_data_dir();
    FILE *fp = open_file(RAW_MATERIALS_FILE, "wb");
    if (!fp)
        return -1;

    if (write_count(fp, count) != 0 ||
        (count > 0 &&
         fwrite(materials, sizeof(RawMaterial), (size_t)count, fp)
             != (size_t)count)) {
        fprintf(stderr, "Error: failed to write raw_materials.dat\n");
        fclose(fp);
        return -1;
    }
    fclose(fp);
    return 0;
}

/* ------------------------------------------------------------------ */
/* Production Processes                                                */
/* ------------------------------------------------------------------ */

int load_production_processes(ProductionProcess *processes, int max_count)
{
    FILE *fp = open_file(PRODUCTION_FILE, "rb");
    if (!fp)
        return 0;

    int count = read_count(fp);
    if (count < 0 || count > max_count) {
        fprintf(stderr, "Warning: production_processes.dat has invalid record "
                        "count (%d). File may be corrupt.\n", count);
        fclose(fp);
        return 0;
    }

    int loaded = (int)fread(processes, sizeof(ProductionProcess),
                            (size_t)count, fp);
    fclose(fp);
    return loaded;
}

int save_production_processes(const ProductionProcess *processes, int count)
{
    ensure_data_dir();
    FILE *fp = open_file(PRODUCTION_FILE, "wb");
    if (!fp)
        return -1;

    if (write_count(fp, count) != 0 ||
        (count > 0 &&
         fwrite(processes, sizeof(ProductionProcess), (size_t)count, fp)
             != (size_t)count)) {
        fprintf(stderr, "Error: failed to write production_processes.dat\n");
        fclose(fp);
        return -1;
    }
    fclose(fp);
    return 0;
}

/* ------------------------------------------------------------------ */
/* Finished Products                                                   */
/* ------------------------------------------------------------------ */

int load_finished_products(FinishedProduct *products, int max_count)
{
    FILE *fp = open_file(FINISHED_PRODUCTS_FILE, "rb");
    if (!fp)
        return 0;

    int count = read_count(fp);
    if (count < 0 || count > max_count) {
        fprintf(stderr, "Warning: finished_products.dat has invalid record "
                        "count (%d). File may be corrupt.\n", count);
        fclose(fp);
        return 0;
    }

    int loaded = (int)fread(products, sizeof(FinishedProduct),
                            (size_t)count, fp);
    fclose(fp);
    return loaded;
}

int save_finished_products(const FinishedProduct *products, int count)
{
    ensure_data_dir();
    FILE *fp = open_file(FINISHED_PRODUCTS_FILE, "wb");
    if (!fp)
        return -1;

    if (write_count(fp, count) != 0 ||
        (count > 0 &&
         fwrite(products, sizeof(FinishedProduct), (size_t)count, fp)
             != (size_t)count)) {
        fprintf(stderr, "Error: failed to write finished_products.dat\n");
        fclose(fp);
        return -1;
    }
    fclose(fp);
    return 0;
}
