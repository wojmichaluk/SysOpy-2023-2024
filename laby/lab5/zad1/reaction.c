//so that signal.h functions are recognized
#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>

#define SIZE 4

int is_invalid_reaction(const char**, const char*);
void handler();

int main(int argc, char* argv[]) {
    const char* reactions[SIZE] = {"none", "ignore", "handler", "mask"};
    int index = -1;

    if (argc != 2 || (index = is_invalid_reaction(reactions, argv[1])) == -1) {
        printf("Nieprawidlowy format uruchomienia programu. Prawidlowy format:\n");
        printf("`./reaction <rodzaj reakcji>`\n");
        printf("Rodzaje reakcji: none | ignore | handler | mask\n\n");
        return 1;
    }

    switch(index) {
    case 0:
        signal(SIGUSR1, SIG_DFL);
        break;

    case 1:
        signal(SIGUSR1, SIG_IGN);
        break;

    case 2:
        signal(SIGUSR1, handler);
        break;

    case 3:
        sigset_t mask;
        sigemptyset(&mask);
        sigaddset(&mask, SIGUSR1);

        if (sigprocmask(SIG_SETMASK, &mask, NULL) < 0)
            perror("Nie udalo sie zablokowac symbolu\n");
        break;

    default:
        perror("To sie nie powinno wydarzyc!");
    }

    raise(SIGUSR1);

    //it needs to be done after raise(SIGNAL) call
    if (index == 3) {
        sigset_t pending;
        sigpending(&pending);
        printf("Czy nasz sygnal nalezy do grupy oczekujacych sygnalow? %d\n", sigismember(&pending, SIGUSR1));
    }

    return 0;
}

int is_invalid_reaction(const char** reactions, const char* input) {
    for (int i = 0; i < SIZE; i++) {
        if (!strcmp(reactions[i], input))
            return i;
    }

    return -1;
}

void handler() {
    printf("Otrzymalem sygnal\n");
}
