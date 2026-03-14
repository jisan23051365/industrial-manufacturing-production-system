#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

/* =========================================================
   INDUSTRIAL MANUFACTURING & PRODUCTION CONTROL SYSTEM
   ========================================================= */

typedef struct {
    int id;
    char username[30];
    unsigned long password_hash;
} User;

typedef struct {
    int raw_id;
    char name[50];
    int quantity;
    float cost_per_unit;
    int reorder_level;
} RawMaterial;

typedef struct {
    int product_id;
    char name[50];
    int quantity;
    float production_cost;
} FinishedProduct;

typedef struct {
    int production_id;
    int raw_id;
    int raw_used;
    int product_id;
    int produced_qty;
    float total_cost;
    char timestamp[30];
} ProductionRecord;

/* ===================== UTILITIES ===================== */

unsigned long hashPassword(const char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;
    return hash;
}

void logActivity(const char *msg) {
    FILE *fp = fopen("manufacturing_logs.txt", "a");
    if (!fp) return;
    time_t now = time(NULL);
    fprintf(fp, "%s - %s\n", ctime(&now), msg);
    fprintf(fp, "--------------------------------\n");
    fclose(fp);
}

/* ===================== AUTH ===================== */

void initializeAdmin() {
    FILE *fp = fopen("manufacturing_users.dat", "rb");
    if (fp) { fclose(fp); return; }

    fp = fopen("manufacturing_users.dat", "wb");
    User admin = {1, "admin", hashPassword("admin123")};
    fwrite(&admin, sizeof(User), 1, fp);
    fclose(fp);
}

int login() {
    char username[30], password[30];
    unsigned long hash;
    User u;

    printf("Username: ");
    scanf("%s", username);
    printf("Password: ");
    scanf("%s", password);

    hash = hashPassword(password);

    FILE *fp = fopen("manufacturing_users.dat", "rb");
    if (!fp) return 0;

    while (fread(&u, sizeof(User), 1, fp)) {
        if (strcmp(u.username, username) == 0 && u.password_hash == hash) {
            fclose(fp);
            logActivity("Admin login successful.");
            return 1;
        }
    }

    fclose(fp);
    printf("Invalid credentials.\n");
    return 0;
}

/* ===================== RAW MATERIAL ===================== */

void addRawMaterial() {
    FILE *fp = fopen("raw_materials.dat", "ab");
    if (!fp) return;

    RawMaterial r;
    printf("Raw Material ID: "); scanf("%d", &r.raw_id);
    printf("Name: "); scanf(" %[^\n]", r.name);
    printf("Quantity: "); scanf("%d", &r.quantity);
    printf("Cost Per Unit: "); scanf("%f", &r.cost_per_unit);
    printf("Reorder Level: "); scanf("%d", &r.reorder_level);

    fwrite(&r, sizeof(RawMaterial), 1, fp);
    fclose(fp);
    logActivity("Raw material added.");
}

void viewRawMaterials() {
    FILE *fp = fopen("raw_materials.dat", "rb");
    if (!fp) return;

    RawMaterial r;
    printf("\n--- Raw Materials ---\n");
    while (fread(&r, sizeof(RawMaterial), 1, fp)) {
        printf("ID:%d | %s | Qty:%d | Cost:%.2f",
               r.raw_id, r.name, r.quantity, r.cost_per_unit);
        if (r.quantity <= r.reorder_level)
            printf("  **REORDER REQUIRED**");
        printf("\n");
    }
    fclose(fp);
}

/* ===================== FINISHED PRODUCT ===================== */

void addFinishedProduct() {
    FILE *fp = fopen("finished_products.dat", "ab");
    if (!fp) return;

    FinishedProduct p;
    printf("Product ID: "); scanf("%d", &p.product_id);
    printf("Name: "); scanf(" %[^\n]", p.name);
    printf("Quantity: "); scanf("%d", &p.quantity);
    printf("Production Cost Per Unit: "); scanf("%f", &p.production_cost);

    fwrite(&p, sizeof(FinishedProduct), 1, fp);
    fclose(fp);
    logActivity("Finished product added.");
}

/* ===================== PRODUCTION PROCESS ===================== */

void produce() {
    FILE *rfp = fopen("raw_materials.dat", "rb+");
    FILE *pfp = fopen("finished_products.dat", "rb+");
    FILE *prfp = fopen("production_records.dat", "ab");

    if (!rfp || !pfp || !prfp) return;

    int raw_id, product_id, raw_used, produced_qty;
    RawMaterial r;
    FinishedProduct p;

    printf("Raw Material ID: "); scanf("%d", &raw_id);
    printf("Raw Quantity Used: "); scanf("%d", &raw_used);
    printf("Product ID: "); scanf("%d", &product_id);
    printf("Produced Quantity: "); scanf("%d", &produced_qty);

    while (fread(&r, sizeof(RawMaterial), 1, rfp)) {
        if (r.raw_id == raw_id && r.quantity >= raw_used) {
            r.quantity -= raw_used;
            fseek(rfp, -sizeof(RawMaterial), SEEK_CUR);
            fwrite(&r, sizeof(RawMaterial), 1, rfp);
            break;
        }
    }

    while (fread(&p, sizeof(FinishedProduct), 1, pfp)) {
        if (p.product_id == product_id) {
            p.quantity += produced_qty;
            fseek(pfp, -sizeof(FinishedProduct), SEEK_CUR);
            fwrite(&p, sizeof(FinishedProduct), 1, pfp);
            break;
        }
    }

    ProductionRecord pr;
    pr.production_id = rand() % 100000;
    pr.raw_id = raw_id;
    pr.raw_used = raw_used;
    pr.product_id = product_id;
    pr.produced_qty = produced_qty;
    pr.total_cost = raw_used * r.cost_per_unit;
    strcpy(pr.timestamp, ctime(&(time_t){time(NULL)}));

    fwrite(&pr, sizeof(ProductionRecord), 1, prfp);

    printf("Production completed. Total Cost: %.2f\n", pr.total_cost);
    logActivity("Production process executed.");

    fclose(rfp);
    fclose(pfp);
    fclose(prfp);
}

/* ===================== ANALYTICS ===================== */

void productionAnalytics() {
    FILE *fp = fopen("production_records.dat", "rb");
    if (!fp) return;

    ProductionRecord pr;
    float arr[500];
    int n = 0;

    while (fread(&pr, sizeof(ProductionRecord), 1, fp) && n < 500) {
        arr[n++] = pr.total_cost;
    }
    fclose(fp);

    if (n == 0) {
        printf("No production data.\n");
        return;
    }

    float sum = 0;
    for (int i = 0; i < n; i++) sum += arr[i];
    float mean = sum / n;

    float variance = 0;
    for (int i = 0; i < n; i++)
        variance += pow(arr[i] - mean, 2);
    variance /= n;

    float std = sqrt(variance);

    printf("\n--- Production Cost Analytics ---\n");
    printf("Total Records: %d\n", n);
    printf("Average Cost: %.2f\n", mean);
    printf("Variance: %.2f\n", variance);
    printf("Standard Deviation: %.2f\n", std);

    logActivity("Production analytics generated.");
}

/* ===================== MAIN ===================== */

int main() {
    initializeAdmin();

    printf("=== MANUFACTURING SYSTEM ===\n");

    if (!login()) return 0;

    int choice;

    while (1) {
        printf("\n1.Add Raw Material\n");
        printf("2.View Raw Materials\n");
        printf("3.Add Finished Product\n");
        printf("4.Produce\n");
        printf("5.Production Analytics\n");
        printf("6.Exit\n");
        printf("Choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: addRawMaterial(); break;
            case 2: viewRawMaterials(); break;
            case 3: addFinishedProduct(); break;
            case 4: produce(); break;
            case 5: productionAnalytics(); break;
            case 6:
                logActivity("System exited.");
                exit(0);
            default:
                printf("Invalid choice.\n");
        }
    }

    return 0;
}