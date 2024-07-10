#include <stdio.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#define TO_PRINT 10
#define MAX_TIME 10
#define MAX_USERS 10

int flag = 1;
void handle_signal() { flag = 0; }

int invalid_params(char**);
void fill_random_letters(char*);

int main(int argc, char* argv[]) {
    if (argc != 2 || invalid_params(argv)) {
        printf("Nieprawidlowe argumenty uruchomienia programu!\n");
        printf("Prawidlowa forma: `./user <liczba_uzytkownikow>`,");
        printf(" gdzie 0 < liczba uzytkownikow <= 20\n");
        return 1;
    }

    int M = (int)strtol(argv[1], NULL, 10);
    const char filename[] = "/tmp";

    key_t token = ftok(filename, 'S');
    if (token == (key_t)-1) {
        perror("Blad generowania tokenu semaforow z grupy 1!");
        return 1;
    }

    int semaphore1 = semget(token, 0, 0666);
    if (semaphore1 == -1) {
        perror("Blad pobierania semaforow z grupy 1 - zapewne jeszcze nie zostaly utworzone!");
        return 1;
    }

    token = ftok(filename, 's');
    if (token == (key_t)-1) {
        perror("Blad generowania tokenu semafora z grupy 2!");
        return 1;
    }

    int semaphore2 = semget(token, 0, 0666);
    if (semaphore2 == -1) {
        perror("Blad pobierania semafora z grupy 2 - zapewne jeszcze nie zostal utworzony!");
        return 1;
    }

    token = ftok(filename, 'M');
    if (token == (key_t)-1) {
        perror("Blad generowania tokenu pamieci dzielonej!");
        return 1;
    }

    int shared_memory = shmget(token, 0, 0666);
    if (shared_memory == -1) {
        perror("Blad pobierania segmentu pamieci dzielonej - zapewne jeszcze nie zostal utworzony!");
        return 1;
    }

    void *mem = shmat(shared_memory, NULL, SHM_W);
    if (mem == (void*)-1) {
        perror("Blad dolaczania segmentu pamieci dzielonej!");
        return 1;
    }

    char to_print[TO_PRINT + 1];
    struct sembuf operation1, operation2;

    operation1.sem_num = 0;
    operation1.sem_op = -1;
    operation1.sem_flg = SEM_UNDO;

    operation2.sem_num = 1;
    operation2.sem_op = -1;
    operation2.sem_flg = SEM_UNDO;

    for (int sig_nr = 1; sig_nr < SIGRTMAX; sig_nr++) {
        signal(sig_nr, handle_signal);
    }

    for (int i = 0; i < M - 1; i++) {
        pid_t child = fork();
        if (child == -1) {
            perror("Blad przy tworzeniu procesu potomnego!");
            return 1;
        }
        if (child == 0) {
            break;
        }
    }

    time_t t;
    srand(time(&t) % getpid());

    while (flag) {
        fill_random_letters(to_print);
        semop(semaphore2, &operation1, 1);
        semop(semaphore1, &operation1, 1);
        strcpy(mem, to_print);
        semop(semaphore1, &operation2, 1);
        
        sleep(rand() % MAX_TIME + 3);
    }

    int detach = shmdt(mem);
    if (detach == -1) {
        perror("Blad odlaczania segmentu pamieci dzielonej!");
        return 1;
    }

    return 0;
}

int invalid_params(char** params) {
    char *temp;
    int no_users = (int)strtol(params[1], &temp, 10);

    if (temp == NULL && no_users > 0 && no_users <= MAX_USERS) {
        return 1;
    }
    return 0;
}

void fill_random_letters(char *to_print) {
    for (int i = 0; i < TO_PRINT; i++) {
        to_print[i] = rand() % ('z' - 'a' + 1) + 'a';
    }
    to_print[TO_PRINT] = '\0';
}
