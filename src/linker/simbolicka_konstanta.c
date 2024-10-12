#include <elf.h>
#include <stdlib.h>

#include "linker/relokacioni_zapis.h"
#include "linker/sekcija.h"
#include "linker/simbol.h"

static int apsolutna_sekcija(Simbol* simbol) { return SHN_ABS; }

static RelokacioniZapis* simkonst_rel(Simbol* simbol, int offset, int addend) {
  return init_relokacioni_zapis(offset, simbol, addend);
}

static int simkonst_vrednost(Simbol* simbol) { return simbol->vrednost; }

static char dohvati_bind_simkonst(Simbol* simbol) {
  if (simbol->tip == LOKALNI) {
    return STB_LOCAL;
  } else {
    return STB_GLOBAL;
  }
}

static void obrisi_simbolicku_konstantu(Simbol* simbol) {
  free(simbol->naziv);
  free(simbol);
}

static Simbol_TVF apsolutni_tvf = {
    .dohvati_sekciju = &apsolutna_sekcija,
    .napravi_relokacioni_zapis = &simkonst_rel,
    .dohvati_vrednost = &simkonst_vrednost,
    .dohvati_bind = &dohvati_bind_simkonst,
    .dohvati_tip = &dohvati_nedefinisan_tip,
    .obrisi_simbol = &obrisi_simbolicku_konstantu,
};

Simbol* init_simbolicka_konstanta(const char* naziv, int vrednost,
                                  enum Tip tip) {
  Simbol* simbol = init_simbol(naziv, vrednost, NULL);

  simbol->tip = tip;
  simbol->tvf = &apsolutni_tvf;

  return simbol;
}

void prebaci_u_simbolicku_konstantu(Simbol* simbol, int vrednost) {
  simbol->vrednost = vrednost;
  simbol->tvf = &apsolutni_tvf;
  simbol->tip = GLOBALNI;
}