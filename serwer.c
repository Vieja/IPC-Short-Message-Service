#include "common.h"

struct tablice {
    char **loginy;
    char **hasla;
    char **grupy;
    char ***grupy_users;
};

struct tablice czytaj_plik() {

    int plik = open("users.txt",O_RDONLY);
    struct tablice tablice;
    tablice.loginy = malloc(sizeof(char*) * 9);
    tablice.hasla = malloc(sizeof(char*) * 9);
    tablice.grupy = malloc(sizeof(char*) * 3);
    tablice.grupy_users = malloc(sizeof(char*) * 3);
    for (int i=0;i<9;i++) {
        tablice.loginy[i] = malloc(sizeof(char*) * 30);
        tablice.hasla[i] = malloc(sizeof(char*) * 30);
    }
    for (int i=0;i<3;i++) {
        tablice.grupy[i] = malloc(sizeof(char*) * 30);
        tablice.grupy_users[i] = malloc(sizeof(char*) * 9);
    }
    for (int i=0;i<3;i++) {
        for (int j=0;j<9;j++) {
            tablice.grupy_users[i][j] = malloc(sizeof(char*) * 30);
        }
    }

    int x = 0;
    char znak;
    int i=0;

    for(int j=0;j<9;j++) {

        read(plik,&znak,1);
        while (znak!=';') {
            tablice.loginy[i][x] = znak;
            x++;
            read(plik,&znak,1);
        }
        x=0;
        read(plik,&znak,1);
        while (znak!='\n') {
            tablice.hasla[i][x] = znak;
            x++;
            read(plik,&znak,1);
        }
        x=0;
        i++;
    }
    i=0;
    for(int j=0;j<3;j++) {

        read(plik,&znak,1);
        while(znak!=';') {
            tablice.grupy[i][x] = znak;
            x++;
            read(plik,&znak,1);
        }
        x=0;
        i++;
    }
    strcpy(tablice.grupy_users[0][0],tablice.loginy[1]);
    strcpy(tablice.grupy_users[0][1],tablice.loginy[5]);

    return tablice;
}

int find_field(char username[30],char **tab,int size) {
    char username2[30];
    int i;
    for (i = 0; i < size; i++) {
        strcpy(username2, tab[i]);
        if (strcmp(username, username2) == 0) break;
    }
    return i;
}


int main(int argc, char* argv[]) {
    puts("[SERWER] Uruchamianie serwera...");

    int ID_USERS[9];
    int PID_USERS[9];

    struct tablice tablice;
    tablice = czytaj_plik();
    puts("[SERWER] Dane z pliku pomyślnie załadowane");
		int queueID = msgget(server_msg_queue_key, 777);
		msgctl(queueID, IPC_RMID, 0);


    int ID_LOG = msgget(server_msg_queue_key,IPC_CREAT|0600);
    for (int i=0;i<9;i++) {
        ID_USERS[i]=0;
        PID_USERS[i]=0;
    }

    puts("[SERWER] Serwer aktywny");
    while (1) {
        struct LoginRequest login_request; //LOGOWANIE
            if ( msgrcv(ID_LOG, &login_request, sizeof(login_request), CLIENT_LOGIN_REQUEST, IPC_NOWAIT) != -1 ) {
                puts("[SERWER] <login_request received>");
                int i = find_field(login_request.username,tablice.loginy,9);
                int j = find_field(login_request.password,tablice.hasla,9);
                struct LoginResponse login_response;
                int PID = login_request.pid;
                login_response.type = PID;
                strcpy(login_response.username, login_request.username);
                if (i == j & i != 9) {
                    puts("[SERWER] Success: dane logowania poprawne");
                    //POPRAWNE DANE LOGOWANIA
                    if (ID_USERS[i] != 0) {
                        //UŻYTKOWNIK JUŻ ZALOGOWANY
                        puts("[SERWER] Error: użytkownik już zalogowany");
                        strcpy(login_response.result, FAILURE);
                    } else { //POPRAWNE ZALOGOWANIE
                        ID_USERS[i] = msgget(PID, IPC_CREAT | 0600);
                        PID_USERS[i] = PID;
                        // printf("PID: %d\n\n",PID);
                        puts("[SERWER] Success: użytkownik zalogowany");
                        strcpy(login_response.result, SUCCESS);
                    }
                } else {//NIEPOPRAWNY LOGIN I/LUB HASŁO
                    puts("[SERWER] Error: niepoprawny login i/lub hasło");
                    strcpy(login_response.result, FAILURE);
                }
                //ODPOWIEDZ SERWERA NA PRÓBĘ LOGOWANIA 
                puts("[SERWER] <sending login_response>");
                msgsnd(ID_LOG, &login_response, sizeof(login_response), 0);
                //printf("%d\n",ID_USERS[0]);
            }

        struct LogoutRequest logout_request; //WYLOGOWANIE
            if ( msgrcv(ID_LOG,&logout_request, sizeof(logout_request),CLIENT_LOGOUT_REQUEST,IPC_NOWAIT) != -1) {
                puts("[SERWER] <logout_request received>");
                int i = find_field(logout_request.username,tablice.loginy,9);
                struct LogoutResponse logout_response;
                if (i != 9) { //POPRAWNY LOGIN
                    strcpy(logout_response.result,SUCCESS);
                    logout_response.type = PID_USERS[i];
                    if ( msgctl(ID_USERS[i],IPC_RMID,0) != -1){  //POMYŚLNIE USUNIĘTO PRYWATNĄ KOLEJKĘ
                        puts("[SERWER] Success: wylogowano użytkownika");
                        ID_USERS[i]=0;
                        PID_USERS[i]=0;
                    } else { //NIE UDAŁO SIĘ USUNĄĆ KOLEJKI
                        puts("[SERWER] Error: Wystąpił błąd podczas usuwania kolejki");
                        strcpy(logout_response.result,FAILURE);
                    }
                } else {
                    puts("[SERWER] Error: Błędny login");
                    strcpy(logout_response.result,FAILURE);
                }
                puts("[SERWER] <sending logout_response>");
                msgsnd(ID_LOG,&logout_response,sizeof(logout_response),0);
            }

        struct UsersListRequest show_users_request;   //WYŚWIETLANIE ZALOGOWANYCH UŻYTKOWNIKÓW
            if ( msgrcv(ID_LOG,&show_users_request, sizeof(show_users_request),CLIENT_LOGGED_USERS_REQUEST,IPC_NOWAIT) != -1) {
                puts("[SERWER] <show_users_request received>");
                int i = find_field(show_users_request.username,tablice.loginy,9);
                struct UsersListResponse show_users_response;
                show_users_response.type = SERVER_LOGGED_USERS_RESPONSE;
                strcpy(show_users_response.users,"");
                if (i!=9) {  //POPRAWNY LOGIN
                    puts("[SERWER] Success: Login prawidłowy");
                    for (int j=0;j<9;j++) {
                        if (ID_USERS[j] != 0) {
                            strcat(show_users_response.users, tablice.loginy[j]);
                            strcat(show_users_response.users,";");
                        }
                    }
                } else {
                    puts("[SERWER] Error: Błędny login");
                }
                puts("[SERWER] <sending show_users_response>");
                msgsnd(ID_USERS[i],&show_users_response, sizeof(show_users_response),0);
            }

        struct Message message;  //  PRZEKAZYWANIE WIADOMOŚCI
            if ( msgrcv(ID_LOG,&message, sizeof(message),CLIENT_MSG_REQUEST,IPC_NOWAIT) != -1) {
                puts("[SERWER] <message received>");
                struct MessageResponse message_response;
                int i = find_field(message.sender,tablice.loginy,9);
                int j = find_field(message.receiver,tablice.loginy,9);
                message_response.type=SERVER_MSG_RESPONSE;
                if (j != 9) { // JEŻELI ODBIORCA ISTNIEJE
                    message.type = SERVER_MSG_GET;
                    puts("[SERWER] <sending message>");
                    if ( msgsnd(ID_USERS[j],&message, sizeof(message),0) != -1) { //JEŻELI WYSYŁANIE SIE POWIODŁO
                        puts("[SERWER] Success: przekierowano wiadomość");
                        strcpy(message_response.result,SUCCESS);
                    } else {
                        puts("[SERWER] Error: nie przekierowano wiadomości");
                        strcpy(message_response.result,FAILURE);
                    }
                } else {
                    puts("[SERWER] Error: nieprawidłowy odbiorca");
                    strcpy(message_response.result,FAILURE);
                }
                puts("[SERWER] <sending message_respose>");
                msgsnd(ID_USERS[i],&message_response,sizeof(message_response),0);
            }

        struct GroupsListRequest groups_list_request;  //  WYŚWIETLANIE GRUP
        if ( msgrcv(ID_LOG,&groups_list_request, sizeof(groups_list_request),CLIENT_GROUPS_REQUEST,IPC_NOWAIT) != -1) {
            puts("[SERWER] <groups_list_request received>");
            struct GroupsListResponse groups_list_response;
            int i = find_field(groups_list_request.username, tablice.loginy,9);
            groups_list_response.type = SERVER_GROUPS_RESPONSE;
            strcpy(groups_list_response.groups,"");
            for (int j = 0; j < 3; j++) {
                strcat(groups_list_response.groups, tablice.grupy[j]);
                strcat(groups_list_response.groups, ";");
            }
            puts("[SERWER] <sending groups_list_response>");
            msgsnd(ID_USERS[i], &groups_list_response, sizeof(groups_list_response), 0);
        }

        struct GroupUsersRequest group_users_request;  //  WYŚWIETLANIE UŻYTKOWNIKÓW GRUPY
        if ( msgrcv(ID_LOG,&group_users_request, sizeof(group_users_request),CLIENT_GROUP_USERS_REQUEST,IPC_NOWAIT) != -1) {
            puts("[SERWER] <group_users_request received>");
            struct GroupUsersResponse group_users_response;
            int i = find_field(group_users_request.username,tablice.loginy,9);
            int j = find_field(group_users_request.group, tablice.grupy,3);
            group_users_response.type = SERVER_GROUP_USERS_RESPONSE;
            strcpy(group_users_response.users,"");
            if (j != 3) {
                puts("[SERWER] Success: wyświetlanie użytkowników grupy");
                for (int k = 0; k < 9; k++) {
                    if (strcmp(tablice.grupy_users[j][k], "") != 0) {
                        strcat(group_users_response.users, tablice.grupy_users[j][k]);
                        strcat(group_users_response.users, ";");
                    }
                }
                strcpy(group_users_response.result,SUCCESS);
            } else {
                puts("[SERWER] Error: nie ma takiej grupy");
                strcpy(group_users_response.result,FAILURE);
            }
            puts("[SERWER] <sending group_users_response>");
            msgsnd(ID_USERS[i], &group_users_response, sizeof(group_users_response), 0);
        }


    }
}
