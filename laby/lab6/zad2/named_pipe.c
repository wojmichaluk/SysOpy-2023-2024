#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

int invalid_parameters(char* []);

int main(int argc, char* argv[]) {
    if (argc != 3 || invalid_parameters(argv)) {
        printf("Nieprawidlowy format uruchomienia programu. Prawidlowy format:\n");
        printf("`./named_pipe <poczatek przedzialu> <koniec przedzialu>`\n");
        printf("Przy czym koniec przedzialu musi byc wiekszy niz poczatek przedzialu\n\n");
        return 1;
    }

    double begin = strtod(argv[1], NULL);
    double end = strtod(argv[2], NULL);
    double result;
    char *name = "potok";

    mkfifo(name, 0666);
    int fd = open(name, O_WRONLY);

    write(fd, &begin, sizeof(double));
    write(fd, &end, sizeof(double));
    close(fd);

    fd = open(name, O_RDONLY);
    read(fd, &result, sizeof(double));
    close(fd);

    printf("Wynik: %lf\n", result);

    return 0;
}

int invalid_parameters(char* params[]) {
    char* str_end; //in case of "0.0" being valid parameter

    double begin = strtod(params[1], &str_end);
    if (*str_end != '\0') {
        return 1;
    }

    double end = strtod(params[2], &str_end);
    if (*str_end != '\0' || begin >= end) {
        return 1;
    }

    return 0;
}
