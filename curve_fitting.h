#ifndef CURVE_FITTING_H
#define CURVE_FITTING_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_COLUMNS 20
#define MAX_COLUMN_NAME 50

// Struktur untuk menyimpan nama kolom
typedef struct {
    char name[MAX_COLUMN_NAME];
    int index;
} ColumnInfo;

// Struktur untuk menyimpan titik data
typedef struct {
    double x;
    double y;
} DataPoint;

// Struktur untuk menyimpan hasil regresi
typedef struct {
    double slope;
    double intercept;
    double r_squared;
} RegressionResult;

// Deklarasi fungsi
ColumnInfo *readCSVHeader(const char *filename, int *num_columns);
DataPoint *readCSVData(const char *filename, int x_column, int y_column, int *num_points);
RegressionResult linearRegression(DataPoint *data, int num_points);
double interpolate(DataPoint *data, int num_points, double x);
void plotData(DataPoint *data, int num_points, RegressionResult *reg_result);
void plotWithGNUPlot(DataPoint *data, int num_points, RegressionResult *reg_result);
void freeData(DataPoint *data);

// Fungsi untuk membaca header CSV
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

// Fungsi untuk membaca data CSV
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
    (*num_points)--; // Kurangi baris header

    // Alokasi memori untuk titik data
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

void plotData(DataPoint *data, int num_points, RegressionResult *reg_result) {
    // Plot ASCII sederhana
    printf("\nPlot Data:\n");
    printf("Y\n");

    // Cari nilai minimum dan maksimum
    double min_x = data[0].x, max_x = data[0].x;
    double min_y = data[0].y, max_y = data[0].y;

    for (int i = 1; i < num_points; i++) {
        if (data[i].x < min_x)
            min_x = data[i].x;
        if (data[i].x > max_x)
            max_x = data[i].x;
        if (data[i].y < min_y)
            min_y = data[i].y;
        if (data[i].y > max_y)
            max_y = data[i].y;
    }

    // Tampilkan persamaan garis regresi
    printf("Garis regresi: y = %.2fx + %.2f (R² = %.4f)\n",
           reg_result->slope, reg_result->intercept, reg_result->r_squared);

    // Tampilkan titik data
    printf("\nTitik Data:\n");
    for (int i = 0; i < num_points; i++) {
        printf("Titik %d: (%.2f, %.2f)\n", i + 1, data[i].x, data[i].y);
    }
}

void plotWithGNUPlot(DataPoint *data, int num_points, RegressionResult *reg_result) {
    FILE *gnuplot = popen("gnuplot", "w");
    if (!gnuplot) {
        printf("Error: Tidak dapat membuka GNUPlot. Pastikan GNUPlot terinstal.\n");
        return;
    }

    // Konfigurasi plot untuk output PNG
    fprintf(gnuplot, "set terminal png\n");
    fprintf(gnuplot, "set output 'plot.png'\n");
    fprintf(gnuplot, "set title 'Data Points dan Garis Regresi'\n");
    fprintf(gnuplot, "set xlabel 'X'\n");
    fprintf(gnuplot, "set ylabel 'Y'\n");
    fprintf(gnuplot, "set grid\n");
    fprintf(gnuplot, "set key left top\n");

    // Plot data points dan regression
    fprintf(gnuplot, "plot '-' using 1:2 title 'Data Points' with points pointtype 7 pointsize 1.5, ");
    fprintf(gnuplot, "%f * x + %f title 'Garis Regresi' with lines linewidth 2\n",
            reg_result->slope, reg_result->intercept);

    // loop setiap data point
    for (int i = 0; i < num_points; i++) {
        fprintf(gnuplot, "%f %f\n", data[i].x, data[i].y);
    }
    fprintf(gnuplot, "e\n");

    fflush(gnuplot);
    pclose(gnuplot);

    printf("Plot telah disimpan ke file 'plot.png'\n");
}

void freeData(DataPoint *data) {
    free(data);
}

#endif