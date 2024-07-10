#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#define BEGIN 0.0
#define END 1.0
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

long double info[4];
long double results[2];

long double function(long double x) {
    return 4 / (x * x + 1);
}

int invalid_parameters(char* []);
long double calculate_integral(long double*);

int main(int argc, char* argv[]) {
    if (argc != 3 || invalid_parameters(argv)) {
        printf("Nieprawidlowy format uruchomienia programu. Prawidlowy format:\n");
        printf("`./unnamed_pipe <szerokosc przedzialu> <liczba procesow potomnych>`\n");
        printf("Przy czym szerokosc musi byc dodatnia i <= 1.0 / liczba procesow\n\n");
        return 1;
    }

    long double width = strtod(argv[1], NULL);
    int n = atoi(argv[2]);

    long double total = 0.0;
    int no_intervals = ceil((END - BEGIN) / width);

    pid_t child_pid;
    FILE *time_measure = fopen("pomiary.txt", "a");
    clock_t start, end;
    float time = 0.0f;

    start = clock();

    for (int i = 1; i <= n; i++) {
        int fd_to[2];
        int fd_from[2];

        pipe(fd_to);
        pipe(fd_from);

        child_pid = fork();

        if (child_pid != 0) {
            close(fd_to[0]);
            close(fd_from[1]);

            info[0] = BEGIN + (int)((i - 1) * no_intervals / n) * width;
            info[1] = MIN(BEGIN + (int)(i * no_intervals / n) * width, END);
            info[2] = (long double)((int)(i * no_intervals / n) - (int)((i - 1) * no_intervals / n));
            info[3] = width;

            write(fd_to[1], info, 4 * sizeof(long double));
            read(fd_from[0], results, 2 * sizeof(long double));

            total += results[0];
            time += (float)results[1];
        } else {
            close(fd_to[1]);
            close(fd_from[0]);

            read(fd_to[0], info, 4 * sizeof(long double));

            clock_t start_calc = clock();
            results[0] = calculate_integral(info);
            clock_t end_calc = clock();

            results[1] = (long double)(end_calc - start_calc) / CLOCKS_PER_SEC;

            write(fd_from[1], results, 2 * sizeof(long double));
            break;
        }
    }

    if (child_pid != 0) {
        end = clock();
        time += (float)(end - start) / CLOCKS_PER_SEC;

        fprintf(time_measure, "\t\t\t%d\t\t\t\t\t%.12Lf\t\t\t%f\n", n, width, time);

        printf("Wynik: %Lf\n\n", total);
        printf("Liczba procesów potomnych\t\tDokładność obliczeń\t\tCzas [s]\n");
        printf("\t\t%d\t\t\t%.12Lf\t\t\t%f\n\n", n, width, time);

        fclose(time_measure);
    }
    
    return 0;
}

int invalid_parameters(char* params[]) {
    int processes = atoi(params[2]);
    if (processes <= 0) {
        return 1;
    }

    long double width = strtod(params[1], NULL);
    if (width <= 0.0 || width > (END - BEGIN) / processes) {
        return 1;
    }

    return 0;
}

long double calculate_integral(long double* info) {
    long double a = info[0];
    long double b = info[1];
    int no_intervals = info[2];
    long double width = info[3];

    long double result = 0.0;

    for (int i = 0; i < no_intervals - 1; i++) {
        long double mid = a + (i + 0.5) * width;

        result += width * function(mid);
    }

    long double last_mid = (a + (no_intervals - 1) * width + b) / 2.0;
    long double last_width = b - a - (no_intervals - 1) * width;
    result += last_width * function(last_mid);

    return result;
}
