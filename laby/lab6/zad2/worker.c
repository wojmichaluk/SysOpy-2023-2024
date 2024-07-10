#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#define INTERVALS 10000

double function(double x) {
    return 4 / (x * x + 1);
}

double calculate_integral(double, double);

int main(int argc, char* argv[]) {
    double begin, end, result;
    char *name = "potok";
    mkfifo(name, 0666);

    int fd = open(name, O_RDONLY);

    read(fd, &begin, sizeof(double));
    read(fd, &end, sizeof(double));
    close(fd);

    result = calculate_integral(begin, end);
    fd = open(name, O_WRONLY);
    write(fd, &result, sizeof(double));
    
    close(fd);
    return 0;
}

double calculate_integral(double begin, double end) {
    double h = (end - begin) / INTERVALS;
    double result = 0.0;

    for (int i = 0; i < INTERVALS; i++) {
        double mid = begin + (i + 0.5) * h;
        result += h * function(mid);
    }

    return result;
}
