#include "asembler/stek_izraz.h"

#include <stdlib.h>

#include "asembler/izraz.h"

StekCvor* push_stek(StekCvor* vrh, ClanIzraza* clan) {
  StekCvor* novi = (StekCvor*)malloc(sizeof(StekCvor));
  if (novi == NULL) {
    exit(1);
  }

  novi->clan = clan;
  novi->sledeci = vrh;

  return novi;
}

StekCvor* pop_stek(StekCvor* vrh) {
  StekCvor* stari = vrh;
  vrh = vrh->sledeci;
  free(stari);
  return vrh;
}

ClanIzraza* top_stek(StekCvor* vrh) { return vrh->clan; }

int prazan_stek(StekCvor* vrh) { return vrh == NULL; }