#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define TRY 100
#define MAX_POINTS 1000 // максимум точек
#define MAX_K 10        // максимум кластеров
#define MAX_ITER 100     // максимум итераций

typedef struct {
    double x, y; // координаты точки
} dot;

dot data[MAX_POINTS];     // массив точек
dot centers[MAX_K];       // массив центров кластеров
int marks[MAX_POINTS];     // к какому кластеру принадлежит каждая точка
int data_count = 0;         // сколько точек загружено

// квадрат расстояния (для WCSS)
double distance(dot a, dot b) {
    double dx = a.x - b.x;
    double dy = a.y - b.y;
    return dx * dx + dy * dy;
}

// случайные центры кластеров
void init(int k) {
    for (int i = 0; i < k; i++) {
        centers[i] = data[rand() % data_count]; // выбираем случайную точку как центр
    }
}

void assign(int k) {
    for (int i = 0; i < data_count; i++) {
        double min_dist = distance(data[i], centers[0]); // расстояние до первого центра
        int label = 0; // ближайший центр

        for (int j = 1; j < k; j++) {
            double dist = distance(data[i], centers[j]); // расстояние до остальных центров
            if (dist < min_dist) {
                min_dist = dist;
                label = j; // более близкий центр
            }
        }
        marks[i] = label; // сохраняем ближайший кластер
    }
}

// Пересчёт центров кластеров как среднее своих точек
void update(int k) {
    dot new_centers[MAX_K] = {0}; // временные суммы координат
    int counts[MAX_K] = {0};      // количество точек в каждом кластере
    double epsilon = 1e-1;        // величина случайного смещения

    // Суммируем координаты всех точек по их кластеру
    for (int i = 0; i < data_count; i++) {
        new_centers[marks[i]].x += data[i].x;
        new_centers[marks[i]].y += data[i].y;
        counts[marks[i]]++;
    }

    // Делим сумму на количество — получаем среднее (новый центр) + случайное смещение
    for (int j = 0; j < k; j++) {
        if (counts[j] > 0) {
            centers[j].x = new_centers[j].x / counts[j];
            centers[j].y = new_centers[j].y / counts[j];

            // Добавляем случайное смещение к координатам центра
            double dx = ((double)rand() / RAND_MAX * 2 - 1) * epsilon;
            double dy = ((double)rand() / RAND_MAX * 2 - 1) * epsilon;
            centers[j].x += dx;
            centers[j].y += dy;
        }
    }
}

// сумма квадратов расстояний (WCSS)
double wcss(int k) {
    double sum = 0;
    for (int i = 0; i < data_count; i++) {
        sum += distance(data[i], centers[marks[i]]); // расстояние до центра кластера
    }
    return sum;
}

void kmeans(int k) {
    init(k);
    for (int iter = 0; iter < MAX_ITER; iter++) {
        assign(k);
        update(k);
    }
}

// находим точку локтя — максимальное расстояние от кривой до прямой
int find_elbow(const double *wcss, int max_k) {
    int elbow = 1;
    double max_dist = -1;

    double x1 = 1, y1 = wcss[1];
    double x2 = max_k, y2 = wcss[max_k];
    double dx = x2 - x1;
    double dy = y2 - y1;

    for (int k = 2; k < max_k; k++) {
        double x0 = k;
        double y0 = wcss[k];
        double px = x0 - x1;
        double py = y0 - y1;

        double area = fabs(px * dy - py * dx);
        double base_len = sqrt(dx * dx + dy * dy);
        double dist = area / base_len;

        if (dist > max_dist) {
            max_dist = dist;
            elbow = k;
        }
    }
    return elbow;
}

int main() {
    srand(time(NULL)); // ГРЧ

    freopen("data3.txt", "r", stdin); // ДАТАСЕТ
    while (scanf("%lf %lf", &data[data_count].x, &data[data_count].y) == 2) {
        data_count++;
        if (data_count >= MAX_POINTS) break;
    }
    fclose(stdin);

    double arr[MAX_K + 1]; // массив значений WCSS

    for (int k = 1; k <= MAX_K; k++) {
        double best_wcss = INFINITY;
        dot best_centers[MAX_K];

        for (int r = 0; r < TRY; r++) {
            kmeans(k);
            double current = wcss(k);
            if (current < best_wcss) {
                best_wcss = current;

                // сохраняем лучшие центры
                for (int i = 0; i < k; i++) {
                    best_centers[i] = centers[i];
                }
            }
        }

        // восстановить лучшие центры перед экспортом (или для визуализации)
        for (int i = 0; i < k; i++) {
            centers[i] = best_centers[i];
        }

        arr[k] = best_wcss;
    }

    int elbow_k = find_elbow(arr, MAX_K); // локоть
    printf("Elbow found at k = %d\n", elbow_k);

    // сохраняем WCSS в CSV для HTML
    FILE *f = fopen("wcss.csv", "w");
    if (f) {
        fprintf(f, "#elbow,%d\n", elbow_k); // добавляем строку локтя для графика
        for (int k = 1; k <= MAX_K; k++) {
            fprintf(f, "%d,%.2f\n", k, arr[k]);
        }
        fclose(f);
    }
}
