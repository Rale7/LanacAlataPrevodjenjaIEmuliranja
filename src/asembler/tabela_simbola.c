
#include <stdlib.h>
#include <string.h>
#include "../../inc/asembler/tabela_simbola.h"

static int nadji_ulaz(const char* ime) {

  int broj_ulaza = 0;

  for (int i = 0; ime[i]; i++) {
    broj_ulaza += ime[i];
  }

  return broj_ulaza % BROJ_ULAZA_TS;

}

TabelaSimbola* init_ts() {

  TabelaSimbola* ts = malloc(sizeof(TabelaSimbola));

  for (int i = 0; i < BROJ_ULAZA_TS; i++) {
    ts->prvi[i] = NULL;
    ts->indirect[i] = &(ts->prvi[i]);
  }

}

void obrisi_tabelu_simbola(TabelaSimbola* ts) {

  for (int i = 0; i < BROJ_ULAZA_TS; i++) {

    while (ts->prvi[i]) {
      SimbolElem* stari = ts->prvi[i];
      ts->prvi[i] = ts->prvi[i]->sledeci;
      free(stari);
    }
  }

  free(ts);

}

Simbol* dohvati_vrednost_simbola(TabelaSimbola* ts, const char* ime) {

  int broj_ulaza = nadji_ulaz(ime);

  for (SimbolElem* trenutni = ts->prvi[broj_ulaza]; trenutni; trenutni = trenutni->sledeci) {
    if (strcmp(trenutni->simbol->naziv, ime) == 0) {
      return trenutni->simbol;
    }
  }

  return NULL;

}

void dodaj_simbol(TabelaSimbola* ts, Simbol* novi) {

  int broj_ulaza = nadji_ulaz(novi->naziv);

  SimbolElem* novi_elem = (SimbolElem*) malloc(sizeof(SimbolElem));
  
  novi_elem->simbol = novi;
  novi_elem->sledeci = NULL;

  *(ts->indirect[broj_ulaza]) = novi_elem;
  ts->indirect[broj_ulaza] = &(novi_elem->sledeci);

}