#include "asembler/relokacioni_zapis.h"

#include <stdlib.h>

RelokacioniZapis* init_RZ(int offset, Simbol* simbol) {
  RelokacioniZapis* novi = (RelokacioniZapis*)malloc(sizeof(RelokacioniZapis));
  if (novi == NULL) {
    exit(1);
  }

  novi->offset = offset;
  novi->simbol = simbol;
  novi->sledeci = NULL;

  return novi;
}

TabelaRZ* init_TRZ() {
  TabelaRZ* nova = (TabelaRZ*)malloc(sizeof(TabelaRZ));
  if (nova == NULL) {
    exit(1);
  }

  nova->prvi = NULL;
  nova->poslednji = NULL;

  return nova;
}

void obrisi_trz(TabelaRZ* trz) {
  while (trz->prvi) {
    RelokacioniZapis* stari = trz->prvi;
    trz->prvi = trz->prvi->sledeci;
    free(stari);
  }

  free(trz);
}

void dodaj_relokacioni_zapis(TabelaRZ* trz, RelokacioniZapis* rz) {
  if (!trz->prvi) {
    trz->prvi = rz;
  } else {
    trz->poslednji->sledeci = rz;
  }
  trz->poslednji = rz;
}