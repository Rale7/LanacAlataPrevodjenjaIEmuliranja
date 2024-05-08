#include <stdlib.h>
#include <elf.h>
#include <stdio.h>
#include "../../inc/linker/simbol.h"
#include "../../inc/linker/sekcija.h"
#include "../../inc/linker/relokacioni_zapis.h"

static int nedefinisana_sekcija(Simbol* simbol) {

  return SHN_UNDEF;
}

static RelokacioniZapis* nedefinisan_rel(Simbol* simbol, int offset, int addend) {
  
  return init_relokacioni_zapis(offset, simbol, addend);
}

static int nedefinisana_vrednost(Simbol* simbol) {
  printf("Simbol %s nije definisan\n", simbol->naziv);
  exit(1);
}

static char nedefinisan_bind(Simbol* simbol) {
  return STB_GLOBAL;
}

static Simbol_TVF nedefinisani_tvf = {
  .dohvati_sekciju = &nedefinisana_sekcija,
  .napravi_relokacioni_zapis = &nedefinisan_rel,
  .dohvati_vrednost = &nedefinisana_vrednost,
  .dohvati_bind = &nedefinisan_bind,
  .dohvati_tip = &dohvati_nedefinisan_tip
};

Simbol* init_nedefinisan_simbol(const char* naziv) {

  Simbol* novi = init_simbol(naziv, 0, NULL);

  novi->tip = GLOBALNI;
  novi->tvf = &nedefinisani_tvf;
  
  return novi;
}