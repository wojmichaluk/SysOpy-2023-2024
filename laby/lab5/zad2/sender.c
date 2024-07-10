//so that signal.h functions are recognized
#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>

int invalid_input(const char*, const char*);
void confirm();
void clear() {}

int main(int argc, char* argv[]) {
    if (argc != 3 || invalid_input(argv[1], argv[2])) {
        printf("Nieprawidlowy format uruchomienia programu. Prawidlowy format:\n");
        printf("'`./sender <PID programu `catcher`> <tryb pracy procesu `catcher`>`\n");
        printf("Tryby pracy:\n1) wypisanie na wyjsciu liczb od 1 do 100\n");
        printf("2) wypisanie na wyjsciu liczby otrzymanych zadan zmiany trybu pracy\n");
        printf("3) zakonczenie dzialania programu catcher\n");
        return 1;
    }

    int pid = atoi(argv[1]);
    char mode = atoi(argv[2]);

    signal(SIGUSR1, confirm);

    union sigval value;
    value.sival_int = 0; //before actual mode
    sigqueue(pid, SIGUSR1, value);

    sigset_t set;
    sigfillset(&set);
    sigdelset(&set, SIGUSR1);
    sigsuspend(&set);

    value.sival_int = mode;
    signal(SIGUSR1, clear);
    sigqueue(pid, SIGUSR1, value);
    sigsuspend(&set);

    return 0;
}

int invalid_input(const char* pid, const char* mode) {
    int possible_pid, possible_mode;

    if ((possible_pid = atoi(pid)) < 0)
        return 1;

    possible_mode = atoi(mode);
    if (possible_mode < 1 || possible_mode > 3)
        return 1;

    return 0;
}

void confirm() {
    printf("Tutaj `sender`. Przyjmuje potwierdzenie, wysylam wlasciwy sygnal\n");
}
