#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

int check_argument(char *arg);

int main(int argc, char* argv[]) {
    if (argc != 2 || !check_argument(argv[1])) {
        printf("Nieprawidlowy format uruchomienia programu. Prawidlowy format:\n");
        printf("'`./descendant <liczba>`, gdzie liczba jest z zakresu 0-100.\n");
        return 1;
    }

    int num = atoi(argv[1]);
    pid_t child_pid = 0;
    
    if (num > 0) {
        child_pid = fork();
    }
    
    for (int i = 1; i < num; i++) {
        if (child_pid != 0)
            child_pid = fork();
    }

    if (child_pid == 0) {
        printf("Identyfikator procesu macierzystego: %d\n", (int)getppid());
        printf("Identyfikator własny: %d\n", (int)getpid());
    } else {
        wait(NULL);
        printf("Wartość argumentu wywołania argv[1]: %d\n", num);
    }

    return 0;
}

int check_argument(char *arg) {
    int n = strlen(arg);

    for (int i = 0; i < n; i++) {
        if (!isdigit(arg[i])) {
            return 0;
        }
    }

    if (atoi(arg) > 100) {
        return 0;
    }
    return 1;
}
