#include "common.h"


void help() {
    puts("\n------------------------------------");
    puts("Dostępne opcje:");
    puts("/help - wyświetl pomoc");
    puts("/show_users - wyświetl wszystkich zalogowanych użytkowników");
    puts("/logout - wyloguj");
    puts("/exit -zakończ");
    puts("------------------------------------");
    puts("\nAby wysłać wiadomość:");
    puts("'adresat': 'treść wiadomości[200]'");
    puts("np.: dbrzez: Witaj!\n");
    puts("------------------------------------\n");
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

void send_msg(char receiver[30],char* msg,int ID_LOG,char sender[30]) {
    struct Message message;
    message.type = CLIENT_MSG_REQUEST;
    strcpy(message.receiver,receiver);
    strcpy(message.sender,sender);
    strcpy(message.message,msg);
    msgsnd(ID_LOG,&message, sizeof(message),0);
}


int main(int argc, char* argv[]) {
    puts("\n[KLIENT] Witaj w Short Message Service!");
    int ID_LOG = msgget(server_msg_queue_key,0600);
    int PID = getpid();
    int username = 0;
    struct user_data userData = logowanie(ID_LOG,PID);

    if (!fork()) {
        while(1) { //NASŁUCHIWANIE PRYWATNEJ KOLEJKI
            struct UsersListResponse show_users_response;
            if( msgrcv(userData.ID_USER,&show_users_response, sizeof(show_users_response),SERVER_LOGGED_USERS_RESPONSE,IPC_NOWAIT) != -1 ) {
                puts("\n[KLIENT] Zalogowani użytkownicy:");
                printf("%s\n", show_users_response.users);
            }

            struct cmd_msg cmd_msg;
            if( msgrcv(userData.ID_USER,&cmd_msg, sizeof(cmd_msg),SERVER_MSG_GET,IPC_NOWAIT) != -1 ) {
                printf("%s", cmd_msg.message);
            }

        }
    } else {
        while(1) { //OBSŁUGA STDIN
            char text[300] = "";
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
            else if ( strcmp(text,"") != 0 && text[0]!='\n'){
                char username[30];
                int i=0;
                while (text[i]!=':') {
                    username[i] = text[i];
                    i++;
                }
                char *msg;
                msg = strrchr(text,':');
                send_msg(username,msg,ID_LOG,userData.username);

            }




        }
    }
}
