//so that signal.h functions are recognized
#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>

void handler(int, siginfo_t*, void*);
void mode_one();
void mode_two();

static int prev = -1;
static int changes = -1;
static int flag = -1;

int main(int argc, char* argv[]) {
    printf("Program `catcher`, PID = %d\n", (int)getpid());

    struct sigaction act;
    act.sa_flags = SA_SIGINFO;
    sigemptyset(&act.sa_mask);
    act.sa_sigaction = handler;
    sigaction(SIGUSR1, &act, NULL);

    while (flag != 3) {
        pause();
    }

    printf("Tryb 3 - koncze dzialanie programu `catcher`\n");

    return 0;
}

void handler(int signo, siginfo_t* info, void* ptr) {
    flag = info->si_value.sival_int;

    if (!flag) { //waiting for confirmation
        printf("Tutaj `catcher`. Potwierdzam odbior sygnalu, wysylam sygnal zwrotny\n");
    } else {
        if (flag != prev) {
            prev = flag;
            changes++;
        }

        if (flag == 1) {
            mode_one();
        } else if (flag == 2) {
            mode_two();
        }
    }

    kill(info->si_pid, SIGUSR1);
}

void mode_one() {
    printf("\n");

    for (int i = 1; i < 101; i++) {
        printf("%d", i);
        i % 10 ? printf("\t") : printf("\n");
    }

    printf("\n");
}

void mode_two() {
    printf("Liczba zmian trybu pracy od poczatku dzialania programu: %d\n", changes);
}
