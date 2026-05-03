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

2. Fungsi Pengecekan Nama Unik
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

Setelah itu, username tersebut akan disimpan.
```
clients[client_count].socket = new_socket;
strcpy(clients[client_count].username, username);
clients[client_count].admin = admin;
client_count++;
```

