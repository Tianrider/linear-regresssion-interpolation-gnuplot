# Linear Regression and Interpolation Calculator with GNUPlot display

Program ini melakukan pencocokan kurva (regresi) dan interpolasi pada data dari file CSV.

## Fitur

-   Membaca data dari file CSV dengan header
-   Mendukung berbagai jenis regresi:
    -   Regresi Linear (y = mx + b)
    -   Regresi Polinomial (y = a₀ + a₁x + a₂x² + ... + aₙxⁿ)
    -   Regresi Logistik (y = c/(1 + ae^(-bx)))
-   Memilih kolom yang ingin diinterpolasi
-   Menampilkan plot data dengan GNUPlot
-   Menampilkan hasil interpolasi dari nilai x apapun
-   Menampilkan nilai R-squared untuk mengevaluasi kualitas regresi

## Persyaratan

-   Compiler C
-   GNUPlot untuk visualisasi
-   File CSV dengan header dan data numerik

## Format File CSV

File CSV harus memiliki format sebagai berikut:

```
column1,column2,column3,...
value1,value2,value3,...
value1,value2,value3,...
...
```

Contoh:

```
age,salary,experience
25,50000,2
30,65000,5
35,80000,8
40,95000,12
```

## Instalasi

### Windows

1. Install WSL (Windows Subsystem for Linux):
    ```powershell
    wsl --install
    ```
2. Di WSL, install dependencies:
    ```bash
    sudo apt update
    sudo apt install build-essential gcc gnuplot
    ```

### Linux (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install build-essential gcc gnuplot
```

### macOS

```bash
brew install gcc gnuplot
```

## Compile

```bash
gcc -o curve_fitting main.c -lm
```

## Usage

1. Run program:

    ```bash
    ./curve_fitting
    ```

2. Masukkan nama file CSV yang akan dianalisis

3. Program akan menampilkan daftar kolom yang tersedia dalam file CSV

4. Pilih kolom yang akan digunakan untuk:

    - Sumbu X (variabel independen)
    - Sumbu Y (variabel dependen)

5. Pilih jenis regresi yang diinginkan:

    - Linear: untuk data yang memiliki hubungan linear
    - Polinomial: untuk data yang memiliki pola kurva
    - Logistik: untuk data yang memiliki pola pertumbuhan terbatas

6. Program akan menampilkan:

    - Hasil analisis regresi
    - Persamaan kurva
    - Nilai R-squared
    - Plot data yang disimpan sebagai 'plot.png'

7. Untuk interpolasi:
    - Masukkan nilai X yang ingin diinterpolasi
    - Program akan menampilkan nilai Y yang diinterpolasi
    - Masukkan 'q' untuk keluar dari mode interpolasi

## Contoh Output

![picture 0](https://i.imgur.com/xdkIW1R.png)

## Catatan

-   Plot akan disimpan sebagai file PNG
-   Program akan menampilkan nama kolom yang dipilih dalam pesan interpolasi
-   Memory management otomatis untuk mencegah memory leak

## Author

Kelompok 8

-   [Christian Hadiwijaya](https://github.com/Tianrider)
-   [Benedict Aurelius](https://github.com/benedictaurel)
-   [Jonathan Kosasih](https://github.com/JonathanKosasih18)
