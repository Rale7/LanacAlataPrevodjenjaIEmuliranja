#ifndef TABELA_SIMBOLA_H
#define TABELA_SIMBOLA_H

#include "simbol.h"

#define BROJ_ULAZA_TS 100

typedef struct {

  SimbolElem* prvi[BROJ_ULAZA_TS];
  SimbolElem** indirect[BROJ_ULAZA_TS];

} TabelaSimbola;

TabelaSimbola* init_ts();

void obrisi_tabelu_simbola(TabelaSimbola*);

Simbol* dohvati_vrednost_simbola(TabelaSimbola*, const char*);

void dodaj_simbol(TabelaSimbola*,Simbol*);

#endif