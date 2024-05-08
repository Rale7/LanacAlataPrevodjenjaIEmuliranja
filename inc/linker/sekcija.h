#ifndef SEKCIJA_H
#define SEKCIJA_H

#include "simbol.h"

struct relokacioni_zapis;
struct tabela_simbola;

typedef struct sekcija{
  Simbol simbol;
  int velicina;
  int kapacitet;
  int virtuelna_adresa;
  char* sadrzaj;
  struct relokacioni_zapis* prvi;
  struct relokacioni_zapis** indirect;
} Sekcija;

Sekcija* init_sekcija(struct tabela_simbola* ,const char*);

char* prosiri_sadrzaj(Sekcija*, int);

void dodaj_relokacioni_zapis(Sekcija* sekcija, struct relokacioni_zapis*);

void razresi_relokacije(Sekcija*);

void ispisi_sadrzaj(Sekcija* sekcija);

#endif
