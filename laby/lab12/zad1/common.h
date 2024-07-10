#pragma once

#include <netinet/in.h>
#include <stdbool.h>

#define MAX_CLIENTS 4
#define MAX_MSG_SIZE 100
#define MIN_ID_LEN 5
#define MAX_ID_LEN 20

typedef struct {
    char msg[MAX_MSG_SIZE];
    char id_from[MAX_ID_LEN];
    char id_to[MAX_ID_LEN];
    char date[MAX_MSG_SIZE];
    int msg_type;
    int idx_from;
} message;

typedef struct {
    char id[MAX_ID_LEN];
    struct sockaddr addr;
    bool active;
} client;
