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

    // Pilih jenis regresi
    int regression_type;
    printf("\nPilih jenis regresi:\n");
    printf("1. Linear\n");
    printf("2. Polynomial\n");
    printf("3. Logistic\n");
    printf("Pilihan (1-3): ");
    scanf("%d", &regression_type);

    RegressionResult result;
    int degree = 1; // Default degree

    if (regression_type == 1) {
        result = linearRegression(data, num_points);

        printf("\nHasil Analisis Regresi Linear:\n");
        printf("Persamaan: y = %.4fx + %.4f\n", result.slope, result.intercept);
        printf("R-squared: %.4f\n", result.r_squared);
    } else if (regression_type == 2) {
        do {
            printf("\nMasukkan derajat polynomial (1-%d): ", MAX_POLY_DEGREE);
            scanf("%d", &degree);
        } while (degree < 1 || degree > MAX_POLY_DEGREE);

        result = polynomialRegression(data, num_points, degree);

        printf("\nHasil Analisis Regresi Polynomial:\n");
        printf("Persamaan: y = ");
        for (int i = 0; i <= degree; i++) {
            if (i == 0) {
                printf("%.4f", result.coefficients[i]);
            } else {
                printf(" + %.4fx^%d", result.coefficients[i], i);
            }
        }
        printf("\nR-squared: %.4f\n", result.r_squared);
    } else {
        result = logisticRegression(data, num_points);

        printf("\nHasil Analisis Regresi Logistic:\n");
        printf("Persamaan: y = %.4f / (1 + %.4f * e^(-%.4f * x))\n",
               result.c, result.a, result.b);
        printf("R-squared: %.4f\n", result.r_squared);
    }

    // Buat plot
    plotWithGNUPlot(data, num_points, &result);

    // Interpolasi
    char choice;
    do {
        double x;
        printf("\nMasukkan nilai %s untuk interpolasi (atau 'q' untuk keluar): ", columns[x_column].name);
        if (scanf(" %lf", &x) == 1) {
            double y;
            double mean_x = 0;

            switch (regression_type) {
            case 1: // Linear
                y = result.slope * x + result.intercept;
                break;
            case 2: // Polynomial
                y = 0;
                for (int i = 0; i <= degree; i++) {
                    y += result.coefficients[i] * pow(x, i);
                }
                break;
            case 3: // Logistic
                for (int i = 0; i < num_points; i++) {
                    mean_x += data[i].x;
                }
                mean_x /= num_points;
                y = result.c / (1 + result.a * exp(-result.b * (x - mean_x)));
                break;
            }
            printf("Nilai %s yang diinterpolasi: %.2f\n", columns[y_column].name, y);
        } else {
            scanf(" %c", &choice);
            if (choice == 'q' || choice == 'Q')
                break;
        }
    } while (1);

    freeData(data);
    free(columns);
    freeRegressionResult(&result);

    return 0;
}