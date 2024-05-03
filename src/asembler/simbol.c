#include <stdlib.h>
#include <stdio.h>
#include "../../inc/asembler/simbol.h"
#include "../../inc/asembler/sekcija.h"
#include "../../inc/asembler/instrukcije.h"
#include "../../inc/asembler/bazen_literala.h"
#include "../../inc/asembler/relokacioni_zapis.h"

static Simbol* prvi = NULL;
static Simbol** indirect = &prvi;

void lokalni_ispis(Simbol* simbol) {
  printf("%d\t%d\tLOC\t%d\t%s\n", simbol->redosled, simbol->vrednost, simbol->sekcija->simbol->redosled, simbol->naziv);
}

void lokalni_ispis_rz(Simbol* simbol, RelokacioniZapis* relokacioni_zapis) {

  printf("%d\t%d\t%d", relokacioni_zapis->offset, simbol->redosled, simbol->vrednost);
}

void globalni_ispis(Simbol* simbol) {
  printf("%d\t%d\tGLOB\t%d\t%s\n", simbol->redosled, simbol->vrednost, simbol->sekcija->simbol->redosled, simbol->naziv);
}

void globalni_ispis_rz(Simbol* simbol, RelokacioniZapis* relokacioni_zapis) {

  printf("%d\t%d\t%d", relokacioni_zapis->offset, simbol->redosled, 0);
}

static Tip_TVF lokalni_tvf = {
  &lokalni_ispis,
  &lokalni_ispis_rz
};

static Tip_TVF globalni_tvf = {
  &globalni_ispis,
  &globalni_ispis_rz
};

Simbol* init_simbol(const char* naziv, int vrednost, Sekcija* sekcija) {

  static int inicijalizator = 0;

  Simbol* novi = (Simbol*) malloc(sizeof(Simbol));

  novi->redosled = inicijalizator++;
  novi->naziv = naziv;
  novi->vrednost = vrednost;
  
  novi->sledeci = NULL;
  novi->sekcija = sekcija;
  novi->oulista = NULL;

  novi->tip_tvf = &lokalni_tvf;

  *indirect = novi;
  indirect = &(novi->sledeci);

  return novi;
}

void prebaci_u_globalni(Simbol* simbol) {
  simbol->tip_tvf = &globalni_tvf;
}

void ispisi_simbole() {

  printf("Tabela simbola\n");
  for (Simbol* simbol = prvi; simbol; simbol = simbol->sledeci) {
    simbol->tip_tvf->ispis_simbola(simbol);
  }
}