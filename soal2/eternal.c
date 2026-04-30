#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>

#define BUFFER_SIZE 1024
#define PORT 8080
#define IP "127.0.0.1"

void menu() {
printf(" _____   __ _______ ____ _   ____    ___ ____\n");
printf("|  _  \\ /  \\ _   _ |   __|| |  __|  / _ \\  __|\n");
printf("| |_) |  /\\ \\ | |   | | | | | |__  | | | ||_\n");
printf("|  _ <  /__\\ \\| |   | | | | |  __| | | | | _|\n");
printf("| |_) | ____  \\ |   | | | |_|_|__  | |_| ||\n");
printf("|_____//    \\__\\|   |_| |_____|__|  \\___/_|\n");
printf(" ____ ______ ___ ___ ______ ___ __     _\n");
printf("|  __|_   __| __| _ \\_   __| _ \\   \\  | |\n");
printf("| |_   | | | |_| |_) || | | | | |   \\ | |\n");
printf("|  _|  | | |  _|  _ < | | | | | | |\\ \\| |\n");
printf("| |__  | | | |_| | \\ \\| |_| |_| | | \\   |\n");
printf("|____| |_| |____||  \\_\\ ___|___/__|  \\__|\n");
printf("\n");
printf("1. Register\n");
printf("2. Login\n");
printf("3. Exit\n");
printf("Choice: ");
}

void pilihanEterion(int sock) {
    char buffer[BUFFER_SIZE];
    char input[100];

    while (1) {
        memset(buffer, 0, sizeof(buffer));

        int n = read(sock, buffer, sizeof(buffer) - 1);

        if (n <= 0) {
            printf("\nDisconnected from Orion.\n");
            break;
        }

        printf("%s", buffer);
        fflush(stdout);

        /* MAIN MENU ETERION */
        if (strstr(buffer, "BATTLE OF ETERION")) {
            printf("Choice: ");
            fgets(input, sizeof(input), stdin);
            input[strcspn(input, "\n")] = 0;

            send(sock, input, strlen(input), 0);

            if (strcmp(input, "4") == 0)
                break;

            continue;
        }

        /* BATTLE ACTION */
        if (strstr(buffer, "(a) Attack")) {
            printf("Action (a/u): ");
            fgets(input, sizeof(input), stdin);
            input[strcspn(input, "\n")] = 0;

            send(sock, input, strlen(input), 0);
            continue;
        }

        /* ARMORY */
        if (strstr(buffer, "=== Armory ===")) {
            printf("Choose weapon: ");
            fgets(input, sizeof(input), stdin);
            input[strcspn(input, "\n")] = 0;

            send(sock, input, strlen(input), 0);
            continue;
        }

        /* AUTO CONTINUE AFTER RESULT */
        if (strstr(buffer, "VICTORY") ||
            strstr(buffer, "DEFEAT") ||
            strstr(buffer, "Weapon equipped") ||
            strstr(buffer, "Gold not enough") ||
            strstr(buffer, "MATCH HISTORY")) {
            continue;
        }
    }
}

int connect_server() {
    int sock;
    struct sockaddr_in serv_addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    inet_pton(AF_INET, IP, &serv_addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Failed connect to Orion.\n");
        exit(1);
    }

    return sock;
}

int main() {
    while (1) {
        char username[50];
        char password[50];
        char message[200];
        char buffer[BUFFER_SIZE] = {0};

        int choice;

        menu();

        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n');
            continue;
        }

        while (getchar() != '\n');

        if (choice == 1) {
            int sock = connect_server();

            printf("\n=== CREATE ACCOUNT ===\n");

            printf("Username: ");
            fgets(username, sizeof(username), stdin);
            username[strcspn(username, "\n")] = 0;

            printf("Password: ");
            fgets(password, sizeof(password), stdin);
            password[strcspn(password, "\n")] = 0;

            snprintf(message, sizeof(message),
                     "REGISTER:%s:%s",
                     username, password);

            send(sock, message, strlen(message), 0);

            read(sock, buffer, BUFFER_SIZE);

            printf("\n%s\n\n", buffer);

            close(sock);
        }

        else if (choice == 2) {
            int sock = connect_server();

            printf("\n=== LOGIN ===\n");

            printf("Username: ");
            fgets(username, sizeof(username), stdin);
            username[strcspn(username, "\n")] = 0;

            printf("Password: ");
            fgets(password, sizeof(password), stdin);
            password[strcspn(password, "\n")] = 0;

            snprintf(message, sizeof(message),
                     "LOGIN:%s:%s",
                     username, password);

            send(sock, message, strlen(message), 0);

            memset(buffer, 0, BUFFER_SIZE);
            read(sock, buffer, BUFFER_SIZE);

            if (strstr(buffer, "Welcome")) {
                printf("\nLogin success!\n");
                pilihanEterion(sock);
            } else {
                printf("\n%s\n\n", buffer);
            }

            close(sock);
        }

        else if (choice == 3) {
            printf("\nSee you in another Battle of Eterion!\n");
            break;
        }

        else {
            printf("Invalid choice.\n");
        }
    }

    return 0;
}
