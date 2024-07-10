#define _DEFAULT_SOURCE

#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>

int socket_no;
int client_idx;
pthread_t receive_thread;

bool invalid_params(char*, char*, char*);
void* receive_routine(void*);
void init_connection(int, char*);
void prepare_message(message*, const char*);
void handle_message(const message*);
int get_message_type(const char*);
void close_client();
void cleanup();

int main(int argc, char* argv[]) {
    if (argc != 4 || invalid_params(argv[1], argv[2], argv[3])) {
        printf("Nieprawidlowe argumenty uruchomienia programu!\n");
        printf("Prawidlowa forma: `./client <id_klienta> <numer_portu> <adres_serwera>`, gdzie\n");
        printf("<id_klienta> to ciąg znaków o długości od 5 do 20 znaków,\n");
        printf("<numer_portu> = 0 lub 1024 <= <numer_portu> <= 65535 i jest taki sam jak serwera, natomiast\n");
        printf("<adres_serwera> jest poprawnym adresem IPv4, zgodnym z adresem serwera.\n");
        return 1;
    }

    in_port_t port = htons((uint16_t)strtol(argv[2], NULL, 10));
    struct in_addr ipv4_addr;
    inet_aton(argv[3], &ipv4_addr);

    socket_no = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_no == -1) {
        perror("Blad tworzenia gniazda po stronie klienta!\n");
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = port;
    addr.sin_addr = ipv4_addr;

    if (connect(socket_no, (const struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("Blad połączenia klienta z serwerem!\n");
        cleanup();
    }

    for (int sig_nr = 1; sig_nr < SIGRTMAX; sig_nr++) {
        signal(sig_nr, close_client);
    }

    
    char whole_message[MAX_MSG_SIZE + MAX_ID_LEN + 5];
    message buffer;

    pthread_create(&receive_thread, NULL, receive_routine, NULL);
    init_connection(socket_no, argv[1]);

    while (fgets(whole_message, MAX_MSG_SIZE + MAX_ID_LEN + 5, stdin) != NULL) {
        whole_message[strcspn(whole_message, "\n")] = '\0';
        int message_type = get_message_type(whole_message);

        if (message_type == 3) {
            prepare_message(&buffer, whole_message + 5);
        } else if (message_type == 2) {
            if (strlen(whole_message) > 5) {
                strcpy(buffer.msg, whole_message + 5);
            } else {
                strcpy(buffer.msg, "");
            }
        }

        buffer.msg_type = message_type;
        buffer.idx_from = client_idx;
        strcpy(buffer.id_from, argv[1]);

        if (send(socket_no, (const void*)&buffer, sizeof(buffer), 0) == -1) {
            perror("Blad przy wysylaniu wiadomosci do serwera!\n");
        }

        if (message_type == 4) {
            cleanup();
        }
    }

    return 0;
}

bool invalid_params(char *param1, char *param2, char *param3) {
    int name_len = strlen(param1);
    if (name_len < MIN_ID_LEN || name_len > MAX_ID_LEN) {
        return 1;
    }

    char *temp;
    struct in_addr ipv4_addr;

    int port = (int)strtol(param2, &temp, 10);
    int addr_status = inet_aton(param3, &ipv4_addr);

    if (!strcmp(temp, "\0") && (port == 0 || (port >= 1024 && port <= 65535)) && addr_status != 0) {
        return 0;
    }
    return 1;
}

void* receive_routine(void *args) {
    message buffer;

    while (true) {
        if (recv(socket_no, (void*)&buffer, sizeof(buffer), 0) != -1) {
            handle_message(&buffer);
        }
    }

    return NULL;
}

void init_connection(int socket_no, char *id) {
    message buffer;

    if (recv(socket_no, (void*)&buffer, sizeof(buffer), 0) != -1) {
        int message_type = buffer.msg_type;
        
        if (message_type == -1) {
            printf("Niestety, serwer mnie odrzucil :( - do widzenia!\n");
            close_client();
        } else {
            printf("Zostalem przyjety przez serwer - swietna sprawa!\n");

            client_idx = buffer.idx_from;
            buffer.msg_type = 0;
            strcpy(buffer.id_from, id);

            if (send(socket_no, (const void*)&buffer, sizeof(buffer), 0) == -1) {
                perror("Blad przy wysylaniu wiadomosci potwierdzajacej zaakceptowanie od klienta!\n");
            }
        }
    }
}

void prepare_message(message *buffer, const char *whole_message) {
    int len = strlen(whole_message);
    int id_end = 0;

    for (int i = 0; i < len; i++) {
        if (whole_message[i] == ' ') {
            id_end = i;
            break;
        }
    }

    id_end = id_end > MAX_ID_LEN ? MAX_ID_LEN : id_end;
    strncpy(buffer->id_to, whole_message, id_end);

    if (id_end == len) {
        strcpy(buffer->msg, "");
    } else {
        strcpy(buffer->msg, whole_message + id_end + 1);
    }
}

void handle_message(const message *buffer) {
    int message_type = buffer->msg_type;
    if (message_type < 0) {
        printf("Niepoprawny kod wiadomosci - nie rozpoznano!");
        return;
    }

    switch(message_type) {
    case 2:
    case 3:
        printf("\nOtrzymano wiadomosc od %s\n", buffer->id_from);
        printf("Data wyslania wiadomosci: %s", buffer->date);
        printf("Tresc wiadomosci: %s\n\n", buffer->msg);
        break;

    case 4:
        cleanup();
        break;

    case 6:
        if (send(socket_no, (const void*)buffer, sizeof(*buffer), 0) == -1) {
            perror("Blad przy wysylaniu odpowiedzi na pingowanie!\n");
        }
        break;

    default:
        printf("Niepoprawny kod wiadomosci - nie powinien taki wystapic!\n");
        break;
    }
}

int get_message_type(const char *msg) {
    char init[6];
    strncpy(init, msg, 5);

    if (strlen(msg) < 5) {
        init[4] = ' ';
    }
    init[5] = '\0';

    if (!strcmp(init, "LIST ")) {
        return 1;
    } else if (!strcmp(init, "2ALL ")) {
        return 2;
    } else if (!strcmp(init, "2ONE ")) {
        return 3;
    } else if (!strcmp(init, "STOP ")) {
        return 4;
    } else if (!strcmp(init, "ALIVE")) {
        return 5;
    } else {
        return -1;
    }
}

void close_client() {
    message buffer;
    buffer.idx_from = client_idx;
    buffer.msg_type = 4;

    if (send(socket_no, (const void*)&buffer, sizeof(buffer), 0) == -1) {
        perror("Blad przy wysylaniu wiadomosci konczacej do serwera!\n");
    }
    cleanup();
}

void cleanup() {
    printf("\nMoja przygoda konczy sie w tym miejscu - bywaj druhu!\n");
    close(socket_no);
    pthread_cancel(receive_thread);
    exit(1);
}
