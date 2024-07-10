#define _DEFAULT_SOURCE

#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>

int socket_no;
int active_idx;
int active_clients = 0;
bool flag = true;

bool invalid_params(char*, char*);
void handle_message(message*, client*, struct sockaddr_in*);
int find_free_idx(client*);
int find_idx_by_id(const char*, client*);

void close_server() { flag = false; }

void handle_accept(message*, client*, struct sockaddr_in*);
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
    socket_no = socket(AF_INET, SOCK_DGRAM, 0);

    if (socket_no == -1) {
        perror("Blad tworzenia gniazda po stronie serwera!\n");
        return 1;
    }

    struct sockaddr_in client_addr, server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = port;
    server_addr.sin_addr = ipv4_addr;

    if (bind(socket_no, (const struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Blad zwiazania gniazda z adresem po stronie serwera!\n");
        close_server();
    }

    client clients[MAX_CLIENTS];
    message buffer;
    socklen_t addr_size;

    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].active = false;
    }

    for (int sig_nr = 1; sig_nr < SIGRTMAX; sig_nr++) {
        signal(sig_nr, close_server);
    }

    printf("Otwieram serwer. Zapraszam do laczenia, ale zeby nie bylo wiecej niz %d klientow na raz!\n\n", MAX_CLIENTS);

    while (flag) {
        addr_size = sizeof(client_addr);

        if (recvfrom(socket_no, &buffer, sizeof(buffer), MSG_DONTWAIT, (struct sockaddr*)&client_addr, &addr_size) != -1) {
            handle_message(&buffer, clients, &client_addr);
        }
    }

    printf("\nZamykam serwer!\n");
    buffer.msg_type = 4;

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active) {
            if (sendto(socket_no, (const void*)&buffer, sizeof(buffer), 0, &clients[i].addr, sizeof(clients[i].addr)) == -1) {
                perror("Blad przy wysylaniu wiadomosci konczacej do klienta!\n");
            }
        }
    }

    close(socket_no);

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

void handle_message(message* msg, client *clients, struct sockaddr_in* cli) {
    int message_type = msg->msg_type;
    if (message_type < 0) {
        printf("Niepoprawny kod wiadomosci - nie rozpoznano!\n");
        return;
    }

    switch(message_type) {
    case 0:
        handle_accept(msg, clients, cli);
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
        if (!clients[i].active) {
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

void handle_accept(message *msg, client *clients, struct sockaddr_in* cli) {
    if (active_clients == MAX_CLIENTS) {
        msg->msg_type = -1;
        printf("Serwer juz obsluguje maksymalna liczbe klientow! Nie moge przyjac nastepnego :(\n");

        if (sendto(socket_no, (const void*)msg, sizeof(*msg), 0, (const struct sockaddr*)cli, sizeof(*cli)) == -1) {
            perror("Blad przy wysylaniu wiadomosci odrzucajacej klienta!\n");
        }
    } else {
        active_idx = find_free_idx(clients);

        clients[active_idx].active = true;
        clients[active_idx].addr = *(struct sockaddr*)cli;
        strcpy(clients[active_idx].id, msg->id_from);

        msg->msg_type = 0;
        msg->idx_from = active_idx;
        printf("Serwer przyjal nowego uzytkownika i nadal mu id %d, witamy serdecznie\n", active_idx);
        
        if (sendto(socket_no, (const void*)msg, sizeof(*msg), 0, &clients[active_idx].addr, sizeof(clients[active_idx].addr)) == -1) {
            perror("Blad przy wysylaniu wiadomosci potwierdzajacej zaakceptowanie klienta!\n");
        }

        active_clients++;
    }
}

void handle_list(const message *msg, client *clients) {
    printf("\nWyswietlam aktywnych klientow - id po stronie serwera i id klienta:\n");
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active) {
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
        if (clients[i].active && msg->idx_from != i) {
            if (sendto(socket_no, (const void*)msg, sizeof(*msg), 0, &clients[i].addr, sizeof(clients[i].addr)) == -1) {
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
    } else if (sendto(socket_no, (const void*)msg, sizeof(*msg), 0, &clients[idx_to].addr, sizeof(clients[idx_to].addr)) == -1) {
        perror("Blad przy wysylaniu wiadomosci 2ONE do adresata!\n");
    }
}

void handle_stop(const message *msg, client *clients) {  
    printf("Wiadomosc STOP - wyrejestrowano klienta o id serwera %d\n", msg->idx_from);
    clients[msg->idx_from].active = false;
    active_clients--;
}

void handle_alive(const message *msg, client *clients) {
    printf("\nSprawdzam, czy klienci odpowiadaja...\n");

    socklen_t addr_size;
    message buffer;

    int it = 0;
    buffer.msg_type = 6;

    while (it++ < 5) {
        for (int i = 0; i < MAX_CLIENTS; i++) {
            addr_size = sizeof(clients[i].addr);

            if (clients[i].active) {
                if (sendto(socket_no, (const void*)&buffer, sizeof(buffer), 0, &clients[i].addr, sizeof(clients[i].addr)) == -1) {
                    perror("Blad po stronie serwera przy pingowaniu klienta!\n");
                }

                sleep(1);

                if (recvfrom(socket_no, (void *)&buffer, sizeof(buffer), 0, &clients[i].addr, &addr_size) <= 0) {
                    printf("Klient o id serwera %d nie odpowiedzial - usuwam go zatem z listy klientow\n", i);

                    buffer.msg_type = 4;

                    if (sendto(socket_no, (const void*)&buffer, sizeof(buffer), 0, &clients[i].addr, sizeof(clients[i].addr)) == -1) {
                        perror("Blad przy probie usuniecia klienta, ktory nie odpowiedzial\n");
                    }

                    clients[i].active = false;
                    active_clients--;
                }
            }
        }

        printf("Sprawdzono aktywnosc klientow - tura %d\n", it);
    }

    printf("\nDo kazdego aktywnego klienta przeslano wiadomosc testowa %d razy\n\n", it - 1);
}
