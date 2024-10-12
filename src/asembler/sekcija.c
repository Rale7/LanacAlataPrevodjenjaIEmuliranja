
#include "asembler/sekcija.h"

#include <elf.h>
#include <stdio.h>
#include <stdlib.h>

#include "asembler/bazen_literala.h"
#include "asembler/relokacioni_zapis.h"

static void sekcija_ispis(Simbol* simbol) {
  printf("%-7d\t\t%-7d\t\tLOC\t\t%-7d\t\t%s\n", simbol->redosled,
         simbol->vrednost, simbol->sekcija->simbol->redosled, simbol->naziv);
}

static void sekcija_ispis_rz(Simbol* simbol,
                             RelokacioniZapis* relokacioni_zapis) {
  printf("%-6d\t\t%-6d\t\t%-6d", relokacioni_zapis->offset, simbol->redosled,
         0);
}

static int sekcija_addend_rz(Simbol* simbol) { return 0; }

static char sekcija_dohvati_bind(Simbol* simbol) { return STB_LOCAL; }

static char sekcija_dohvati_tip(Simbol* simbol) { return STT_SECTION; }

static int sekcija_simbol_rel(Simbol* simbol) { return simbol->redosled; }

static Tip_TVF sekcija_tvf = {
    .ispis_simbola = &sekcija_ispis,
    .ispis_relokacionog_zapisa = &sekcija_ispis_rz,
    .dohvati_dodavanje = &sekcija_addend_rz,
    .dohvati_bind = &sekcija_dohvati_bind,
    .dohvati_tip = &sekcija_dohvati_tip,
    .dohvati_simbol_rel = &sekcija_simbol_rel,
    .dohvati_referisanu_sekciju = &definisana_referisana_sekcija};

Sekcija* init_sekcija(const char* ime, Simbol* simbol) {
  Sekcija* nova = (Sekcija*)malloc(sizeof(Sekcija));

  if (nova == NULL) {
    exit(1);
  }

  nova->location_counter = 0;
  nova->sadrzaj = init_sadrzaj_sekcije();
  nova->trz = init_TRZ();
  nova->bazen_literala = init_bazen();
  nova->prethodni_bl = NULL;

  if (simbol == NULL) {
    nova->simbol = init_simbol(ime, 0, nova);
  } else {
    nova->simbol = simbol;
  }

  nova->simbol->tip_tvf = &sekcija_tvf;

  return nova;
}

void obrisi_sekciju(Sekcija* sekcija) {
  obrisi_trz(sekcija->trz);

  if (sekcija->prethodni_bl) {
    obrisi_bazen(sekcija->bazen_literala);
  }

  free(sekcija->sadrzaj->byte);
  free(sekcija->sadrzaj);

  free(sekcija);
}