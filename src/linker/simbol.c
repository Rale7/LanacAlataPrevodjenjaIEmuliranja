#include "linker/simbol.h"

#include <elf.h>
#include <stdlib.h>
#include <string.h>

#include "linker/sekcija.h"

Simbol* init_simbol(const char* naziv, int vrednost, Sekcija* sekcija) {
  Simbol* simbol = (Simbol*)malloc(sizeof(Simbol));
  if (simbol == NULL) {
    exit(1);
  }

  simbol->naziv = (char*)calloc(strlen(naziv) + 1, sizeof(char));
  strcpy(simbol->naziv, naziv);

  simbol->vrednost = vrednost;
  simbol->sekcija = sekcija;

  return simbol;
}

char dohvati_nedefinisan_tip(Simbol* simbol) { return STT_NOTYPE; }