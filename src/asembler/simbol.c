#include <stdlib.h>
#include <stdio.h>
#include <elf.h>
#include "../../inc/asembler/simbol.h"
#include "../../inc/asembler/sekcija.h"
#include "../../inc/asembler/instrukcije.h"
#include "../../inc/asembler/bazen_literala.h"
#include "../../inc/asembler/relokacioni_zapis.h"

static Simbol* prvi = NULL;
static Simbol** indirect = &prvi;

ObracanjeInstrukcije* init_obracanje_instrukcija(Sekcija* sekcija, int lokacija) {

  ObracanjeInstrukcije* novo = (ObracanjeInstrukcije*) malloc(sizeof(ObracanjeInstrukcije));
  if (novo == NULL) {
    exit(1);
  }

  novo->sekcija = sekcija;
  novo->lokacija = lokacija;

  return novo;
}

void lokalni_ispis(Simbol* simbol) {
  printf("%-7d\t\t%-7d\t\tLOC \t\t%-7d\t\t%s\n", simbol->redosled, simbol->vrednost, simbol->sekcija->simbol->redosled, simbol->naziv);
}

static void lokalni_ispis_rz(Simbol* simbol, RelokacioniZapis* relokacioni_zapis) {

  printf("%-6d\t\t%-6d\t\t%-6d", relokacioni_zapis->offset, simbol->sekcija->simbol->redosled, simbol->vrednost);
}

static int lokalni_addend_rz(Simbol* simbol) {
  return simbol->vrednost;
}

char lokalni_tip(Simbol* simbol) {
  return STB_LOCAL;
}

static int lokalni_simbol_rel(Simbol *simbol) {
  return simbol->sekcija->simbol->redosled;
}

static Tip_TVF lokalni_tvf = {
  .ispis_simbola = &lokalni_ispis,
  .ispis_relokacionog_zapisa = &lokalni_ispis_rz,
  .dohvati_dodavanje = &lokalni_addend_rz,
  .dohvati_bind = &lokalni_tip,
  .dohvati_tip = &dohvati_tip_nedefinisan,
  .dohvati_simbol_rel = &lokalni_simbol_rel,
  .dohvati_referisanu_sekciju = &definisana_referisana_sekcija
};

Simbol* dohvati_prvi_simbol() {
  return prvi;
}

void uvezi_simbol(Simbol* novi) {
  static int inicijalizator = 0;

  novi->redosled = inicijalizator++;
  *indirect = novi;
  indirect = &(novi->sledeci);
}

Simbol* init_simbol(const char* naziv, int vrednost, Sekcija* sekcija) {

  Simbol* novi = (Simbol*) malloc(sizeof(Simbol));
  if (novi == NULL) {
    exit(1);
  }

  novi->naziv = naziv;
  novi->vrednost = vrednost;
  
  novi->sledeci = NULL;
  novi->sekcija = sekcija;
  novi->oulista = NULL;
  novi->oilista = NULL;
  novi->neizracunjivi = NULL;

  uvezi_simbol(novi);

  novi->tip_tvf = &lokalni_tvf;

  return novi;
}

char dohvati_tip_nedefinisan(Simbol* simbol) {
  return STT_NOTYPE;
}

int definisana_referisana_sekcija(Simbol* simbol) {
  return simbol->sekcija->broj_elf_ulaza;
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

void obrisi_simbol(Simbol* simbol) {

  while (simbol->oilista) {
    ObracanjeInstrukcije* stari = simbol->oilista;
    simbol->oilista = simbol->oilista->sledeci;
    free(stari);
  }

  while (simbol->oulista) {
    ObracanjeUnapred* stari = simbol->oulista;
    simbol->oulista = simbol->oulista->sledeci;
    free(stari);
  }

  free(simbol);

}

void obrisi_simbole() {

  while (prvi) {
    Simbol* stari = prvi;
    prvi = prvi->sledeci;
    obrisi_simbol(stari);
  }

}