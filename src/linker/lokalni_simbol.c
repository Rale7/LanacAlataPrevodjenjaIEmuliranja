#include <elf.h>
#include <stdio.h>
#include <stdlib.h>

#include "linker/relokacioni_zapis.h"
#include "linker/sekcija.h"
#include "linker/simbol.h"

static int lokalni_dohvati_sekciju(Simbol* simbol) {
  return simbol->sekcija->simbol.id;
}

static RelokacioniZapis* lokalni_relokacioni_zapis(Simbol* simbol, int offset,
                                                   int addend) {
  printf("Linker ne treba da ima pristup lokalnom simbolu %s\n", simbol->naziv);
  exit(1);
}

static int dohvati_vrednost(Simbol* simbol) {
  printf("Linker ne treba da ima pristup lokalnom simbolu %s\n", simbol->naziv);
  exit(1);
}

static char lokalni_tip(Simbol* simbol) { return STB_LOCAL; }

static void obrisi_lokalni_simbol(Simbol* simbol) {
  free(simbol->naziv);
  free(simbol);
}

static Simbol_TVF lokalni_tvf = {
    .dohvati_sekciju = &lokalni_dohvati_sekciju,
    .napravi_relokacioni_zapis = &lokalni_relokacioni_zapis,
    .dohvati_vrednost = &dohvati_vrednost,
    .dohvati_tip = &lokalni_tip,
    .dohvati_tip = &dohvati_nedefinisan_tip,
    .obrisi_simbol = &obrisi_lokalni_simbol,
};

Simbol* init_lokalni_simbol(const char* naziv, int vrednost, Sekcija* sekcija) {
  Simbol* novi = init_simbol(naziv, vrednost, sekcija);

  novi->vrednost = sekcija->velicina + vrednost;
  novi->tvf = &lokalni_tvf;
  novi->tip = LOKALNI;

  return novi;
}