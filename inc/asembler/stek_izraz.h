#ifndef STEK_IZRAZ_H
#define STEK_IZRAZ_H

struct clan_izraza;

typedef struct stek_cvor {
  struct clan_izraza* clan;
  struct stek_cvor* sledeci;
} StekCvor;

StekCvor* push_stek(StekCvor* vrh, struct clan_izraza* clan);

StekCvor* pop_stek(StekCvor* vrh);

struct clan_izraza* top_stek(StekCvor* vrh);

int prazan_stek(StekCvor* vrh);

#endif