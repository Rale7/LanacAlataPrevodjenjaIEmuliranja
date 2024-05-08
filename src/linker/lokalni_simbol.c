#include <stdlib.h>
#include <stdio.h>
#include <elf.h>
#include "../../inc/linker/simbol.h"
#include "../../inc/linker/sekcija.h"
#include "../../inc/linker/relokacioni_zapis.h"

static int lokalni_dohvati_sekciju(Simbol* simbol) {
  return simbol->sekcija->simbol.id;
}

static RelokacioniZapis* lokalni_relokacioni_zapis(Simbol* simbol, int offset, int addend) {
  printf("Linker ne treba da ima pristup lokalnom simbolu %s\n", simbol->naziv);
  exit(1);
}

static int dohvati_vrednost(Simbol* simbol) {
  printf("Linker ne treba da ima pristup lokalnom simbolu %s\n", simbol->naziv);
  exit(1);
}

static char lokalni_tip(Simbol* simbol) {
  return STB_LOCAL;
}

static Simbol_TVF lokalni_tvf = {
  .dohvati_sekciju = &lokalni_dohvati_sekciju,
  .napravi_relokacioni_zapis = &lokalni_relokacioni_zapis,
  .dohvati_vrednost = &dohvati_vrednost,
  .dohvati_tip = &lokalni_tip,
  .dohvati_tip = &dohvati_nedefinisan_tip
};

Simbol* init_lokalni_simbol(const char* naziv, int vrednost, Sekcija* sekcija) {

  Simbol* novi = init_simbol(naziv, vrednost, sekcija);

  novi->vrednost= sekcija->velicina + vrednost;
  novi->tvf = &lokalni_tvf;
  novi->tip = LOKALNI;

  return novi;
}