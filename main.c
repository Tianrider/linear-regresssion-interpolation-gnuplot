#include "curve_fitting.h"
#include <math.h>

int main() {
    char filename[256];
    printf("Masukkan nama file CSV: ");
    scanf("%255s", filename);

    // Baca header untuk mendapatkan informasi kolom
    int num_columns;
    ColumnInfo *columns = readCSVHeader(filename, &num_columns);
    if (!columns) {
        printf("Error membaca header file\n");
        return 1;
    }

    // Tampilkan pilihan kolom
    printf("\nKolom yang tersedia:\n");
    for (int i = 0; i < num_columns; i++) {
        printf("%d. %s\n", i + 1, columns[i].name);
    }

    // Pilih kolom untuk sumbu X
    int x_choice;
    do {
        printf("\nPilih kolom untuk sumbu X (1-%d): ", num_columns);
        scanf("%d", &x_choice);
    } while (x_choice < 1 || x_choice > num_columns);

    // Pilih kolom untuk sumbu Y
    int y_choice;
    do {
        printf("Pilih kolom untuk sumbu Y (1-%d): ", num_columns);
        scanf("%d", &y_choice);
    } while (y_choice < 1 || y_choice > num_columns);

    // Konversi pilihan ke indeks (0-based)
    int x_column = x_choice - 1;
    int y_column = y_choice - 1;

    // Baca data dari file
    int num_points;
    DataPoint *data = readCSVData(filename, x_column, y_column, &num_points);
    if (!data) {
        printf("Error membaca data\n");
        free(columns);
        return 1;
    }

    // Lakukan regresi linear
    RegressionResult result = linearRegression(data, num_points);

    // Tampilkan hasil
    printf("\nHasil Analisis Regresi:\n");
    printf("Persamaan: y = %.2fx + %.2f\n", result.slope, result.intercept);
    printf("R-squared: %.4f\n", result.r_squared);

    // Buat plot
    plotWithGNUPlot(data, num_points, &result);

    // Interpolasi
    char choice;
    do {
        double x;
        printf("\nMasukkan nilai %s untuk interpolasi (atau 'q' untuk keluar): ", columns[x_column].name);
        if (scanf(" %lf", &x) == 1) {
            double y = interpolate(data, num_points, x);
            printf("Nilai %s yang diinterpolasi: %.2f\n", columns[y_column].name, y);
        } else {
            scanf(" %c", &choice);
            if (choice == 'q' || choice == 'Q')
                break;
        }
    } while (1);

    // Bersihkan memori
    freeData(data);
    free(columns);

    return 0;
}