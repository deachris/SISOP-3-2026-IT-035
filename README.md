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
