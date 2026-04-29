#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#define PORT 8080

void *run(void *args) {
    int sock;
    sock = *((int *)args);
    char buffer[1024];

    while (1) {
      int valread = recv(sock, buffer, sizeof(buffer)-1, 0);
      if (valread <= 0) break;

      buffer[valread] = '\0';
      printf("%s", buffer);
    }

    return NULL;
}

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    char buffer[1024];
    char username[50];

    while (1) {
       printf("Enter your name: ");
       fgets(username, sizeof(username), stdin);
       username[strcspn(username, "\n")] = 0;
       send(sock, username, strlen(username), 0);

       int valread = recv(sock, buffer, sizeof(buffer) - 1, 0);
       buffer[valread] = '\0';

       printf("%s", buffer);

       if (strstr(buffer, "Welcome") != NULL) {
           break;
       }

       if (strstr(buffer, "password") != NULL) {
           char pass[50];
           fgets(pass, sizeof(pass), stdin);
           send(sock, pass, strlen(pass), 0);

	   valread = recv(sock, buffer, sizeof(buffer) - 1, 0);
	   buffer[valread] = '\0';
	   printf("%s", buffer);
       }
    }

    pthread_t recv_thread;
    pthread_create(&recv_thread, NULL, run, &sock);

    char message[1024];
    while (1) {
        fgets(message, sizeof(message), stdin);
        send(sock, message, strlen(message), 0);
        if (strncmp(message, "/exit", 5) == 0) {
            printf("[System] Disconnecting from The Wired...\n");
            break;
        }
    }

    close(sock);

    return 0;
}
