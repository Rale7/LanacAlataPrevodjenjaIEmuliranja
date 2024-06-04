#ifndef SIMBOL_H
#define SIMBOL_H

struct sekcija;
struct simbol;
struct relokacioni_zapis;

enum Tip {LOKALNI, GLOBALNI};

typedef int (*DohvatiSekciju)(struct simbol*);
typedef struct relokacioni_zapis* (*NapraviRelokacioniZapis)(struct simbol*, int offset, int addend);
typedef int (*DohvatiVrednost) (struct simbol*);
typedef char (*DohvatiTip) (struct simbol*);
typedef char (*DohvatiBind) (struct simbol*);
typedef void (*ObrisiSimbol) (struct simbol*);

typedef struct simbol_TVF {
  DohvatiSekciju dohvati_sekciju;
  NapraviRelokacioniZapis napravi_relokacioni_zapis;
  DohvatiVrednost dohvati_vrednost;
  DohvatiTip dohvati_tip;
  DohvatiBind dohvati_bind;
  ObrisiSimbol obrisi_simbol;
} Simbol_TVF;

typedef struct simbol {
  enum Tip tip;
  int vrednost;
  int id;
  char* naziv;
  struct sekcija* sekcija;
  Simbol_TVF* tvf;
} Simbol;

Simbol* init_simbol(const char*, int, struct sekcija*);

Simbol* init_lokalni_simbol(const char*, int, struct sekcija*);

Simbol* init_globalni_simbol(const char*, int, struct sekcija*);

Simbol* init_simbolicka_konstanta(const char*, int, enum Tip);

Simbol* init_nedefinisan_simbol(const char*);

void prebaci_u_simbolicku_konstantu(struct simbol*, int vrednost);

void prebaci_u_definisan(struct simbol*, struct sekcija*, int vrednost);

char dohvati_nedefinisan_tip(Simbol* simbol);

#endif

