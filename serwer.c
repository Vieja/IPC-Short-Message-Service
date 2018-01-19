#include "common.h"


char** czytaj_plik(int ktory) {

    int plik = open("users.txt",O_RDONLY);

    char** tab;
    tab = malloc(sizeof(char*) * 9);
    for (int i=0;i<9;i++) {
        tab[i] = malloc(sizeof(char*) * 30);
    }
    int x = 0;
    char znak;
    int i=0;

    for(int j=0;j<9;j++) {

        read(plik,&znak,1);
        while (znak!=';') {
            if (ktory == 0 ) tab[i][x] = znak;
            x++;
            read(plik,&znak,1);
        }
        //if (ktory == 0) tab[i][x]='\n';
        x=0;
        read(plik,&znak,1);
        while (znak!='\n') {
            if (ktory == 1 ) tab[i][x] = znak;
            x++;
            read(plik,&znak,1);
        }
        //if (ktory == 1) tab[i][x]='\n';
        x=0;
        i++;
    }
    return tab;
}

int find_field(char username[30],char **tab) {
    char username2[30];
    int i;
    for (i = 0; i < 9; i++) {
        strcpy(username2, tab[i]);
        if (strcmp(username, username2) == 0) break;
    }
    return i;
}


int main(int argc, char* argv[]) {
    puts("[SERWER] Uruchamianie serwera...");

    int ID_USERS[9];
    int PID_USERS[9];
    //printf("Możliwe loginy i hasła:\nAli  ali\nB   b\nC   c\n");

    char **loginy = czytaj_plik(0);
    char **hasla  = czytaj_plik(1);
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
                int i = find_field(login_request.username,loginy);
                int j = find_field(login_request.password,hasla);
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
                int i = find_field(logout_request.username,loginy);
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
                int i = find_field(show_users_request.username,loginy);
                struct UsersListResponse show_users_response;
                show_users_response.type = SERVER_LOGGED_USERS_RESPONSE;
                strcpy(show_users_response.users,"");
                if (i!=9) {  //POPRAWNY LOGIN
                    puts("[SERWER] Success: Login prawidłowy");
                    for (int j=0;j<9;j++) {
                        if (ID_USERS[j] != 0) {
                            strcat(show_users_response.users, loginy[j]);
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
                //struct
                int i = find_field(message.sender,loginy);
                int j = find_field(message.receiver,loginy);
                struct cmd_msg cmd_msg;
                if (j != 9) { // JEŻELI ODBIORCA ISTNIEJE
                    cmd_msg.type=SERVER_MSG_GET;
                    strcpy(cmd_msg.message,message.message);
                    puts("<sending cmd_msg>");
                    if ( msgsnd(ID_USERS[j],&cmd_msg, sizeof(cmd_msg),0) != -1) { //JEŻELI WYSYŁANIE SIE POWIODŁO
                        puts("[SERWER] Success: przekierowano wiadomość");
                    } else {
                        puts("[SERWER] Error: nir przekierowano wiadomości");
                    }
                } else {
                    puts("[SERWER] Error: nieprawidłowy odbiorca");
                }
            }







    }
}
