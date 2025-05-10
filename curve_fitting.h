#ifndef CURVE_FITTING_H
#define CURVE_FITTING_H

#include <float.h> // For DBL_MAX
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_COLUMNS 20
#define MAX_COLUMN_NAME 50
#define MAX_POLY_DEGREE 10
#define MAX_ITERATIONS 1000
#define LEARNING_RATE 0.01
#define TOLERANCE 1e-6

typedef struct {
    char name[MAX_COLUMN_NAME];
    int index;
} ColumnInfo;

typedef struct {
    double x;
    double y;
} DataPoint;

// Jenis regresi
typedef enum {
    REGRESSION_LINEAR,
    REGRESSION_POLYNOMIAL,
    REGRESSION_LOGISTIC
} RegressionType;

// Struct untuk menyimpan hasil regresi
typedef struct {
    RegressionType type;  // Jenis regresi
    double slope;         // Untuk regresi linear
    double intercept;     // Untuk regresi linear
    double a;             // Untuk regresi logistic (y = c/(1+a*e^(-bx)))
    double b;             // Untuk regresi logistic
    double c;             // Untuk regresi logistic (carrying capacity/upper limit)
    double *coefficients; // Untuk polynomial regression
    int degree;           // Derajat polynomial
    double r_squared;
} RegressionResult;

// Deklarasi fungsi
ColumnInfo *readCSVHeader(const char *filename, int *num_columns);
DataPoint *readCSVData(const char *filename, int x_column, int y_column, int *num_points);
RegressionResult linearRegression(DataPoint *data, int num_points);
RegressionResult polynomialRegression(DataPoint *data, int num_points, int degree);
RegressionResult logisticRegression(DataPoint *data, int num_points);
double interpolate(DataPoint *data, int num_points, double x);
void plotWithGNUPlot(DataPoint *data, int num_points, RegressionResult *reg_result);
void freeData(DataPoint *data);
void freeRegressionResult(RegressionResult *result);

// Function untuk membaca header CSV
ColumnInfo *readCSVHeader(const char *filename, int *num_columns) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Error membuka file %s\n", filename);
        return NULL;
    }

    char line[1024];
    if (!fgets(line, sizeof(line), file)) {
        printf("Error membaca header file\n");
        fclose(file);
        return NULL;
    }

    // Hitung jumlah kolom
    *num_columns = 0;
    char *token = strtok(line, ",");
    while (token) {
        (*num_columns)++;
        token = strtok(NULL, ",");
    }

    // Alokasi memori untuk informasi kolom
    ColumnInfo *columns = (ColumnInfo *)malloc(*num_columns * sizeof(ColumnInfo));
    if (!columns) {
        fclose(file);
        return NULL;
    }

    // Baca nama kolom
    rewind(file);
    fgets(line, sizeof(line), file);
    token = strtok(line, ",");
    int i = 0;
    while (token && i < *num_columns) {
        // Hapus whitespace dan newline
        char *end = token + strlen(token) - 1;
        while (end > token && (*end == '\n' || *end == '\r' || *end == ' ')) {
            *end = '\0';
            end--;
        }
        while (*token == ' ')
            token++;

        strncpy(columns[i].name, token, MAX_COLUMN_NAME - 1);
        columns[i].name[MAX_COLUMN_NAME - 1] = '\0';
        columns[i].index = i;

        token = strtok(NULL, ",");
        i++;
    }

    fclose(file);
    return columns;
}

// Function untuk membaca data CSV
DataPoint *readCSVData(const char *filename, int x_column, int y_column, int *num_points) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Error membuka file %s\n", filename);
        return NULL;
    }

    // Hitung jumlah baris
    char line[1024];
    *num_points = 0;
    while (fgets(line, sizeof(line), file)) {
        (*num_points)++;
    }
    (*num_points)--;

    // Alokasi memori
    DataPoint *data = (DataPoint *)malloc(*num_points * sizeof(DataPoint));
    if (!data) {
        fclose(file);
        return NULL;
    }

    // Reset pointer file ke awal
    rewind(file);

    // Lewati header
    fgets(line, sizeof(line), file);

    // Baca data
    int i = 0;
    while (fgets(line, sizeof(line), file) && i < *num_points) {
        char *token = strtok(line, ",");
        int col = 0;
        double x_val = 0, y_val = 0;

        while (token) {
            if (col == x_column) {
                x_val = atof(token);
            }
            if (col == y_column) {
                y_val = atof(token);
            }
            token = strtok(NULL, ",");
            col++;
        }

        data[i].x = x_val;
        data[i].y = y_val;
        i++;
    }

    fclose(file);
    return data;
}

RegressionResult linearRegression(DataPoint *data, int num_points) {
    RegressionResult result;
    result.type = REGRESSION_LINEAR;
    result.coefficients = NULL; // Inisialisasi ke NULL untuk membedakan dari regresi polynomial
    double sum_x = 0, sum_y = 0, sum_xy = 0, sum_x2 = 0;
    double mean_x, mean_y;

    // Hitung jumlah
    for (int i = 0; i < num_points; i++) {
        sum_x += data[i].x;
        sum_y += data[i].y;
        sum_xy += data[i].x * data[i].y;
        sum_x2 += data[i].x * data[i].x;
    }

    mean_x = sum_x / num_points;
    mean_y = sum_y / num_points;

    // Hitung slope dan intercept
    result.slope = (num_points * sum_xy - sum_x * sum_y) / (num_points * sum_x2 - sum_x * sum_x);
    result.intercept = mean_y - result.slope * mean_x;

    // Hitung R-squared
    double ss_tot = 0, ss_res = 0;
    for (int i = 0; i < num_points; i++) {
        double y_pred = result.slope * data[i].x + result.intercept;
        ss_tot += pow(data[i].y - mean_y, 2);
        ss_res += pow(data[i].y - y_pred, 2);
    }
    result.r_squared = 1 - (ss_res / ss_tot);

    return result;
}

RegressionResult polynomialRegression(DataPoint *data, int num_points, int degree) {
    RegressionResult result;
    result.type = REGRESSION_POLYNOMIAL;
    result.degree = degree;
    result.coefficients = (double *)malloc((degree + 1) * sizeof(double));

    // Buat matriks untuk sistem persamaan
    double **matrix = (double **)malloc((degree + 1) * sizeof(double *));
    for (int i = 0; i <= degree; i++) {
        matrix[i] = (double *)calloc(degree + 2, sizeof(double));
    }

    // Isi matriks dengan data
    for (int i = 0; i <= degree; i++) {
        for (int j = 0; j <= degree; j++) {
            for (int k = 0; k < num_points; k++) {
                matrix[i][j] += pow(data[k].x, i + j);
            }
        }
        for (int k = 0; k < num_points; k++) {
            matrix[i][degree + 1] += data[k].y * pow(data[k].x, i);
        }
    }

    // Selesaikan sistem persamaan dengan eliminasi Gauss
    for (int i = 0; i <= degree; i++) {
        double pivot = matrix[i][i];
        for (int j = i; j <= degree + 1; j++) {
            matrix[i][j] /= pivot;
        }
        for (int k = 0; k <= degree; k++) {
            if (k != i) {
                double factor = matrix[k][i];
                for (int j = i; j <= degree + 1; j++) {
                    matrix[k][j] -= factor * matrix[i][j];
                }
            }
        }
    }

    // Ambil koefisien
    for (int i = 0; i <= degree; i++) {
        result.coefficients[i] = matrix[i][degree + 1];
    }

    // Hitung R-squared
    double mean_y = 0;
    for (int i = 0; i < num_points; i++) {
        mean_y += data[i].y;
    }
    mean_y /= num_points;

    double ss_tot = 0, ss_res = 0;
    for (int i = 0; i < num_points; i++) {
        double y_pred = 0;
        for (int j = 0; j <= degree; j++) {
            y_pred += result.coefficients[j] * pow(data[i].x, j);
        }
        ss_tot += pow(data[i].y - mean_y, 2);
        ss_res += pow(data[i].y - y_pred, 2);
    }
    result.r_squared = 1 - (ss_res / ss_tot);

    // Bersihkan memori
    for (int i = 0; i <= degree; i++) {
        free(matrix[i]);
    }
    free(matrix);

    return result;
}

double interpolate(DataPoint *data, int num_points, double x) {
    // Cari dua titik yang membatasi x
    int i;
    for (i = 0; i < num_points - 1; i++) {
        if (data[i].x <= x && data[i + 1].x >= x) {
            break;
        }
    }

    // Interpolasi linear
    double x0 = data[i].x;
    double x1 = data[i + 1].x;
    double y0 = data[i].y;
    double y1 = data[i + 1].y;

    return y0 + (y1 - y0) * (x - x0) / (x1 - x0);
}

void plotWithGNUPlot(DataPoint *data, int num_points, RegressionResult *reg_result) {
    FILE *gnuplot = popen("gnuplot", "w");
    if (!gnuplot) {
        printf("Error: Tidak dapat membuka GNUPlot. Pastikan GNUPlot terinstal.\n");
        return;
    }

    // Siapkan file data temporari
    FILE *temp = fopen("tempdata.dat", "w");
    if (!temp) {
        printf("Error: Tidak dapat membuat file data temporari.\n");
        pclose(gnuplot);
        return;
    }

    // Tulis data ke file temporer
    for (int i = 0; i < num_points; i++) {
        fprintf(temp, "%f %f\n", data[i].x, data[i].y);
    }
    fclose(temp);

    // Buat file model untuk evaluasi kurva
    FILE *model_data = fopen("modeldata.dat", "w");
    if (!model_data) {
        printf("Error: Tidak dapat membuat file model data.\n");
        pclose(gnuplot);
        remove("tempdata.dat");
        return;
    }

    // Tentukan range data
    double min_x = data[0].x, max_x = data[0].x;
    for (int i = 1; i < num_points; i++) {
        if (data[i].x < min_x)
            min_x = data[i].x;
        if (data[i].x > max_x)
            max_x = data[i].x;
    }

    // Buat data model dengan lebih banyak titik untuk kurva
    double step = (max_x - min_x) / 100.0;

    // mean_x untuk regresi logistic
    double mean_x = 0;
    if (reg_result->type == REGRESSION_LOGISTIC) {
        for (int i = 0; i < num_points; i++) {
            mean_x += data[i].x;
        }
        mean_x /= num_points;
    }

    for (double x = min_x; x <= max_x; x += step) {
        double y = 0;
        switch (reg_result->type) {
        case REGRESSION_LINEAR:
            y = reg_result->slope * x + reg_result->intercept;
            break;
        case REGRESSION_POLYNOMIAL:
            for (int i = 0; i <= reg_result->degree; i++) {
                y += reg_result->coefficients[i] * pow(x, i);
            }
            break;
        case REGRESSION_LOGISTIC:
            y = reg_result->c / (1 + reg_result->a * exp(-reg_result->b * (x - mean_x)));
            break;
        }
        fprintf(model_data, "%f %f\n", x, y);
    }
    fclose(model_data);

    // Konfigurasi plot untuk output PNG
    fprintf(gnuplot, "set terminal png size 800,600 enhanced font 'Arial,12'\n");
    fprintf(gnuplot, "set output 'plot.png'\n");
    fprintf(gnuplot, "set title 'Data Points dan Kurva Regresi'\n");
    fprintf(gnuplot, "set xlabel 'X'\n");
    fprintf(gnuplot, "set ylabel 'Y'\n");
    fprintf(gnuplot, "set grid\n");
    fprintf(gnuplot, "set key left top\n");

    // Debug info
    if (reg_result->type == REGRESSION_LOGISTIC) {
        printf("Debug: Plotting logistic y = %.6f / (1 + %.6f * e^(-%.6f * (x - %.6f)))",
               reg_result->c, reg_result->a, reg_result->b, mean_x);
    }

    // Plot data points dan kurva regresi
    fprintf(gnuplot, "plot 'tempdata.dat' using 1:2 title 'Data Points' with points pointtype 7 pointsize 1.5, ");
    fprintf(gnuplot, "'modeldata.dat' using 1:2 title '");

    switch (reg_result->type) {
    case REGRESSION_LINEAR:
        fprintf(gnuplot, "Regresi Linear y = %.4fx + %.4f", reg_result->slope, reg_result->intercept);
        break;
    case REGRESSION_POLYNOMIAL:
        fprintf(gnuplot, "Regresi Polynomial");
        break;
    case REGRESSION_LOGISTIC:
        fprintf(gnuplot, "Regresi Logistic y = %.4f / (1 + %.4f * e^(-%.4f * (x - %.4f)))",
                reg_result->c, reg_result->a, reg_result->b, mean_x);
        break;
    }

    fprintf(gnuplot, "' with lines linewidth 2\n");

    // Tambahkan label R-squared
    fprintf(gnuplot, "set label 'R² = %.4lf' at graph 0.02, 0.95 font 'Arial,10'\n", reg_result->r_squared);
    fprintf(gnuplot, "replot\n");

    fflush(gnuplot);
    pclose(gnuplot);

    printf("Plot telah disimpan ke file 'plot.png'\n");

    // Hapus temp file
    remove("tempdata.dat");
    remove("modeldata.dat");
}

void freeData(DataPoint *data) {
    free(data);
}

void freeRegressionResult(RegressionResult *result) {
    if (result->coefficients != NULL) {
        free(result->coefficients);
        result->coefficients = NULL;
    }
}

// Fungsi sigmoid untuk logistic regression
double sigmoid(double z) {
    if (z < -20.0)
        return 0.00000001; // Avoid underflow
    if (z > 20.0)
        return 0.99999999; // Avoid overflow
    return 1.0 / (1.0 + exp(-z));
}

// Function untuk regresi logistic
// Referensi: https://math.libretexts.org/Workbench/1250_Draft_3/06%3A_Exponential_and_Logarithmic_Functions/6.09%3A_Exponential_and_Logarithmic_Regressions
RegressionResult logisticRegression(DataPoint *data, int num_points) {
    RegressionResult result;
    result.type = REGRESSION_LOGISTIC;
    result.coefficients = NULL;

    // Mencari nilai maksimum y untuk estimasi kapasitas
    double max_y = 0;
    for (int i = 0; i < num_points; i++) {
        if (data[i].y > max_y) {
            max_y = data[i].y;
        }
    }

    // Inisialisasi parameter
    double a = 1.0;         // Initial a value
    double b = 0.1;         // Initial b value (growth rate)
    double c = max_y * 1.1; // Initial c value (carrying capacity)

    printf("Nilai awal: a=%.6f, b=%.6f, c=%.6f\n", a, b, c);

    // Normalisasi data x untuk stabilitas numerik
    double sum_x = 0;
    for (int i = 0; i < num_points; i++) {
        sum_x += data[i].x;
    }
    double mean_x = sum_x / num_points;

    double sum_squared_x = 0;
    for (int i = 0; i < num_points; i++) {
        sum_squared_x += (data[i].x - mean_x) * (data[i].x - mean_x);
    }
    double std_x = sqrt(sum_squared_x / num_points);

    // Alokasi memori untuk data yang dinormalisasi
    DataPoint *normalized_data = (DataPoint *)malloc(num_points * sizeof(DataPoint));
    for (int i = 0; i < num_points; i++) {
        normalized_data[i].x = (data[i].x - mean_x) / std_x;
        normalized_data[i].y = data[i].y;
    }

    // Gradient descent untuk menemukan parameter
    double cost, prev_cost = DBL_MAX;
    double learning_rate = LEARNING_RATE;

    // Mulai iterasi
    for (int iter = 0; iter < MAX_ITERATIONS; iter++) {
        // Reset gradients
        double grad_a = 0, grad_b = 0, grad_c = 0;
        cost = 0;

        // Hitung gradients untuk semua poin data
        for (int i = 0; i < num_points; i++) {
            double x = normalized_data[i].x;
            double y = normalized_data[i].y;

            // Prediksi fungsi logistik: y_pred = c / (1 + a * exp(-b * x))
            double y_pred = c / (1 + a * exp(-b * x));

            // Hitung error
            double error = y_pred - y;

            // Hitung gradients
            double exp_term = exp(-b * x);
            double denominator = 1 + a * exp_term;
            double denominator_squared = denominator * denominator;

            grad_a += error * (c * exp_term / denominator_squared);
            grad_b += error * (c * a * x * exp_term / denominator_squared);
            grad_c += error / denominator;

            // Hitung cost (mean squared error)
            cost += error * error;
        }

        grad_a /= num_points;
        grad_b /= num_points;
        grad_c /= num_points;
        cost /= num_points;

        a -= learning_rate * grad_a;
        b -= learning_rate * grad_b;
        c -= learning_rate * grad_c;

        // Batasi nilai parameter
        if (a <= 0)
            a = 0.01;
        if (c <= 0)
            c = 0.01;

        // Print status
        if (iter % 100 == 0 || iter == MAX_ITERATIONS - 1) {
            printf("Iterasi %d: a=%.6f, b=%.6f, c=%.6f, cost=%.6f\n",
                   iter, a, b, c, cost);
        }

        // Check for convergence
        if (fabs(prev_cost - cost) < TOLERANCE) {
            printf("Konvergen pada iterasi %d\n", iter);
            break;
        }

        prev_cost = cost;
    }

    result.a = a;
    result.b = b / std_x; // Sesuaikan b untuk normalisasi
    result.c = c;

    // Hitung R-squared
    double mean_y = 0;
    for (int i = 0; i < num_points; i++) {
        mean_y += data[i].y;
    }
    mean_y /= num_points;

    double ss_tot = 0, ss_res = 0;
    for (int i = 0; i < num_points; i++) {
        double x = data[i].x;
        double y = data[i].y;
        double y_pred = result.c / (1 + result.a * exp(-result.b * (x - mean_x)));

        ss_tot += (y - mean_y) * (y - mean_y);
        ss_res += (y - y_pred) * (y - y_pred);
    }

    result.r_squared = 1 - (ss_res / ss_tot);

    free(normalized_data);

    // Contoh prediksi untuk verifikasi
    printf("\nContoh prediksi:\n");
    for (int i = 0; i < num_points; i += num_points / 5) {
        double x = data[i].x;
        double y_actual = data[i].y;
        double y_pred = result.c / (1 + result.a * exp(-result.b * (x - mean_x)));
        printf("x=%.2f: y_aktual=%.4f, y_prediksi=%.4f\n", x, y_actual, y_pred);
    }

    printf("\nHasil regresi logistik:\n");
    printf("a = %.6f\n", result.a);
    printf("b = %.6f\n", result.b);
    printf("c = %.6f (carrying capacity)\n", result.c);
    printf("R-squared = %.6f\n", result.r_squared);

    return result;
}

#endif