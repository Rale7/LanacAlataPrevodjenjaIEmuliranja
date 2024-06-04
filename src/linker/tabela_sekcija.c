#include <stdlib.h>
#include <string.h>
#include "../../inc/linker/tabela_sekcija.h"
#include "../../inc/linker/sekcija.h"
#include "../../inc/linker/tabela_simbola.h"

TabelaSekcija* init_tabela_sekcija() {

  TabelaSekcija* nova = (TabelaSekcija*) malloc(sizeof(TabelaSekcija));
  if (nova == NULL) {
    exit(1);
  }

  nova->kapacitet = 50;
  nova->sekcije = (Sekcija**) calloc(nova->kapacitet, sizeof(Sekcija*));
  if (nova->sekcije == NULL) {
    exit(1);
  }

  nova->broj_sekcija = 0;

  for (int i = 0; i < MAKS_BROJ_SEKCIJA; i++) {
    nova->prvi[i] = NULL;
    nova->indirect[i] = &nova->prvi[i];
  }

  return nova;

}

static int izracunaj_indeks(TabelaSekcija* ts, const char* naziv) {

  int broj = 0;

  for (int i = 0; naziv[i]; i++) {
    broj += naziv[i];
  }

  return broj % MAKS_BROJ_SEKCIJA;

} 

static void upisi_sekciju_niz(TabelaSekcija* tabela_sekcija,  Sekcija* sekcija) {

  if (tabela_sekcija->broj_sekcija == tabela_sekcija->kapacitet) {
    tabela_sekcija->kapacitet = (tabela_sekcija->kapacitet * 3) / 2;
    tabela_sekcija->sekcije = (Sekcija**) realloc(tabela_sekcija->sekcije, tabela_sekcija->kapacitet);
    if (tabela_sekcija->sekcije == NULL) {
      exit(1);
    }
  }

  tabela_sekcija->sekcije[tabela_sekcija->broj_sekcija++] = sekcija;
}



void ubaci_zadatu_sekciju(TabelaSekcija* ts, TabelaSimbola* tab_sim, const char* naziv) {

  int indeks = izracunaj_indeks(ts, naziv);

  for (SekcijaUlaz* tekuci = ts->prvi[indeks]; tekuci; tekuci = tekuci->sledeci) {
    if (strcmp(tekuci->sekcija->simbol.naziv, naziv) == 0) {
      return;
    } 
  }

  SekcijaUlaz* novi = (SekcijaUlaz*) malloc(sizeof(SekcijaUlaz));
  if (novi == NULL) {
    exit(1);
  }

  *ts->indirect[indeks] = novi;
  novi->sledeci = NULL;
  ts->indirect[indeks] = &novi->sledeci;

  novi->sekcija = init_sekcija(tab_sim, naziv);
  upisi_sekciju_niz(ts, novi->sekcija);
}

struct sekcija* dohvati_sekciju(TabelaSekcija* ts, const char* naziv) {

  int indeks = izracunaj_indeks(ts, naziv);

  for (SekcijaUlaz* tekuci = ts->prvi[indeks]; tekuci; tekuci = tekuci->sledeci) {
    if (strcmp(tekuci->sekcija->simbol.naziv, naziv) == 0) {
      return tekuci->sekcija;
    }
  }
}

void obrisi_tabelu_sekcija(TabelaSekcija* ts) {

  for (int i = 0; i < MAKS_BROJ_SEKCIJA; i++) {

    while (ts->prvi[i]) {
      SekcijaUlaz* stari = ts->prvi[i];
      ts->prvi[i] = ts->prvi[i]->sledeci;
      free(stari);
    }
  }

  free(ts);

}

void heapify(Sekcija** sekcije, int n, int i) {
  int najveci = i;

  int l = 2 * i + 1;
  int d = 2 * i + 2;

  if (l < n && sekcije[l]->virtuelna_adresa > sekcije[najveci]->virtuelna_adresa) {
    najveci = l;
  }

  if (d < n && sekcije[d]->virtuelna_adresa > sekcije[najveci]->virtuelna_adresa) {
    najveci = d;
  }

  if (najveci != i) {
    Sekcija* temp = sekcije[i];
    sekcije[i] = sekcije[najveci];
    sekcije[najveci] = temp;

    heapify(sekcije, n, najveci);
  }
}

void heap_sort(Sekcija** sekcije, int n) {

  for (int i = n / 2; i >= 0; i--) {
    heapify(sekcije, n, i);
  }

  for (int i = n - 1; i > 0; i--) {

    Sekcija* tmp = sekcije[i];
    sekcije[i] = sekcije[0];
    sekcije[0] = tmp; 

    heapify(sekcije, i, 0);
  }

}

void sortiraj_tabelu_sekcija(TabelaSekcija* tabela_sekcija) {

  heap_sort(tabela_sekcija->sekcije, tabela_sekcija->broj_sekcija);

}