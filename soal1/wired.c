#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "protocol.h"

typedef struct {
       int socket;
       char username[50];
       int admin;
} Client;

Client clients[MAX_CLIENTS];
int client_count = 0;
time_t start_time;

void log_history(char *role, char *message) {
    FILE *f = fopen("history.log", "a");
    if (!f) return;

    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    fprintf(f, "[%04d-%02d-%02d %02d:%02d:%02d] [%s] %s\n",
        t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec, role, message);

    fclose(f);
}

int name_check(char *name) {
    for (int i = 0; i < client_count; i++) {
        if (strcmp(clients[i].username, name) == 0) return 0;
    }
    return 1;
}

void broadcast(char *message, int client_index) {
    for (int i = 0; i < client_count; i++) {
        if(i != client_index) {
	   send(clients[i].socket, message, strlen(message), 0);
        }
    }
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    start_time = time(NULL);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));

    listen(server_fd, 10);

    log_history("System", "[SERVER ONLINE]");
    printf("Server running on port %d...\n", PORT);

    while(1) {
	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(server_fd, &readfds);

	int max_sd = server_fd;

	for (int i = 0; i < client_count; i++) {
	     FD_SET(clients[i].socket, &readfds);
	     if (clients[i].socket > max_sd)
	         max_sd = clients[i].socket;
	}

	select(max_sd + 1, &readfds, NULL, NULL, NULL);

    if (FD_ISSET(server_fd, &readfds)) {
	new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);

    char username[50];
    while(1) {
	memset(username, 0, sizeof(username));
	int len = recv(new_socket, username, sizeof(username)-1, 0);
	if(len <= 0) break;

	username[len] = '\0';
	username[strcspn(username, "\n")] = 0;
	if (!name_check(username)) {
	    char message[200];
 	    sprintf(message, "[System] The identity '%s' is already synchronized in The Wired.\n", username);
	    send(new_socket, message, strlen(message), 0);
	}
	else
	{
	    break;
	}
    }

    int admin = 0;

    if (strcmp(username, "The Knights") == 0) {
         char pass[50];

         while (1) {
            char *ask = "Enter Password: ";
            send(new_socket, ask, strlen(ask), 0);

	    int len = recv(new_socket, pass, sizeof(pass)-1, 0);
            pass[len] = '\0';
            pass[strcspn(pass, "\n")] = 0;

            if (strcmp(pass, ADMIN_PASS) == 0) {
		admin = 1;

                char *success = "[System] Authentication Successfull. Granted Admin previleges.\n";
                send(new_socket, success, strlen(success), 0);

		char *menu =
		"\n=== THE KNIGHTS CONSOLE ===\n"
		"1. Check Active Entites (Users)\n"
		"2. Check Server Uptime\n"
		"3. Execute Emergency Shutdown\n"
		"4. Disconnect\n"
		"Command >> ";
		send(new_socket, menu, strlen(menu), 0);

                break;
            } else {
                char *fail = "[System] Wrong password. Try again.\n";
                send(new_socket, fail, strlen(fail), 0);
            }
        }
    }


    clients[client_count].socket = new_socket;
    strcpy(clients[client_count].username, username);
    clients[client_count].admin = admin;
    client_count++;

    char msg[200];
    sprintf(msg, "--- Welcome to The Wired, %s ---\n", username);
    send(new_socket, msg, strlen(msg), 0);

    char logbuf[100];
    sprintf(logbuf, "[User '%s' connected]", username);
    log_history("System", logbuf);

    printf("%s connected\n", username);
    }

    for (int i = 0; i < client_count; i++) {
        int sd = clients[i].socket;

        if (FD_ISSET(sd, &readfds)) {
            char buffer[512] = {0};
            int valread = read(sd, buffer, sizeof(buffer)-1);
	    buffer[valread] = '\0';

            if (valread <= 0) {
                char logbuf[100];
                sprintf(logbuf, "[User '%s' disconnected]", clients[i].username);
                log_history("System", logbuf);

                close(sd);
                clients[i] = clients[client_count - 1];
                client_count--;
                i--;
             } else {
                buffer[strcspn(buffer, "\n")] = 0;

                if (strcmp(buffer, "/exit") == 0) {
                    char logbuf[100];
                    sprintf(logbuf, "[User '%s' disconnected]", clients[i].username);
                    log_history("System", logbuf);

                    close(sd);
                    clients[i] = clients[client_count - 1];
                    client_count--;
                    i--;
                    continue;
                }

		if (clients[i].admin) {
        	if (strcmp(buffer, "1") == 0) {
		    log_history("Admin", "[RPC_GET_USERS]");
            	    char message[512] = "Active users:\n";
                    for (int j = 0; j < client_count; j++) {
                	strcat(message, clients[j].username);
                	strcat(message, "\n");
            	    }
            	    send(sd, message, strlen(message), 0);
            	    continue;
        	}
        	else if (strcmp(buffer, "2") == 0) {
		    log_history("Admin", "[RPC_GET_UPTIME]");
            	    char message[100];
            	    sprintf(message, "Uptime: %ld seconds\n", time(NULL) - start_time);
            	    send(sd, message, strlen(message), 0);
            	    continue;
        	}
       		else if (strcmp(buffer, "3") == 0) {
            	    log_history("Admin", "[RPC_SHUTDOWN]");
            	    exit(0);
        	}
    	    }

            char message[600];
            sprintf(message, "[[%s]: %s]\n", clients[i].username, buffer);

            printf("%s", message);
            log_history("User", message);
            broadcast(message, i);
         }
       }
     }
  }

  return 0;
}
