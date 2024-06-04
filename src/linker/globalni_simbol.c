#include <stdlib.h>
#include <elf.h>
#include "../../inc/linker/simbol.h"
#include "../../inc/linker/sekcija.h"
#include "../../inc/linker/relokacioni_zapis.h"

static int globalni_sekcija(Simbol* simbol) {
  return simbol->sekcija->simbol.id;
}

static RelokacioniZapis* globalni_rel(Simbol* simbol, int offset, int addend) {

  return init_relokacioni_zapis(offset, simbol, addend);
}

static int globalna_vrednost(Simbol* simbol) {
  return simbol->sekcija->virtuelna_adresa + simbol->vrednost;
}

static char globalni_bind(Simbol* simbol) {
  return STB_GLOBAL;
}

static void obrisi_globalni_simbol(Simbol* simbol) {
  free(simbol->naziv);
  free(simbol);
}

static Simbol_TVF globalni_tvf = {
  .dohvati_sekciju = &globalni_sekcija,
  .napravi_relokacioni_zapis = &globalni_rel,
  .dohvati_vrednost = &globalna_vrednost,
  .dohvati_tip = &globalni_bind,
  .dohvati_tip = &dohvati_nedefinisan_tip,
  .obrisi_simbol = &obrisi_globalni_simbol,
};

Simbol* init_globalni_simbol(const char* naziv, int vrednost, Sekcija* sekcija) {

  Simbol* novi = init_simbol(naziv, vrednost, sekcija);

  novi->vrednost = sekcija->velicina + vrednost;
  novi->tip = GLOBALNI;
  novi->tvf = &globalni_tvf;

  return novi;
}

void prebaci_u_definisan(Simbol* simbol, Sekcija* sekcija, int vrednost) {

  simbol->vrednost = vrednost;
  simbol->tvf = &globalni_tvf;
  simbol->tip = GLOBALNI;
  simbol->vrednost = sekcija->velicina + simbol->vrednost;
  simbol->sekcija = sekcija;
}