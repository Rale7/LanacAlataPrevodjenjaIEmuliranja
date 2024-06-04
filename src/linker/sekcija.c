#include <stdlib.h>
#include <string.h>
#include <elf.h>
#include <stdio.h>
#include <string.h>
#include "../../inc/linker/sekcija.h"
#include "../../inc/linker/relokacioni_zapis.h"
#include "../../inc/linker/linker.h"


static int sekcija_dohvati_sekciju(Simbol* simbol) {
  return simbol->sekcija->simbol.id;
}

static RelokacioniZapis* sekcija_relokacioni_zapis(Simbol* simbol, int offset, int addend) {
  Sekcija* sekcija = (Sekcija*) simbol;

  return init_relokacioni_zapis(offset, simbol, addend + sekcija->velicina);
}

static int dohvati_vrednost_sekcije(Simbol* simbol) {
  Sekcija* sekcija = (Sekcija*) simbol;

  return sekcija->virtuelna_adresa;
}

static char dohvati_bind_sekcije(Simbol* simbol) {
  return STB_LOCAL;
}

static char dohvati_tip_sekcije(Simbol* simbol) {
  return STT_SECTION;
}

static void obrisi_sekciju(Simbol* simbol) {
  Sekcija* sekcija = (Sekcija*) simbol;

  free(sekcija->sadrzaj);
  free(sekcija->simbol.naziv);
  
  while (sekcija->prvi) {
    RelokacioniZapis* stari = sekcija->prvi;
    sekcija->prvi = sekcija->prvi->sledeci;
    free(stari);
  }

  free(sekcija);
}

static Simbol_TVF sekcija_tvf = {
  .dohvati_sekciju = &sekcija_dohvati_sekciju,
  .napravi_relokacioni_zapis = &sekcija_relokacioni_zapis,
  .dohvati_vrednost = &dohvati_vrednost_sekcije,
  .dohvati_bind = &dohvati_bind_sekcije,
  .dohvati_tip = &dohvati_tip_sekcije,
  .obrisi_simbol = &obrisi_sekciju,
};

Sekcija* init_sekcija(TabelaSimbola* tabela_simbola, const char* naziv) {

  Sekcija* sekcija = (Sekcija*) malloc(sizeof(Sekcija));
  if (sekcija == NULL) {
    exit(1);
  }

  sekcija->kapacitet = 50;
  sekcija->sadrzaj = (char*) malloc(sekcija->kapacitet * sizeof(char));
  if (sekcija->sadrzaj == NULL) {
    exit(1);
  }

  sekcija->velicina = 0;
  sekcija->prvi = NULL;
  sekcija->indirect = &sekcija->prvi;
  sekcija->virtuelna_adresa = 0;
  sekcija->broj_elf_ulaza = 0;
  
  sekcija->simbol.naziv = (char*) calloc(strlen(naziv) + 1, sizeof(char));
  strcpy(sekcija->simbol.naziv, naziv);
  sekcija->simbol.sekcija = sekcija;
  sekcija->simbol.tip = LOKALNI;
  sekcija->simbol.vrednost = 0;
  sekcija->simbol.tvf = &sekcija_tvf;

  ubaci_simbol(tabela_simbola, (Simbol*) sekcija);

  return sekcija;
}

static void safe_realloc(Sekcija* sekcija) {

  sekcija->kapacitet = (sekcija->kapacitet * 3) / 2;
  sekcija->sadrzaj = (char*) realloc(sekcija->sadrzaj, sekcija->kapacitet);
  if (sekcija->sadrzaj == NULL) {
    exit(1);
  }
  
}

char* prosiri_sadrzaj(Sekcija* sekcija, int prosirenje) {

  while (sekcija->kapacitet < prosirenje + sekcija->velicina) {
    safe_realloc(sekcija);
  }

  return &sekcija->sadrzaj[sekcija->velicina];
}

void dodaj_relokacioni_zapis(Sekcija* sekcija, RelokacioniZapis* relokacioni_zapis) {

  *sekcija->indirect = relokacioni_zapis;
  sekcija->indirect = &relokacioni_zapis->sledeci;
}

void razresi_relokacije(Sekcija* sekcija) {

  for (RelokacioniZapis* tekuci = sekcija->prvi; tekuci; tekuci = tekuci->sledeci) {
    int zamena = tekuci->simbol->tvf->dohvati_vrednost(tekuci->simbol) + tekuci->addend;

    memcpy(&sekcija->sadrzaj[tekuci->offset], &zamena, sizeof(zamena));
  }

}

void ispisi_sadrzaj(Sekcija* sekcija, FILE* izlaz) {

  for (int i = 0; i < sekcija->velicina; i += 4) {
    fprintf(izlaz, "\n%08x:\t", (unsigned int) sekcija->virtuelna_adresa + i);
    for (int j = i; j < sekcija->velicina && j < i + 4; j++) {
      fprintf(izlaz, "%02hhx\t", sekcija->sadrzaj[j]);
    }
  }
  fprintf(izlaz, "\n");
}
