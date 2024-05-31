#ifndef LINKER_H
#define LINKER_H

#include "tabela_sekcija.h"
#include "tabela_simbola.h"

#define MY_ENTRY 0x40000000

struct cmd_sekcija;

typedef struct linker {
  TabelaSekcija* tabela_sekcija;
  TabelaSimbola* tabela_simbola;
  struct cmd_sekcija* prvi;
  struct cmd_sekcija** indirect;

} Linker;

Linker* init_linker();

void procesiraj_ulazni_fajl(Linker* linker, const char* ime_ulaznog_fajla);

void razresi_virtuelne_adrese(Linker* linker);

void ispisi_strukture(Linker* linker);

void napravi_izvrsni_fajl(Linker* linker, const char* ime_izlaznog_fajla);

void napravi_relokativni_fajl(Linker* linker, const char* ime_izlaznog_fajla);

Linker* dohvati_linker();

#endif