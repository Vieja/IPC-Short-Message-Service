#include "common.h"


void help() {
    puts("\n\n----------------------------------------");
    puts("Dostępne opcje:");
    puts("/help - wyświetl pomoc");
    puts("/show_users - wyświetl wszystkich zalogowanych użytkowników");
    puts("/show_groups - wyświetl dostępne grupy");
    puts("/show_group_users - wyświetl użytkowników zapisanych do konkretnej grupy");
    puts("/join_group - dołącz do konkretnej grupy");
    puts("/leave_group - opuść konkretną grupę");
    puts("/logout - wyloguj");
    puts("----------------------------------------");
    puts("\nAby wysłać wiadomość:");
    puts("'adresat': 'treść wiadomości[200]'");
    puts("np.: dbrzez: Witaj!\n");
    puts("----------------------------------------\n");
}

void execute_order_66() {
    puts("[KLIENT] Zamykanie programu");
    exit(0);
}

struct user_data {
    int ID_USER;
    char username[30];
};

struct user_data logowanie(int ID_LOG,int PID) {
    char password[30];
    struct user_data userData;

    while(1) {
        struct LoginRequest login_request;
        login_request.pid = PID;
        login_request.type = CLIENT_LOGIN_REQUEST;
        struct LoginResponse login_response;

        puts("\n[KLIENT] Wprowadź nazwę użytkownika:");
        scanf("%s",userData.username);
        puts("[KLIENT] Wprowadź hasło:");
        scanf("%s",password);
        strcpy(login_request.username,userData.username);
        strcpy(login_request.password,password);

        //PRÓBA LOGOWANIA
        puts("[KLIENT] <sending login_request>");
        msgsnd(ID_LOG,&login_request,sizeof(login_request),0);
        while( msgrcv(ID_LOG,&login_response, sizeof(login_response),PID,0) == -1 ); //CZEKANIE NA ODPOWIEDZ SERWERA
        if ( strcmp(login_response.result,SUCCESS) == 0) {
            userData.ID_USER = msgget(PID,0600);
            puts("[KLIENT] Success: Zalogowano pomyślnie");
            printf("\n     Witaj %s!",userData.username);
            break;
        } else {
            puts("[KIENT] Error: Nieudane logowanie. Spróbuj ponownie");
        }
    }
    help();
    return userData;
}

void wylogowanie(char username[30],int ID_LOG,int PID) {
    puts("[KLIENT] Próba wylogowania");
    struct LogoutRequest logout_request;
    logout_request.type = CLIENT_LOGOUT_REQUEST;
    strcpy(logout_request.username,username);
    puts("[KLIENT] <sending logout_request>");
    msgsnd(ID_LOG,&logout_request, sizeof(logout_request),0);
    struct LogoutResponse logout_response;
    while( msgrcv(ID_LOG,&logout_response, sizeof(logout_response),PID,0) == -1 ); //CZEKANIE NA ODPOWIEDZ SERWERA
    puts("[KLIENT] Success: Wylogowano pomyślnie");
}

void wyswietl_zalogowanych(char username[30],int ID_LOG) {
    puts("[KLIENT] Wyświetlenie zalogowanych użytkowników");
    struct UsersListRequest show_users_request;
    show_users_request.type = CLIENT_LOGGED_USERS_REQUEST;
    strcpy(show_users_request.username,username);
    puts("[KLIENT] <sending show_users_request>");
    msgsnd(ID_LOG,&show_users_request, sizeof(show_users_request),0);
}

void wyswietl_grupy(char username[30],int ID_LOG) {
    puts("[KLIENT] Wyświetlenie dostępnych grup");
    struct GroupsListRequest groups_list_request;
    groups_list_request.type = CLIENT_GROUPS_REQUEST;
    strcpy(groups_list_request.username,username);
    puts("[KLIENT] <sending groups_list_request>");
    msgsnd(ID_LOG,&groups_list_request, sizeof(groups_list_request),0);
}

void wyswietl_czlonkow(char username[30],int ID_LOG,char grupa[30]){
    printf("[KLIENT] Wyświetlenie członków grupy %s\n",grupa);
    struct GroupUsersRequest group_users_request;
    group_users_request.type = CLIENT_GROUP_USERS_REQUEST;
    strcpy(group_users_request.username,username);
    strcpy(group_users_request.group,grupa);
    puts("[KLIENT] <sending group_users_request>");
    msgsnd(ID_LOG,&group_users_request, sizeof(group_users_request),0);
}

void dolacz_grupa(char username[30],int ID_LOG,char grupa[30]){
    printf("[KLIENT] Próba dołączenia do grupy %s\n",grupa);
    struct JoinGroupRequest join_group_request;
    join_group_request.type = CLIENT_JOIN_REQUEST;
    strcpy(join_group_request.username,username);
    strcpy(join_group_request.group,grupa);
    puts("[KLIENT] <sending join_group_request>");
    msgsnd(ID_LOG,&join_group_request, sizeof(join_group_request),0);
}

void opusc_grupa(char username[30],int ID_LOG,char grupa[30]){
    printf("[KLIENT] Próba opusczenia grupy %s\n",grupa);
    struct LeaveGroupRequest leave_group_request;
    leave_group_request.type = CLIENT_LEAVE_REQUEST;
    strcpy(leave_group_request.username,username);
    strcpy(leave_group_request.group,grupa);
    puts("[KLIENT] <sending leave_group_request>");
    msgsnd(ID_LOG,&leave_group_request, sizeof(leave_group_request),0);
}

void send_msg(char receiver[30],char* msg,int ID_LOG,char sender[30]) {
    struct Message message;
    message.type = CLIENT_MSG_REQUEST;
    strcpy(message.receiver,receiver);
    strcpy(message.sender,sender);
    strcpy(message.message,msg);
    msgsnd(ID_LOG,&message, sizeof(message),0);
}


int main(int argc, char* argv[]) {
    puts("----------------------------------------");
    puts("[KLIENT] Witaj w Short Message Service!");
    puts("----------------------------------------");
    int ID_LOG = msgget(server_msg_queue_key,0600);
    int PID = getpid();
    struct user_data userData = logowanie(ID_LOG,PID);

    if (!fork()) {
        while(1) { //NASŁUCHIWANIE PRYWATNEJ KOLEJKI
            struct UsersListResponse show_users_response;
            if( msgrcv(userData.ID_USER,&show_users_response, sizeof(show_users_response),SERVER_LOGGED_USERS_RESPONSE,IPC_NOWAIT) != -1 ) {
                puts("\n[KLIENT] Zalogowani użytkownicy:");
                printf("%s\n\n", show_users_response.users);
            }

            struct Message message;
            if( msgrcv(userData.ID_USER,&message, sizeof(message),SERVER_MSG_GET,IPC_NOWAIT) != -1 ) {
                printf("from %s%s",message.sender,message.message);
            }

            struct MessageResponse message_response;
            if( msgrcv(userData.ID_USER,&message_response, sizeof(message),SERVER_MSG_RESPONSE,IPC_NOWAIT) != -1 ) {
                if (strcmp(message_response.result,FAILURE) == 0)
                    puts("[KLIENT] Error: Grupa jest pusta - brak zalogowanych uzytkowników");
            }

            struct GroupsListResponse groups_list_response;
            if( msgrcv(userData.ID_USER,&groups_list_response, sizeof(groups_list_response),SERVER_GROUPS_RESPONSE,IPC_NOWAIT) != -1 ) {
                puts("\n[KLIENT] Dostępne grupy:");
                printf("%s\n\n", groups_list_response.groups);
            }

            struct JoinGroupResponse join_group_response;
            if( msgrcv(userData.ID_USER,&join_group_response, sizeof(join_group_response),SERVER_JOIN_RESPONSE,IPC_NOWAIT) != -1 ) {
                if (strcmp(join_group_response.result,SUCCESS) == 0)
                    puts("[KLIENT] Pomyślnie dołączono do grupy");
                else puts("[KLIENT] Error: Wystąpił błąd podczas dołączania do grupy");
            }

            struct LeaveGroupResponse leave_group_response;
            if( msgrcv(userData.ID_USER,&leave_group_response, sizeof(leave_group_response),SERVER_LEAVE_RESPONSE,IPC_NOWAIT) != -1 ) {
                if (strcmp(leave_group_response.result,SUCCESS) == 0)
                    puts("[KLIENT] Pomyślnie opuszczono grupę");
                else puts("[KLIENT] Error: Wystąpił błąd podczas opuszczania grupy");
            }

            struct GroupUsersResponse group_users_response;
            if( msgrcv(userData.ID_USER,&group_users_response, sizeof(group_users_response),SERVER_GROUP_USERS_RESPONSE,IPC_NOWAIT) != -1 ) {
                if (strcmp(group_users_response.result,SUCCESS) == 0) {
                    if (strcmp(group_users_response.users,"") != 0) {
                        puts("\n[KLIENT] Użytkownicy w grupie:");
                        printf("%s\n\n", group_users_response.users);
                    } else {
                        puts("[KLIENT] Do grupy nie należy żaden użytkownik");
                    }
                } else puts ("[KLIENT] Error: Wystąpił błąd podczas wyświetlania użytkowników");
            }
        }
    } else {
        while(1) { //OBSŁUGA STDIN
            char text[300] = "";
            char username[30] = "";
            fgets(text,300,stdin);
            if ( strcmp(text,"/logout\n") == 0 ){     //LOGOUT
                wylogowanie(userData.username,ID_LOG,PID);
                execute_order_66();
            }
            else if ( strcmp(text,"/help\n") == 0 ){      // HELP
                help();
            }
            else if ( strcmp(text,"/show_users\n") == 0 ){   // SHOW_USERS
                wyswietl_zalogowanych(userData.username,ID_LOG);
            }
            else if ( strcmp(text,"/show_groups\n") == 0 ){   // SHOW_GROUPS
                wyswietl_grupy(userData.username,ID_LOG);
            }
            else if ( strcmp(text,"/show_group_users\n") == 0 ){   // SHOW_GROUP_USERS
                puts("[KLIENT] Podaj nazwę grupy:");
                char grupa[30];
                scanf("%s",grupa);
                wyswietl_czlonkow(userData.username,ID_LOG,grupa);
            }
            else if ( strcmp(text,"/join_group\n") == 0 ){   // JOIN_GROUP
                puts("[KLIENT] Podaj nazwę grupy:");
                char grupa[30];
                scanf("%s",grupa);
                dolacz_grupa(userData.username,ID_LOG,grupa);
            }
            else if ( strcmp(text,"/leave_group\n") == 0 ){   // LEAVE_GROUP
                puts("[KLIENT] Podaj nazwę grupy:");
                char grupa[30];
                scanf("%s",grupa);
                opusc_grupa(userData.username,ID_LOG,grupa);
            }
            else if ( strcmp(text,"") != 0 && text[0]!='\n'){
                int i=0;

                while ( (text[i] != ':' ) & ( text[i] != '\0' ) ) {
                    username[i] = text[i];
                    i++;
                }
                if (text[i] != '\0') {
                    char *msg;
                    msg = strrchr(text, ':');
                    send_msg(username, msg, ID_LOG, userData.username);
                } else puts("[KLIENT] Wybrano niepoprawną opcję");
            }




        }
    }
}
