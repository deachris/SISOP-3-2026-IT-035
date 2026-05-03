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
$ touch orion.c eternal.c arena.h"
```

A. File Server
1. Program mendefinisikan struktur data user dan message queue yang diminta.
```
typedef struct {
    char username[50];
    char password[50];
    int login;
    int gold;
    int lvl;
    int xp;
    int weapon;
} Users;
```
Struktur Users ini digunakan untuk menyimpan data setiap user, meliputi username, password, status login, jumlah gold, level, XP, dan senjata yang digunakan nantinya.

Kemudian, terdaoat struktur untuk mengirim pesan ke message queue untuk logging.
```
typedef struct {
    long msg_type;
    char msg_text[MAX_SIZE];
} MsgBuffer;
```

Di sini, program juga menggunakan shared memory untuk menyimpan data dari user.
```
Users *users;
int *user_count;
```
Program menggunakan shared memory agar data user tetap tersimpan meskipun server berjalan terus atau digunakan di banyak proses.
`users` menunjuk ke array data user, sedangkan `user_count` berarti jumlah user yang terdaftar.

2. Selanjutnya terdapat fungsi untuk mencatat aktivitas menggunakan message queue.
```
void log_event(const char *text) {
    MsgBuffer msg;
    msg.msg_type = 99;
    strcpy(msg.msg_text, text);
    msgsnd(msgid, &msg, sizeof(msg.msg_text), 0);
}
```
Fungsi `log_event()` ini digunakan untuk mencatat aktivitas, contohnya seperti registrasi user.

Kemudian, fungsi ini sendiri nanti akan berguna untuk cek kesamaan nama.
```
int find_user(char *name) {
    for (int i = 0; i < *user_count; i++) {
        if (strcmp(users[i].username, name) == 0)
            return i;
    }
    return -1;
}
```
Fungsi find_user() digunakan untuk mencari user berdasarkan username. Jika ditemukan, fungsi mengembalikan index user, jika tidak ditemukan maka mengembalikan -1.

Implementasi dari fungsinya yaitu untuk mengecek ketersediaan username:
```
int name_check(char *name) {
    return find_user(name) == -1;
}
```
Fungsi ini digunakan saat registrasi untuk memastikan bahwa username belum digunakan oleh user lain.

3. Fungsi untuk menampilkan menu utama game (Eterion)
```
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
```
Pada kode di atas, fungsi tersebut digunakan untuk menampilkan menu utama Eterion ke client. Informasi seperti nama, level, gold, dan XP akan ditampilkan kepada user.

4. Program kemudian membaca pilihan user di dunia Eterion
```
char choice[10] = {0};
read(sock, choice, sizeof(choice));

int c = atoi(choice);
```
Server menerima input dari client berupa pilihan menu, kemudian mengubahnya menjadi integer menggunakan `atoi`.

5. Program menjalankan setiap pilihan sesuai yang diinput oleh user.
```
if (c == 1) battle_menu(sock, u);
else if (c == 2) armory(sock, u);
else if (c == 3) history(sock);
else if (c == 4) {
    u->login = 0;
    break;
}
```
Jika logout, status login user diubah menjadi 0 dan keluar dari loop.

6. Program menginisialisasi socket, shared memory dan message queue
```
int shmid_users = shmget(SHM_KEY_USERS, sizeof(Users) * MAX_USERS, 0666 | IPC_CREAT);
int shmid_count = shmget(SHM_KEY_COUNT, sizeof(int), 0666 | IPC_CREAT);

users = shmat(shmid_users, NULL, 0);
user_count = shmat(shmid_count, NULL, 0);

msgid = msgget(MSG_KEY, 0666 | IPC_CREAT);
server_fd = socket(AF_INET, SOCK_STREAM, 0);

int opt = 1;
setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

address.sin_family = AF_INET;
address.sin_addr.s_addr = INADDR_ANY;
address.sin_port = htons(PORT);

bind(server_fd, (struct sockaddr *)&address, sizeof(address));
listen(server_fd, 10);
```
Server membuat socket TCP, mengatur opsi agar port bisa digunakan kembali, lalu melakukan binding ke alamat dan port. Setelah itu server mulai menerima koneksi dari client (`listen`).

7. Server menerima koneksi dari client
```
new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
```
Server menerima koneksi dari client menggunakan accept(). Setiap client yang terhubung akan mendapatkan socket baru (new_socket).

```
char buffer[BUFFER_SIZE] = {0};
read(new_socket, buffer, BUFFER_SIZE);
```
Pada kode di atas, server membaca data yang dikirim oleh client yaitu perintah login atau register.

8. Server memproses pilihan user yaitu "Register" (pilihan 1)
```
if (strncmp(buffer, "REGISTER", 8) == 0) {
    char user[50], pass[50];
    sscanf(buffer, "REGISTER:%[^:]:%s", user, pass);
```
Server mengecek apakah request adalah register. Jika iya, username dan password diambil dari string menggunakan sscanf.

9. Setelah melakukan proses register, maka server akan menyimpan data user tersebut.
```
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
```
Jika username belum digunakan, server akan menyimpan data user baru dengan nilai default seperti gold, level, dan XP.

Kemudian, server mengirim respon register dan mencatat log:
```
send(new_socket, "Account created!", 16, 0);

char logmsg[100];
sprintf(logmsg, "%s registered", user);
log_event(logmsg);
```
Server mengirim pesan bahwa akunnya sudah berhasil dibuaat ke client dan mencatat aktivitas registrasi ke message queue.

10. Jika pilihan user adalah untuk login (pilihan 2).
```
else if (strncmp(buffer, "LOGIN", 5) == 0) {
    char user[50], pass[50];
    sscanf(buffer, "LOGIN:%[^:]:%s", user, pass);

    int idx = find_user(user);
```
Server mengecek apakah yang diinput user adalah login, lalu mengambil username dan password.

11. Di sini, server akan mengelola proses login.
```
if (idx != -1 && strcmp(users[idx].password, pass) == 0) {
    users[idx].login = 1;

    send(new_socket, "Welcome", 8, 0);
    pilihan(new_socket, &users[idx]);
} else {
    send(new_socket, "Login failed", 12, 0);
}
sloce(socket);
```
Jika usernamenya ditemukan dan password sesuai, maka login berhasil. Server akan mengeluarkan output "Welcome" dan memanggil fungsi pilihan() untuk memulai gamenya. Jika gagal, server mengirim pesan error.

B. File Client
1. Menu
```
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
```

2. Fungsi `pilihanEterion` untuk menu Battle of Eterion
a. Untuk menerima data dari server dan menampilkannya ke user
```
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
```

3. Program membaca pesan dari server dengan perulangan loop.
```
while (1) {
    memset(buffer, 0, sizeof(buffer));

    int n = read(sock, buffer, sizeof(buffer) - 1);

    if (n <= 0) {
        printf("\nDisconnected from Orion.\n");
        break;
    }

    printf("%s", buffer);
    fflush(stdout);
```
Program menggunakan while loop untuk terus membaca pesan dari server. Fungsi read() sendiri digunakan untuk menerima data. Jika koneksi terputus (n <= 0), maka loop dihentikan. Semua pesan dari server langsung ditampilkan di terminal.

4. Program memproses menu utama dunia Eterion
if (strstr(buffer, "BATTLE OF ETERION")) {
    printf("Choice: ");
    fgets(input, sizeof(input), stdin);
    input[strcspn(input, "\n")] = 0;

    send(sock, input, strlen(input), 0);

    if (strcmp(input, "4") == 0)
        break;

    continue;
}

Pada kode di atas, jika pesan dari server terdapat "BATTLE OF ETERION", maka client mengetahui bahwa server sedang menampilkan menu utama, sehingga user diminta memasukkan pilihan menu, kemudian input dikirim ke server. Jika user memilih keluar yaitu opsi 4, maka loop akan dihentikan.

5. Program memproses aksi battle
```
if (strstr(buffer, "(a) Attack")) {
    printf("Action (a/u): ");
    fgets(input, sizeof(input), stdin);
    input[strcspn(input, "\n")] = 0;

    send(sock, input, strlen(input), 0);
    continue;
}
```
Jika client mendeteksi bahwa user sedang berada dalam battle of eterion, maka user diminta memilih action, yaitu attack atau ultimate. Input dari user tersebut kemudian dikirim ke server untuk diproses.

6. Untuk Armory
```
	if (strstr(buffer, "=== Armory ===")) {
    printf("Choose weapon: ");
    fgets(input, sizeof(input), stdin);
    input[strcspn(input, "\n")] = 0;

    send(sock, input, strlen(input), 0);
    continue;
}
```
Ketika user berada di armory, client meminta user memilih senjata. Pilihan tersebut kemudian dikirim ke server untuk diproses pembeliannya atau penggunaan senjatanya.

7. Fungsi untuk menghubungkan server dengan client.
```
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
```
Fungsi connect_server() digunakan untuk membuat koneksi ke server. Alamat server diatur menggunakan IP dan port yang sudah didefinisikan sebelumnya. Fungsi connect() digunakan untuk menghubungkan client ke server. Jika gagal, program akan dihentikan.

8. Untuk menunjukkan menu, digunakan perulangan while loop sebagai berikut.

```
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
```
Program membaca input user menggunakan scanf. Jika input tidak valid, buffer dibersihkan agar tidak terjadi error.

9. Untuk pilihan pertama, program akan melakukan proses register user.
```
if (choice == 1) {
    int sock = connect_server();

    printf("\n=== CREATE ACCOUNT ===\n");

    printf("Username: ");
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = 0;

    printf("Password: ");
    fgets(password, sizeof(password), stdin);
    password[strcspn(password, "\n")] = 0;
```
Jika user memilih register, program membuat koneksi ke server, lalu meminta username dan password dari user tersebut.

10. Untuk menyimpan data user yang sudah melakukan register.
```
snprintf(message, sizeof(message),
         "REGISTER:%s:%s",
         username, password);

send(sock, message, strlen(message), 0);

read(sock, buffer, BUFFER_SIZE);
```
Data dikirim ke server dalam format seperti yang tertera kemudian client menunggu respon dari server.

11. Setelah melakukan registrasi, maka program akan menampilkannya.
```
printf("\n%s\n\n", buffer);
close(sock);
```
Respon dari server ditampilkan ke terminal usernya, lalu koneksi ditutup.

12. Untuk pilihan 2, yaitu login pengguna.
```
else if (choice == 2) {
    int sock = connect_server();

    printf("\n=== LOGIN ===\n");

    printf("Username: ");
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = 0;

    printf("Password: ");
    fgets(password, sizeof(password), stdin);
    password[strcspn(password, "\n")] = 0;
```
Ketika user memilih login, program akan membuat koneksi ke server dan meminta username serta password dari user tersebut. 

Setelah itu, program akan mengirim data login ini ke terminal.
```
snprintf(message, sizeof(message),
         "LOGIN:%s:%s",
         username, password);

send(sock, message, strlen(message), 0);

memset(buffer, 0, BUFFER_SIZE);
read(sock, buffer, BUFFER_SIZE);
```
Data login dikirim ke server, lalu client membaca respon dari server tersebut di terminal.

Setelah itu, program akan mengeluarkan output:
```
if (strstr(buffer, "Welcome")) {
    printf("\nLogin success!\n");
    pilihanEterion(sock);
} else {
    printf("\n%s\n\n", buffer);
}
```
Pada kode di atas, jika login berhasil, client pun masuk ke dunia Eterion. Jika gagal, program akan menampilkan pesan bahwa ia error.

13. Untuk pilihan 3 adalah berhenti dari program.
```
else if (choice == 3) {
    printf("\nSee you in another Battle of Eterion!\n");
    break;
}
```
Di sini, jika user memilih exit, program akan berhenti.

Kemudian jika pilihan user adalah selain dari pilihan 1, 2, dan 3, maka program akan mengeluarkan output invalid.
```
else {
    printf("Invalid choice.\n");
}
```

### OUTPUT
