#ifndef SEKCIJA_H
#define SEKCIJA_H

struct simbol;
struct tabela_rz;
struct bazen_literala;

#include "sadrzaj_sekcije.h"


typedef struct sekcija {
  int location_counter;
  SadrzajSekcije* sadrzaj;
  struct simbol* simbol;
  struct tabela_rz* trz;
  struct bazen_literala* bazen_literala;
  struct bazen_literala* prethodni_bl;
  int broj_elf_ulaza;

} Sekcija;

typedef struct sekcija_elem{
  Sekcija* sekcija;
  struct sekcija_elem* sledeci;
} SekcijaElem;

Sekcija* init_sekcija(const char*, struct simbol*);

#endif