#ifndef ASEMBLER_H
#define ASEMBLER_H

#include "tabela_simbola.h"
#include "sekcija.h"

struct st_tns;

typedef struct asembler {
  TabelaSimbola* tabel_simbola;
  SekcijaElem* sekcije;
  SekcijaElem** indirect;
  Sekcija* undefined;
  Sekcija* trenutna_sekcija;
  struct st_tns* tabela_neizrazunljivih_simbola;
} Asembler;

Asembler* dohvati_asembler();

Sekcija* napravi_novu_sekciju(Asembler*, const char* ime);

void napravi_elf_file(Asembler* asembler, const char* izlazni_fajl);

void obrisi_asembler(Asembler* asembler);

#endif
