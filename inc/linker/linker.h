#ifndef LINKER_H
#define LINKER_H

#include "tabela_sekcija.h"
#include "tabela_simbola.h"

#define MY_ENTRY 0x40000000

typedef struct {
  TabelaSekcija* tabela_sekcija;
  TabelaSimbola* tabela_simbola;
} Linker;

Linker* init_linker();

void procesiraj_ulazni_fajl(Linker* linker, const char* ime_ulaznog_fajla);

void ispisi_strukture(Linker* linker);

void napravi_izvrsni_fajl(Linker* linker, const char* ime_izlaznog_fajla);

Linker* dohvati_linker();

#endif