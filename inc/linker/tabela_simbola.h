#ifndef TABELA_SIMBOLA_H
#define TABELA_SIMBOLA_H

struct simbol;

#define BROJ_SIMBOLA 100

typedef struct ulaz_simbol {
  struct simbol* simbol;
  struct ulaz_simbol* sledeci;
} UlazSimbol;

typedef struct tabela_simbola {
  struct simbol** simboli;
  int kapacitet;
  int trenutni_id;
  UlazSimbol* ulazi[BROJ_SIMBOLA];
  UlazSimbol** indirect[BROJ_SIMBOLA];

} TabelaSimbola;

TabelaSimbola* init_tabela_simbola();

struct simbol* proveri_postojanje_globalnog(TabelaSimbola*, const char*);

int ubaci_simbol(TabelaSimbola*, struct simbol*);

struct simbol* dohvati_simbol_ime(TabelaSimbola*, const char*);

struct simbol* dohvati_simbol_id(TabelaSimbola*, int id);

struct simbol* provera_postoji_nedefinisan(TabelaSimbola*);

#endif