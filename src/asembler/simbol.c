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
  printf("%-7d\t\t%-7d\t\tLOC \t\t%-7d\t\t%s\n", simbol->redosled, simbol->vrednost, simbol->sekcija->simbol->redosled, simbol->naziv);
}

void lokalni_ispis_rz(Simbol* simbol, RelokacioniZapis* relokacioni_zapis) {

  printf("%-6d\t\t%-6d\t\t%-6d", relokacioni_zapis->offset, simbol->sekcija->simbol->redosled, simbol->vrednost);
}

void globalni_ispis(Simbol* simbol) {
  printf("%-7d\t\t%-7d\t\tGLOB\t\t%-7d\t\t%s\n", simbol->redosled, simbol->vrednost, simbol->sekcija->simbol->redosled, simbol->naziv);
}

void globalni_ispis_rz(Simbol* simbol, RelokacioniZapis* relokacioni_zapis) {

  printf("%-6d\t\t%-6d\t\t%-6d", relokacioni_zapis->offset, simbol->redosled, 0);
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
  printf("%-7s\t\t%-7s\t%-7s\t%-7s\t\t%s\n", "RB", "Vrednost", "Tip", "Bind", "Naziv");
  for (Simbol* simbol = prvi; simbol; simbol = simbol->sledeci) {
    simbol->tip_tvf->ispis_simbola(simbol);
  }
}

void ugradi_pomeraj_simbol(Sekcija* sekcija, int obracanje, int pomeraj) {
  char reg_pom = *dohvati_sadrzaj(sekcija->sadrzaj, obracanje + 2);
  reg_pom = (char)((reg_pom & 0xF0) | ((pomeraj & 0xF00) >> 8));
  postavi_sadrzaj(sekcija->sadrzaj, obracanje + 2, &reg_pom, 1);
  
  char pom = (char) (pomeraj & 0xFF);
  postavi_sadrzaj(sekcija->sadrzaj, obracanje + 3, &pom, 1);
}