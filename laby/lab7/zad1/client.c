#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#define MSG_LENGTH 100

int flag = 1;
mqd_t cl_qdes;
mqd_t sv_qdes;

void close_client();

int main(int argc, char* argv[]) {
    char filename[MSG_LENGTH - 10];
    char message[MSG_LENGTH];
    sprintf(filename, "/cl_q_%d", getpid());

    struct mq_attr cl_attr;
    cl_attr.mq_flags = 0;
    cl_attr.mq_msgsize = MSG_LENGTH;
    cl_attr.mq_maxmsg = 3;

    cl_qdes = mq_open(filename, O_RDWR | O_CREAT, S_IRWXU, &cl_attr);
    if (cl_qdes == -1) {
        perror("Nie udalo sie otworzyc kolejki komunikatow klienta!");
        mq_unlink(filename);
        return 1;
    }

    sv_qdes = mq_open("/msg_queue", O_RDWR | O_CREAT, S_IRWXU, NULL);
    if (sv_qdes == -1) {
        perror("Nie udalo sie otworzyc kolejki komunikatow serwera!");
        mq_unlink(filename);
        return 1;
    }

    sprintf(message, "INIT %s", filename);
    mq_send(sv_qdes, message, strlen(message) + 1, 1);
    mq_receive(cl_qdes, message, MSG_LENGTH, NULL);

    int client_id = strtol(message, NULL, 10);
    if (client_id == 0) {
        perror("Blad przydzielenia id klientowi!");
        mq_unlink(filename);
        return 1;
    }

    if (client_id == -1) {
        printf("Dla tego klienta juz nie ma miejsca w gospodzie... znaczy na serwerze\n");
        mq_unlink(filename);
        return 1;
    }

    message[0] = '\0';
    printf("Udalo sie nawiazac polaczenie z serwerem. Identyfikator klienta: %d\n", client_id);

    for (int sig_nr = 1; sig_nr < SIGRTMAX; sig_nr++) {
        signal(sig_nr, close_client);
    }

    pid_t child_pid = fork();
    if (child_pid == -1) {
        perror("Blad tworzenia procesu potomnego!");
        mq_unlink(filename);
        return 1;
    }

    if (child_pid == 0) {
        char *contents;
        int from_id;

        while(flag) {
            mq_receive(cl_qdes, message, MSG_LENGTH, NULL);
            from_id = (int)strtol(message, &contents, 10);

            if (from_id == -1) {
                printf("Serwer zamkniety, pora i na mnie!\n");
                mq_close(cl_qdes);
                mq_close(sv_qdes);

                mq_unlink(filename);
                return 0;
            }

            if (contents[1] != '\0') {
                printf("Otrzymano wiadomosc od klienta %d: %s\n", from_id, contents + 1);
                contents[1] = '\0';
            }
        }
    } else {
        char contents[MSG_LENGTH - 10];
        printf("Tu mozesz wpisywac wiadomosci. Tutaj takze pojawia sie wiadomosci od innych klientow.\n\n");

        while (flag) {
            fgets(contents, MSG_LENGTH - 10, stdin);
            contents[strcspn(contents, "\n")] = '\0';
            sprintf(message, "%d %s", client_id, contents);
            mq_send(sv_qdes, message, strlen(message) + 1, 1);
        }
    }

    if (child_pid == 0) {
        printf("\nZamykam klienta. Informuje o tym serwer\n");
        sprintf(message, "CLOSE %d", client_id);
        sv_qdes = mq_open("/msg_queue", O_RDWR | O_CREAT, S_IRWXU, NULL);
        mq_send(sv_qdes, message, strlen(message) + 1, 1);
        mq_close(sv_qdes);
    }
         
    mq_unlink(filename);
    return 0;
}

void close_client() { 
    flag = 0; 
    mq_close(cl_qdes);
    mq_close(sv_qdes);
}
