#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <time.h>
#include "arena.h"

#define MAX_USERS 100
#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_SIZE 256

#define SHM_KEY_USERS 5678
#define SHM_KEY_COUNT 5679
#define MSG_KEY 1234

typedef struct {
    char username[50];
    char password[50];
    int login;
    int gold;
    int lvl;
    int xp;
    int weapon;
} Users;

typedef struct {
    long msg_type;
    char msg_text[MAX_SIZE];
} MsgBuffer;

Users *users;
int *user_count;
int msgid;

void log_event(const char *text) {
    MsgBuffer msg;
    msg.msg_type = 99;
    strcpy(msg.msg_text, text);
    msgsnd(msgid, &msg, sizeof(msg.msg_text), 0);
}

int find_user(char *name) {
    for (int i = 0; i < *user_count; i++) {
        if (strcmp(users[i].username, name) == 0)
            return i;
    }
    return -1;
}

int name_check(char *name) {
    return find_user(name) == -1;
}

void pilihan(int sock, Users *u) {
    while (1) {
        char msg[BUFFER_SIZE];

        sprintf(msg,
            "\n====== BATTLE OF ETERION ======\n"
            "Name : %s\n"
            "Lvl  : %d\n"
            "Gold : %d\n"
            "XP   : %d\n"
            "\n1. Battle\n"
            "2. Armory\n"
            "3. History\n"
            "4. Logout\n",
            u->username, u->lvl, u->gold, u->xp);

        send(sock, msg, strlen(msg), 0);

        char choice[10] = {0};
        read(sock, choice, sizeof(choice));

        int c = atoi(choice);

        if (c == 1) battle_menu(sock, u);
        else if (c == 2) armory(sock, u);
        else if (c == 3) history(sock);
        else if (c == 4) {
            u->login = 0;
            break;
        }
    }
}

int main() {
    srand(time(NULL));

    int shmid_users = shmget(SHM_KEY_USERS, sizeof(Users) * MAX_USERS, 0666 | IPC_CREAT);
    int shmid_count = shmget(SHM_KEY_COUNT, sizeof(int), 0666 | IPC_CREAT);

    users = shmat(shmid_users, NULL, 0);
    user_count = shmat(shmid_count, NULL, 0);

    if (*user_count < 0 || *user_count > MAX_USERS) {
        *user_count = 0;
        memset(users, 0, sizeof(Users) * MAX_USERS);
    }

    msgid = msgget(MSG_KEY, 0666 | IPC_CREAT);

    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 10);

    printf("Orion is ready (PID: %d)\n", getpid());

    while (1) {
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);

        char buffer[BUFFER_SIZE] = {0};
        read(new_socket, buffer, BUFFER_SIZE);

        if (strncmp(buffer, "REGISTER", 8) == 0) {
            char user[50], pass[50];
            sscanf(buffer, "REGISTER:%[^:]:%s", user, pass);

            if (!name_check(user)) {
                send(new_socket, "Username already exists", 23, 0);
            } else {
                strcpy(users[*user_count].username, user);
                strcpy(users[*user_count].password, pass);
                users[*user_count].login = 0;
                users[*user_count].gold = 150;
                users[*user_count].lvl = 1;
                users[*user_count].xp = 0;
                users[*user_count].weapon = 0;

                (*user_count)++;

                send(new_socket, "Account created!", 16, 0);

                char logmsg[100];
                sprintf(logmsg, "%s registered", user);
                log_event(logmsg);
            }
        }


        else if (strncmp(buffer, "LOGIN", 5) == 0) {
            char user[50], pass[50];
            sscanf(buffer, "LOGIN:%[^:]:%s", user, pass);

            int idx = find_user(user);

            if (idx != -1 && strcmp(users[idx].password, pass) == 0) {
                users[idx].login = 1;

                send(new_socket, "Welcome", 8, 0);
                pilihan(new_socket, &users[idx]);
            } else {
                send(new_socket, "Login failed", 12, 0);
            }
        }

        close(new_socket);
    }

    return 0;
}
