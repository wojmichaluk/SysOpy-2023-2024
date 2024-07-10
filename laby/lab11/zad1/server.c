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
#include <time.h>
#include <pthread.h>
#include <poll.h>

int socket_no;
int active_clients = 0;
bool flag = true;

pthread_t accept_thread;
struct pollfd poll_fds[MAX_CLIENTS];

typedef struct {
    client *clients;
    struct pollfd *pollfds;
} thread_args; 

bool invalid_params(char*, char*);
void* accept_routine(void*);
void handle_message(message*, client*);
int find_free_idx(client*);
int find_idx_by_id(const char*, client*);
void close_server();

void handle_accept(const message*, client*);
void handle_list(const message*, client*);
void handle_2all(message*, client*);
void handle_2one(message*, client*);
void handle_stop(const message*, client*);
void handle_alive(const message*, client*);

int main(int argc, char* argv[]) {
    if (argc != 3 || invalid_params(argv[1], argv[2])) {
        printf("Nieprawidlowe argumenty uruchomienia programu!\n");
        printf("Prawidlowa forma: `./server <numer_portu> <adres_serwera>`, gdzie\n");
        printf("<numer_portu> = 0 lub 1024 <= <numer_portu> <= 65535,\n");
        printf("<adres_serwera> jest poprawnym adresem IPv4.\n");
        return 1;
    }

    in_port_t port = htons((uint16_t)strtol(argv[1], NULL, 10));
    struct in_addr ipv4_addr;

    inet_aton(argv[2], &ipv4_addr);
    socket_no = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_no == -1) {
        perror("Blad tworzenia gniazda po stronie serwera!\n");
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = port;
    addr.sin_addr = ipv4_addr;

    if (bind(socket_no, (const struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("Blad zwiazania gniazda z adresem!\n");
        close_server();
    }

    if (listen(socket_no, BACKLOG) == -1) {
        perror("Blad rozpoczecia akceptowania polaczen od klientow!\n");
        close_server();
    }

    client clients[MAX_CLIENTS];
    message buffer;

    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].socket_no = -1;
    }

    for (int sig_nr = 1; sig_nr < SIGRTMAX; sig_nr++) {
        signal(sig_nr, close_server);
    }

    thread_args args;
    args.clients = clients;
    args.pollfds = poll_fds;
    pthread_create(&accept_thread, NULL, accept_routine, (void*)&args);

    while (flag) {
        if(poll(poll_fds, MAX_CLIENTS, 0) <= 0) {
            continue;
        }

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (poll_fds[i].revents & POLLIN) {
                if (recv(clients[i].socket_no, &buffer, sizeof(buffer), 0) != -1) {
                    handle_message(&buffer, clients);
                }
            }
        }
    }

    printf("\nZamykam serwer!\n");
    buffer.msg_type = 4;

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].socket_no != -1) {
            if (send(clients[i].socket_no, (const void*)&buffer, sizeof(buffer), 0) == -1) {
                perror("Blad przy wysylaniu wiadomosci konczacej do klienta!\n");
            }
        }
    }

    // shutdown(socket_no, SHUT_RDWR);
    close(socket_no);
    pthread_cancel(accept_thread);

    return 0;
}

bool invalid_params(char *port_param, char *addr_param) {
    char *temp;
    struct in_addr ipv4_addr;

    int port = (int)strtol(port_param, &temp, 10);
    int addr_status = inet_aton(addr_param, &ipv4_addr);

    if (!strcmp(temp, "\0") && (port == 0 || (port >= 1024 && port <= 65535)) && addr_status != 0) {
        return 0;
    }
    return 1;
}

void* accept_routine(void *args) {
    int active_idx;
    message buffer;

    thread_args *arguments = (thread_args*)args;
    client* clients = arguments->clients;
    struct pollfd *pollfds = arguments->pollfds;

    while (true) {
        int client_des = accept(socket_no, NULL, NULL);
        
        if (client_des != -1) {
            if (active_clients == MAX_CLIENTS) {
                buffer.msg_type = -1;
                printf("Serwer juz obsluguje maksymalna liczbe klientow! Nie moge przyjac nastepnego :(\n");

                if (send(client_des, (const void*)&buffer, sizeof(buffer), 0) == -1) {
                    perror("Blad przy wysylaniu wiadomosci odrzucajacej klienta!\n");
                }
            } else {
                active_idx = find_free_idx(clients);

                clients[active_idx].socket_no = client_des;
                pollfds[active_idx].fd = client_des;
                pollfds[active_idx].events = POLLIN | POLLOUT;

                buffer.msg_type = 0;
                buffer.idx_from = active_idx;
                printf("Serwer przyjal nowego uzytkownika i nadal mu id %d, witamy serdecznie\n", active_idx);
                
                if (send(client_des, (const void*)&buffer, sizeof(buffer), 0) == -1) {
                    perror("Blad przy wysylaniu wiadomosci potwierdzajacej zaakceptowanie klienta!\n");
                }

                active_clients++;
            }
        }
    }

    return NULL;
}

void handle_message(message* msg, client *clients) {
    int message_type = msg->msg_type;
    if (message_type < 0) {
        printf("Niepoprawny kod wiadomosci - nie rozpoznano!\n");
        return;
    }

    switch(message_type) {
    case 0:
        handle_accept(msg, clients);
        break;
    case 1:
        handle_list(msg, clients);
        break;

    case 2:
        handle_2all(msg, clients);
        break;

    case 3:
        handle_2one(msg, clients);
        break;

    case 4:
        handle_stop(msg, clients);
        break;

    case 5:
        handle_alive(msg, clients);
        break;

    case 6:
        break;

    default:
        printf("Niepoprawny kod wiadomosci - powinno to byc wychwycone wczesniej!\n");
        break;
    }
}

int find_free_idx(client *clients) {
    int i;
    for (i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].socket_no == -1) {
            break;
        }
    }
    return i;
}

int find_idx_by_id(const char *id, client *clients) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (!strcmp(id, clients[i].id)) {
            return i;
        }
    }
    return -1;
}

void close_server() {
    flag = false;
}

void handle_accept(const message *msg, client *clients) {
    strcpy(clients[msg->idx_from].id, msg->id_from);
}

void handle_list(const message *msg, client *clients) {
    printf("\nWyswietlam aktywnych klientow - id po stronie serwera i id klienta:\n");
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].socket_no != -1) {
            printf("Id serwera: %d id klienta: %s\n", i, clients[i].id);
        }
    }
    printf("\n");
}

void handle_2all(message *msg, client *clients) {
    time_t t;   
    time(&t);

    char* curr_time = ctime(&t);
    strcpy(msg->date, curr_time);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].socket_no != -1 && msg->idx_from != i) {
            if (send(clients[i].socket_no, (const void*)msg, sizeof(*msg), 0) == -1) {
                perror("Blad przy wysylaniu wiadomosci 2ALL do klienta!\n");
            }
        }
    }
}

void handle_2one(message *msg, client *clients) {
    time_t t;   
    time(&t);

    char* curr_time = ctime(&t);
    strcpy(msg->date, curr_time);
    int idx_to = find_idx_by_id(msg->id_to, clients);

    if (idx_to == -1) {
        printf("Blad wiadomosci 2ONE - nie znaleziono adresata wiadomosci\n");
    } else if (send(clients[idx_to].socket_no, (const void*)msg, sizeof(*msg), 0) == -1) {
        perror("Blad przy wysylaniu wiadomosci 2ONE do adresata!\n");
    }
}

void handle_stop(const message *msg, client *clients) {  
    printf("Wiadomosc STOP - wyrejestrowano klienta o id serwera %d\n", msg->idx_from);
    clients[msg->idx_from].socket_no = -1;
    active_clients--;
}

void handle_alive(const message *msg, client *clients) {
    printf("\nSprawdzam, czy klienci odpowiadaja...\n");

    int it = 0;
    message buffer;
    buffer.msg_type = 6;

    while (it++ < 5) {
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].socket_no != -1) {
                if (send(clients[i].socket_no, (const void*)&buffer, sizeof(buffer), 0) == -1) {
                    perror("Blad po stronie serwera przy pingowaniu klienta!\n");
                }
            }
        }

        poll(poll_fds, MAX_CLIENTS, 1000);

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (poll_fds[i].revents & POLLIN) {
                if (clients[i].socket_no != -1 && recv(clients[i].socket_no, &buffer, sizeof(buffer), 0) <= 0) {
                    printf("Klient o id serwera %d nie odpowiedzial - usuwam go zatem z listy klientow\n", i);

                    buffer.msg_type = 4;

                    if (send(clients[i].socket_no, (const void*)&buffer, sizeof(buffer), 0) == -1) {
                        perror("Blad przy probie usuniecia klienta, ktory nie odpowiedzial\n");
                    }

                    clients[i].socket_no = -1;
                    active_clients--;
                }
            }
        }
    }
}
