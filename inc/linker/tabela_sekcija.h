#ifndef TABELA_SEKCIJA_H
#define TABELA_SEKCIJA_H

struct sekcija;
struct tabela_simbola;

#define MAKS_BROJ_SEKCIJA 30

typedef struct sekcija_ulaz {
  struct sekcija_ulaz* sledeci;
  struct sekcija* sekcija;
} SekcijaUlaz;

typedef struct tabela_sekcija {
  struct sekcija** sekcije;
  int broj_sekcija;
  int kapacitet;
  SekcijaUlaz* prvi[MAKS_BROJ_SEKCIJA];
  SekcijaUlaz** indirect[MAKS_BROJ_SEKCIJA];
} TabelaSekcija;

TabelaSekcija* init_tabela_sekcija();

void sortiraj_tabelu_sekcija(TabelaSekcija*);

void ubaci_zadatu_sekciju(TabelaSekcija*, struct tabela_simbola*, const char*);

struct sekcija* dohvati_sekciju(TabelaSekcija*, const char*);

void obrisi_tabelu_sekcija(TabelaSekcija*);

#endif