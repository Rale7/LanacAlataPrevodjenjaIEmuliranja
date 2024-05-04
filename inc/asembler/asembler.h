#ifndef ASEMBLER_H
#define ASEMBLER_H

#include "tabela_simbola.h"
#include "sekcija.h"

typedef struct asembler {
  TabelaSimbola* tabel_simbola;
  SekcijaElem* sekcije;
  SekcijaElem** indirect;
  Sekcija* undefined;
  Sekcija* trenutna_sekcija;
} Asembler;

Asembler* dohvati_asembler();

Sekcija* napravi_novu_sekciju(Asembler*, const char* ime);

void napravi_elf_file(Asembler* asembler, const char* izlazni_fajl);

#endif
