#include <stdio.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#define MAX_PRINTERS 10
#define TO_PRINT 10

int flag = 1;
union semun {
    int val;
    struct semid_ds *buf;
    unsigned short  *array;
};

void handle_signal() { flag = 0; }

int invalid_params(char**);
int determine_next(int, int);

int main(int argc, char* argv[]) {
    if (argc != 2 || invalid_params(argv)) {
        printf("Nieprawidlowe argumenty uruchomienia programu!\n");
        printf("Prawidlowa forma: `./printer <liczba_drukarek>`,");
        printf(" gdzie 0 < liczba drukarek <= 10\n");
        return 1;
    }

    int N = (int)strtol(argv[1], NULL, 10);
    const char filename[] = "/tmp";

    key_t token = ftok(filename, 'S');
    if (token == (key_t)-1) {
        perror("Blad generowania tokenu semaforow z grupy 1!");
        return 1;
    }

    int semaphore1 = semget(token, 2, IPC_CREAT | 0666);
    if (semaphore1 == -1) {
        perror("Blad tworzenia semaforow z grupy 1!");
        return 1;
    }

    token = ftok(filename, 's');
    if (token == (key_t)-1) {
        perror("Blad generowania tokenu semafora z grupy 2!");
        semctl(semaphore1, 0, IPC_RMID);
        return 1;
    }

    int semaphore2 = semget(token, 1, IPC_CREAT | 0666);
    if (semaphore2 == -1) {
        perror("Blad tworzenia semafora z grupy 2!");
        semctl(semaphore1, 0, IPC_RMID);
        return 1;
    }

    union semun arg;
    arg.val = 1;
    semctl(semaphore1, 0, SETVAL, arg);
    semctl(semaphore1, 1, SETVAL, arg);
    arg.val = N;
    semctl(semaphore2, 0, SETVAL, arg);

    token = ftok(filename, 'M');
    if (token == (key_t)-1) {
        perror("Blad generowania tokenu pamieci dzielonej!");
        semctl(semaphore1, 0, IPC_RMID);
        semctl(semaphore2, 0, IPC_RMID);
        return 1;
    }

    int shared_memory = shmget(token, TO_PRINT + 1, IPC_CREAT | 0666);
    if (shared_memory == -1) {
        perror("Blad tworzenia segmentu pamieci dzielonej!");
        semctl(semaphore1, 0, IPC_RMID);
        semctl(semaphore2, 0, IPC_RMID);
        return 1;
    }

    void *mem = shmat(shared_memory, NULL, SHM_RDONLY);
    if (mem == (void*)-1) {
        perror("Blad dolaczania segmentu pamieci dzielonej!");
        semctl(semaphore1, 0, IPC_RMID);
        semctl(semaphore2, 0, IPC_RMID);
        shmctl(shared_memory, IPC_RMID, NULL);
        return 1;
    }
    
    char to_print[TO_PRINT + 1];
    struct sembuf operation1, operation2;

    operation1.sem_num = 0;
    operation1.sem_op = 1;
    operation1.sem_flg = SEM_UNDO;

    operation2.sem_num = 1;
    operation2.sem_op = 1;
    operation2.sem_flg = SEM_UNDO;

    for (int sig_nr = 1; sig_nr < SIGRTMAX; sig_nr++) {
        signal(sig_nr, handle_signal);
    }
 
    pid_t child;
    int printer_id;
    int pipe_from;
    int pipes_to[MAX_PRINTERS];

    for (int i = 0; i < N; i++) {
        int fd[2];
        pipe(fd);
        child = fork();

        if (child == -1) {
            perror("Blad przy tworzeniu procesu potomnego!");
            semctl(semaphore1, 0, IPC_RMID);
            semctl(semaphore2, 0, IPC_RMID);
            shmctl(shared_memory, IPC_RMID, NULL);
            return 1;
        }
        if (child == 0) {
            close(fd[1]);
            pipe_from = fd[0];
            printer_id = i;
            break;
        } else {
            close(fd[0]);
            pipes_to[i] = fd[1];
        }
    }

    if (child != 0) {
        int current = 0;

        while (flag) {
            int val1 = semctl(semaphore1, 0, GETVAL, arg);
            int val2 = semctl(semaphore1, 1, GETVAL, arg);

            if (val1 == 0 && val2 == 0) {
                strcpy(to_print, mem);
                write(pipes_to[current], to_print, TO_PRINT + 1);

                semop(semaphore1, &operation1, 1);
                semop(semaphore1, &operation2, 1);
                current = determine_next(current, N);
            }
        }

        semctl(semaphore1, 0, IPC_RMID);
        semctl(semaphore2, 0, IPC_RMID);
        shmctl(shared_memory, IPC_RMID, NULL);
    } else {
        while (flag) { 
            read(pipe_from, to_print, TO_PRINT + 1);

            if (flag) {
                for (int i = 0; i < TO_PRINT; i++) {
                    printf("Drukarka %d, znak %d: %c\n", printer_id + 1, i + 1, to_print[i]);
                    sleep(1);
                }

                printf("\n");
                semop(semaphore2, &operation1, 1);
            }
        }
    }

    if (shmdt(mem) == -1) {
        perror("Blad odlaczania segmentu pamieci dzielonej!");
    }
    return 0;
}

int invalid_params(char** params) {
    char *temp;
    int no_printers = (int)strtol(params[1], &temp, 10);

    if (temp == NULL && no_printers > 0 && no_printers <= MAX_PRINTERS) {
        return 1;
    }
    return 0;
}

int determine_next(int current, int limit) {
    if (current < limit - 1) {
        return current + 1;
    }
    return 0;
}
