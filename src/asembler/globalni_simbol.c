#include <elf.h>
#include <stdio.h>
#include <stdlib.h>

#include "asembler/relokacioni_zapis.h"
#include "asembler/sekcija.h"
#include "asembler/simbol.h"

static void globalni_ispis(Simbol *simbol) {
  printf("%-7d\t\t%-7d\t\tGLOB\t\t%-7d\t\t%s\n", simbol->redosled,
         simbol->vrednost, simbol->sekcija->simbol->redosled, simbol->naziv);
}

static void globalni_ispis_rz(Simbol *simbol,
                              RelokacioniZapis *relokacioni_zapis) {
  printf("%-6d\t\t%-6d\t\t%-6d", relokacioni_zapis->offset, simbol->redosled,
         0);
}

static int globalni_addend_rz(Simbol *simbol) { return 0; }

static char globalni_tip(Simbol *simbol) { return STB_GLOBAL; }

static int globalni_simbol_rel(Simbol *simbol) { return simbol->redosled; }

static Tip_TVF globalni_tvf = {
    .ispis_simbola = &globalni_ispis,
    .ispis_relokacionog_zapisa = &globalni_ispis_rz,
    .dohvati_dodavanje = &globalni_addend_rz,
    .dohvati_bind = &globalni_tip,
    .dohvati_tip = &dohvati_tip_nedefinisan,
    .dohvati_simbol_rel = &globalni_simbol_rel,
    .dohvati_referisanu_sekciju = &definisana_referisana_sekcija};

void prebaci_u_globalni(Simbol *simbol) {
  if (simbol->sekcija &&
      simbol->tip_tvf->dohvati_referisanu_sekciju(simbol) == SHN_ABS) {
    simbol->tip = STB_GLOBAL;
    return;
  }

  simbol->tip_tvf = &globalni_tvf;
  simbol->tip = STB_GLOBAL;
}