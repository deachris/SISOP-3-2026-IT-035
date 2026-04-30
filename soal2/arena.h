#ifndef ARENA_H
#define ARENA_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

typedef struct {
    char username[50];
    char password[50];
    int login;
    int gold;
    int lvl;
    int xp;
    int weapon;
} Users;

static inline int damage(Users *u) {
    return 10 + (u->xp / 50) + u->weapon;
}

static inline void battle_menu(int sock, Users *u) {
    char msg[BUFFER_SIZE];

    send(sock, "\n=== MATCHMAKING ===\nSearching opponent...\n", 38, 0);
    sleep(2);

    send(sock, "No player found.\nMonster Bot appeared!\n", 36, 0);

    int hp = 100 + (u->xp / 10);
    int enemy = 100;

    while (hp > 0 && enemy > 0) {
        sprintf(msg,
            "\n=== ARENA ===\n"
            "Your HP : %d\n"
            "Enemy HP: %d\n"
            "(a) Attack\n"
            "(u) Ultimate\n",
            hp, enemy);

        send(sock, msg, strlen(msg), 0);

        char move[20] = {0};
        read(sock, move, sizeof(move));

        int dmg = (move[0] == 'u' && u->weapon > 0)
                    ? damage(u) * 3
                    : damage(u);

        enemy -= dmg;

        if (enemy <= 0) break;

        hp -= (rand() % 15) + 5;
    }

    if (hp > 0) {
        u->xp += 50;
        u->gold += 120;
        u->lvl = (u->xp / 100) + 1;

        send(sock,
            "\n=== VICTORY ===\nXP +50\nGold +120\n",
            33, 0);
    } else {
        u->xp += 15;
        u->gold += 30;

        send(sock,
            "\n=== DEFEAT ===\nXP +15\nGold +30\n",
            31, 0);
    }
}

static inline void armory(int sock, Users *u) {
    char msg[BUFFER_SIZE];

    sprintf(msg,
        "\n=== Armory ===\n"
        "Gold: %d\n"
        "1. Wood Sword  (100G) +5\n"
        "2. Iron Sword  (300G) +15\n"
        "3. Steel Axe   (600G) +30\n"
        "4. Demon Blade (1500G) +60\n",
        u->gold);

    send(sock, msg, strlen(msg), 0);

    char c[10];
    read(sock, c, sizeof(c));

    int opsi = atoi(c);
    int price = 0, bonus = 0;

    switch (opsi) {
        case 1: price = 100; bonus = 5; break;
        case 2: price = 300; bonus = 15; break;
        case 3: price = 600; bonus = 30; break;
        case 4: price = 1500; bonus = 60; break;
        default:
            send(sock, "Invalid weapon.\n", 15, 0);
            return;
    }

    if (u->gold >= price) {
        u->gold -= price;
        u->weapon = bonus;
        send(sock, "Weapon equipped!\n", 16, 0);
    } else {
        send(sock, "Gold not enough!\n", 17, 0);
    }
}

static inline void history(int sock) {
    send(sock,
        "\n=== MATCH HISTORY ===\n(Placeholder)\n",
        34, 0);
}

#endif
