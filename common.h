//
// Created by jacek on 09.01.18.
//

#ifndef PROJEKT_COMMON_H
#define PROJEKT_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <error.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <ctype.h>
#include <string.h>

// boolean
#define TRUE 1
#define FALSE 0

#define CLIENT_LOGIN_REQUEST 100
#define CLIENT_LOGOUT_REQUEST 101
#define CLIENT_LOGGED_USERS_REQUEST 102
#define CLIENT_MSG_REQUEST 103
#define CLIENT_GROUPS_REQUEST 105
#define CLIENT_GROUP_USERS_REQUEST 106

#define SERVER_LOGIN_RESPONSE 200
#define SERVER_LOGOUT_RESPONSE 201
#define SERVER_LOGGED_USERS_RESPONSE 202
#define SERVER_MSG_RESPONSE 203
#define SERVER_MSG_GET 204
#define SERVER_GROUPS_RESPONSE 205
#define SERVER_GROUP_USERS_RESPONSE 206


struct Message {
    long type;
    char message[200];
    char sender[30];
    char receiver[30];
};

//struktura zwracana wysyłającemu
struct MessageResponse {
    long type;
    char result[10];
};

struct LoginRequest {
    long type;
    char username[30];
    char password[30];
    int pid;
};

struct LoginResponse {
    long type;
    char result[10];
    char username[30];
};

struct LogoutRequest {
    long type;
    char username[30];
};

struct LogoutResponse {
    long type;
    char result[10];
};

struct UsersListRequest {
    long type;
    char username[30];
};

struct UsersListResponse {
    long type;
    char users[280];
};

// obsługa wyświetlania grup
struct GroupsListRequest {
    long type;
    char username[30];
};

struct GroupsListResponse {
    long type;
    char groups[120];
};

//obsługa wyświetlania członków danej grupy
struct GroupUsersRequest {
    long type;
    char username[30];
    char group[30];
};

struct GroupUsersResponse {
    long type;
    char users[280];
    char result[10];
};

key_t server_msg_queue_key = 32452;

const char* SUCCESS = "success";
const char* FAILURE = "failure";

#endif //PROJEKT_COMMON_H
