# SISOP-3-2026-IT-035

**Dikerjakan oleh: Dea Chrisna Butarbutar - 5027251035**

## Reporting

### Soal 1
**Present Day, Present Time**

#### Penjelasan
Langkah pertama adalah membuat file untuk server yaitu `wired.c` dan client yaitu `navi.c`. Untuk definisi berada di file `protocol.h`.
```bash
$ touch mavi.c wired.c protocol.h"
```

a. File `navi.c` (CLIENT)
1. Membuat koneksi yang stabil dan NAVI terdaftar di server The Wired.

```bash
if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
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
```

Di sini yang digunakan adalah socket programming (TCP) untuk memastikan koneksi yang stabil. Untuk membuat navi terkoneksi ke server The Wired, digunakan `connect()` melalui alamat dan PORT yang sudah didefinisikan di file `protocol.h`.

2. Navi menjalankan dua fungsi dengan asinkronisasi tanpa menggunakan fork.
Membuat thread baru yang menjalankan fungsi run untuk menerima pesan dari server.
```
pthread_t recv_thread;
pthread_create(&recv_thread, NULL, run, &sock);
```

Kemudian, untuk menerimanya digunakan fungsi run seperti ini:
```
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
```
Kode di atas adalah thread untuk menerima pesan dari server The Wired secara asinkronus. Penerimaan data dari socketnya adalah menggunakan `recv`. Kemudian, di terminal akan ditampilkan pesan yang diterima dari server.

3. Navi menginisialisasi identitas unik (nama) penggunanya dan memastikan tidak ada 2 atau lebih client dengan nama yang sama.
```
char buffer[1024];
char username[50];

    while (1) {
       printf("Enter your name: ");
       fgets(username, sizeof(username), stdin);
       username[strcspn(username, "\n")] = 0;
       send(sock, username, strlen(username), 0);
```
Pada kode di atas, client mengirim username ke server The Wired dari nama yang sudah diinput oleh user.

```

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
```
- Kemudian, pada kode di atas, client menerima pesan dari server terlebih dahulu. Lalu, semua respon dari server akan ditampikan ke semua user. Untuk mengecek sukses atau tidaknya login adalah dengan mengecek adanya kata "Welcome" di dalam buffer. 
- Untuk pengisian password, client mengecek apakah server meminta password dengan cara mengecek adanya kata "password". Setelah password selesai diinput, maka akan dikirim ke server.
- Selanjutnya, server akan mengecek password. Jika salah, akan diminta ulang. Kalo benar, maka akan keluar dari perulangan.

b. File `wired.c` (SERVER)
1. Penyimpanan data user dalam Client
```
typedef struct {
    int socket;
    char username[50];
    int admin;
} Client;

Client clients[MAX_CLIENTS];
int client_count = 0;
```
Pada kode di atas, server The Wired menyimpan semua user yang sudah login. Inisialisasi jumlah client dimulai dari 0. 


2. Untuk mencatat semua aktivitas server ke file log `history.log`. Digunakan waktu saat ini dengan `localtime(&now)`.
```
void log_history(char *role, char *message) {
    FILE *f = fopen("history.log", "a");
    if (!f) return;

    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    fprintf(f, "[%04d-%02d-%02d %02d:%02d:%02d] [%s] %s\n",
        t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec, role, message);

    fclose(f);
}
```


3. Fungsi Pengecekan Nama Unik
```
int name_check(char *name) {
    for (int i = 0; i < client_count; i++) {
        if (strcmp(clients[i].username, name) == 0) return 0;
    }
    return 1;
}
```
Kode di atas adalah membandingkan nama dengan strcmp dengan mengeloop semua user yang sedang aktif.

Username yang sudah diinput user dikirim oleh client ke server. Kemudian, server akan mengecek nama. Jika sudah ada, maka akan mengeluarkan output bahwa nama tersebut sudah ada. Jika belum ada, maka akan keluar dari loop.

Setelah itu, username tersebut akan disimpan dan berhasil aktif.
```
clients[client_count].socket = new_socket;
strcpy(clients[client_count].username, username);
clients[client_count].admin = admin;
client_count++;
```


4. Untuk mengirim pesan-pesan ke semua client (broadcast), digunakna fungsi broadcast sebagai berikut.
```
void broadcast(char *message, int client_index) {
    for (int i = 0; i < client_count; i++) {
        if(i != client_index) {
	   send(clients[i].socket, message, strlen(message), 0);
        }
    }
}
```


5. Selajutnya adalah inisialisasi server dengan socket dan membuat socketnya itu sendiri.
```
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
```

Untuk memberitahu bahwa server aktif, maka menggunakan fungsi log_history yang sudah dibuat tadi, yaitu sebagai berikut:
```
log_history("System", "[SERVER ONLINE]");
printf("Server running on port %d...\n", PORT);
```


6. Untuk menjalankan server, digunakan perulangan while loop kemudian semua client yang ada dideteksi oleh server.
```
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
```


7. Untuk pengecekan nama yang unik menggunakan fungsi check_name.
```
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
```


8. Pengecekan user yang login adalah "The Knights" atau bukan adalah dengan mengecek usernamenya.
 ```
if (strcmp(username, "The Knights") == 0)
```

Jika user adalah "The Knights", maka akan diminta untuk menginput password khusus "The Knights"
```
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
```


Setelah berhasil masuk sebagai "The Knights", maka semacam menu khusus The Knights akan ditampilkan. 


9. Server menyimpan informasi client yang baru terhubung ke dalam array. Data yang disimpan tersebut adalah socket, username, dan status dari The Knights.
```
clients[client_count].socket = new_socket;
strcpy(clients[client_count].username, username);
clients[client_count].admin = admin;
client_count++;
```
Kode di atas adalah untuk mengelola seluruh client yang aktif.


10. Setelah client berhasil terdaftar, server akan memberikan output sebagai tanda bahwa client telah berhasil masuk ke dalam sistem.
```
char msg[200];
sprintf(msg, "--- Welcome to The Wired, %s ---\n", username);
send(new_socket, msg, strlen(msg), 0);
```


11. Server mencatat aktivitas client yang baru terhubung ke dalam file log.
```
char logbuf[100];
sprintf(logbuf, "[User '%s' connected]", username);
log_history("System", logbuf);

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
```
Pada kode di atas, jika koneksi client terputus, server akan menghapus client dari daftar dan mengeluarkan output disconnected dan dicatat dalam log_history. 

Jika client mengirimkan perintah /exit, maka server akan memperlakukannya sebagai proses disconnect.
```
if (strcmp(buffer, "/exit") == 0) {
    close(sd);
    clients[i] = clients[client_count - 1];
    client_count--;
    i--;
}
```


12. Jika client memiliki status The Knights, maka server akan memproses perintah khusus dan dimasukkan dalam log_history.
```
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
```


13. Untuk menampilkan uptime admin, server menghitung lama waktu berjalan sejak dijalankan.
```
else if (strcmp(buffer, "2") == 0) {
		    log_history("Admin", "[RPC_GET_UPTIME]");
            	    char message[100];
            	    sprintf(message, "Uptime: %ld seconds\n", time(NULL) - start_time);
            	    send(sd, message, strlen(message), 0);
            	    continue;
```


14. Untuk mematikan server sehingga berhenti beroperasi.
```
else if (strcmp(buffer, "3") == 0) {
            	    log_history("Admin", "[RPC_SHUTDOWN]");
            	    exit(0);
        	}
```

15. Untuk menampilkan pesan yang dikirim client tersebut di terminal server.
```
  char message[600];
            sprintf(message, "[[%s]: %s]\n", clients[i].username, buffer);

            printf("%s", message);
            log_history("User", message);
            broadcast(message, i);
```
Kode di atas juga menyimpan log pesan ke dalam file log dan selanjutnya server mengirim pesan dari client ke semua client yang lain dengan fungsi broadcast.

Untuk semua definisi yang dipakai pada file navi.c dan wired.c diletakkan di dalam file protocol.h sebagai berikut.
```
#ifndef PROTOCOL_H
#define PROTOCOL_H

#define PORT 8080
#define MAX_CLIENTS 100

#define ADMIN_NAME "The Knights"
#define ADMIN_PASS "wired123"

#define BUFFER_SIZE 1024

#endif
```

### OUTPUT
1. Compile file navi.c dan wired.c, kemudian menjalankan wired
<img width="897" height="177" alt="image" src="https://github.com/user-attachments/assets/990f7f50-a9b4-4f09-bb27-87a189d636ae" />

2. Menjalankan navi
<img width="803" height="126" alt="image" src="https://github.com/user-attachments/assets/772c4e92-0f51-4d1d-a2f6-236b84252409" />

3. Menjalankan navi di terminal lain
<img width="812" height="451" alt="image" src="https://github.com/user-attachments/assets/605e25ec-9aad-4d7d-8a04-a0c99c55e4c0" />

4. Ketika satu user mengirim pesan, maka akan terlihat di terminal user lainnya
<img width="817" height="569" alt="image" src="https://github.com/user-attachments/assets/1b75e935-ce3b-469b-88ff-973e2c6bb9d8" />
<img width="811" height="647" alt="image" src="https://github.com/user-attachments/assets/7c3bbc4c-7b0a-4125-9331-3d884a6c4329" />

5. Ketika masuk sebagai admin
<img width="804" height="595" alt="image" src="https://github.com/user-attachments/assets/d3e34ad5-1801-430a-9092-aa35e7489ee8" />

6. Pilihan ./exit
<img width="470" height="91" alt="image" src="https://github.com/user-attachments/assets/85a9f27c-5309-4ee3-8e2a-3109c097cde2" />


### Soal 2
**The Battle of Eterion**

#### Penjelasan

Langkah pertama adalah membuat file untuk server yaitu `orion.c` dan client yaitu `eternal.c`. Untuk definisi berada di file `arena.h`.
```bash
$ touch mavi.c wired.c protocol.h"
```

1. Menu 
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
