#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#define MAX_CLIENTS 3
#define MSG_LENGTH 100

int flag = 1;
mqd_t qdes;

int handle_message(mqd_t*, int, const char*);
int is_init(const char*);
int is_close(const char*);
int find_id(mqd_t*, int);
void close_server();

int main(int argc, char* argv[]) {
    struct mq_attr sv_attr;
    sv_attr.mq_flags = 0;
    sv_attr.mq_msgsize = MSG_LENGTH;
    sv_attr.mq_maxmsg = 10;

    qdes = mq_open("/msg_queue", O_RDWR | O_CREAT, S_IRWXU, &sv_attr);
    if (qdes == -1) {
        perror("Nie udalo sie otworzyc kolejki komunikatow serwera!");
        mq_unlink("/msg_queue");
        return 1;
    }

    int last = 0;
    char message[MSG_LENGTH];
    mqd_t clients[MAX_CLIENTS];

    for (int sig_nr = 1; sig_nr < SIGRTMAX; sig_nr++) {
        signal(sig_nr, close_server);
    }

    printf("Otwieram serwer. Zapraszam do laczenia, ale zeby nie bylo wiecej niz %d klientow na raz!\n", MAX_CLIENTS);

    while (flag) {
        char *contents;
        mq_receive(qdes, message, MSG_LENGTH, NULL);
        strtol(message, &contents, 10);

        if (contents[1] != '\0') {
            last = handle_message(clients, last, message);
            contents[1] = '\0';
        }
    }

    printf("\nZamykam serwer. Zamykam z tego powodu wszystkich aktywnych klientow\n");
    strcpy(message, "-1 ");

    for (int i = 0; i < last; i++) {
        if (clients[i] != - 1) {
            mq_send(clients[i], message, strlen(message) + 1, 1);
            mq_close(clients[i]);
        }
    }

    mq_close(qdes);
    mq_unlink("/msg_queue");
    return 0;
}

int handle_message(mqd_t* clients, int size, const char* message) {
    if (is_init(message)) {
        char temp[MSG_LENGTH];
        strcpy(temp, message + 5);
        int current_id = find_id(clients, size);

        if (current_id < MAX_CLIENTS) {
            printf("Nawiazano polaczenie z klientem. Nadano identyfikator klienta: %d\n", current_id + 1);
            clients[current_id] = mq_open(temp, O_RDWR, S_IRWXU, NULL);
            sprintf(temp, "%d", current_id + 1);
            mq_send(clients[current_id], temp, strlen(temp) + 1, 1);

            size = size > current_id ? size : current_id + 1;
        } else {
            printf("Maksymalna liczba klientow - nie mozna zarejestrowac nowego klienta!\n");
            mqd_t rejected_client = mq_open(temp, O_RDWR, S_IRWXU, NULL);
            sprintf(temp, "%d", -1);
            mq_send(rejected_client, temp, strlen(temp) + 1, 1);
        }   
    } else if (is_close(message)) {
        int close_id = (int)strtol(message + 6, NULL, 10) - 1;
        clients[close_id] = -1;
        printf("Zgodnie z prosba zamykam klienta o identyfikatorze %d. Zwalniam ten identyfikator\n", close_id + 1);
    } else {
        int client_id = (int)strtol(message, NULL, 10) - 1;

        for (int i = 0; i < size; i++) {
            if (i != client_id && clients[i] != -1) {
                mq_send(clients[i], message, strlen(message) + 1, 1);
            }
        }
    }

    return size;
}

int is_init(const char* message) {
    if (strlen(message) < 4) {
        return 0;
    }

    char init[] = "INIT";
    for (int i = 0; i < 4; i++) {
        if (message[i] != init[i]) {
            return 0;
        }
    }

    return 1;
}

int is_close(const char *message) {
    if (strlen(message) < 5) {
        return 0;
    }

    char close[] = "CLOSE";
    for (int i = 0; i < 5; i++) {
        if (message[i] != close[i]) {
            return 0;
        }
    }

    return 1;
}

int find_id(mqd_t* clients, int size) {
    for (int i = 0; i < size; i++) {
        if (clients[i] == -1) {
            return i;
        }
    }

    return size;
}

void close_server() { 
    flag = 0; 
}
