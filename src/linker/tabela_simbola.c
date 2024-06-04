#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <elf.h>
#include "../../inc/linker/simbol.h"
#include "../../inc/linker/tabela_simbola.h"

TabelaSimbola* init_tabela_simbola() {

  TabelaSimbola* nova = (TabelaSimbola*) malloc(sizeof(TabelaSimbola));
  if (nova == NULL) {
    exit(1);
  }

  nova->kapacitet = 50;
  nova->simboli = (Simbol**) calloc(nova->kapacitet, sizeof(Simbol*));
  if (nova->simboli == NULL) {
    exit(1);
  }

  nova->trenutni_id = 0;

  for (int i = 0; i < BROJ_SIMBOLA; i++) {
    nova->ulazi[i] = NULL;
    nova->indirect[i] = &nova->ulazi[i];
  }

  return nova;
}

static int izracunaj_indeks(const char* naziv) {

  int broj = 0;

  for (int i = 0; naziv[i]; i++) {
    broj += naziv[i];
  }

  return broj % BROJ_SIMBOLA;

} 

Simbol* proveri_postojanje_globalnog(TabelaSimbola* tabela_simbola, const char* naziv) {

  int indeks = izracunaj_indeks(naziv);

  for (UlazSimbol* tekuci = tabela_simbola->ulazi[indeks]; tekuci; tekuci = tekuci->sledeci) {
    if (strcmp(tekuci->simbol->naziv, naziv) == 0 && tekuci->simbol->tip == GLOBALNI) {
      return tekuci->simbol;
    }
  }

  return NULL;
}

static int ubaci_u_niz(TabelaSimbola* tabela_simbola, Simbol* simbol) {
  
  if (tabela_simbola->trenutni_id == tabela_simbola->kapacitet) {
    tabela_simbola->kapacitet = (tabela_simbola->kapacitet * 3) / 2;
    tabela_simbola->simboli = realloc(tabela_simbola->simboli, tabela_simbola->kapacitet);
    if (tabela_simbola->simboli == NULL) {
      exit(1);
    }
  }

  simbol->id = tabela_simbola->trenutni_id;
  tabela_simbola->simboli[tabela_simbola->trenutni_id++] = simbol;

  return simbol->id;
}

int ubaci_simbol(TabelaSimbola* tabela_simbola, Simbol* simbol) {

  int indeks = izracunaj_indeks(simbol->naziv);

  UlazSimbol* novi_ulaz = (UlazSimbol*) malloc(sizeof(UlazSimbol));
  if (novi_ulaz == NULL) {
    exit(1);
  }

  novi_ulaz->simbol = simbol;
  novi_ulaz->sledeci = NULL;

  *tabela_simbola->indirect[indeks] = novi_ulaz;
  tabela_simbola->indirect[indeks] = &novi_ulaz->sledeci;

  return ubaci_u_niz(tabela_simbola, simbol);
}

Simbol* dohvati_simbol_ime(TabelaSimbola* tabela_simbola, const char* ime) {

  int indeks = izracunaj_indeks(ime);

  for (UlazSimbol* tekuci = tabela_simbola->ulazi[indeks]; tekuci; tekuci = tekuci->sledeci) {
    if (strcmp(tekuci->simbol->naziv, ime) == 0) {
      return tekuci->simbol;
    }
  }

  return NULL;
}

Simbol* dohvati_simbol_id(TabelaSimbola* tabela_simbola, int id) {

  if (id > tabela_simbola->trenutni_id) {
    return NULL;
  }
  
  return tabela_simbola->simboli[id];
}

Simbol* provera_postoji_nedefinisan(TabelaSimbola* tabela_simbola) {

  for (int i = 1; i < tabela_simbola->trenutni_id; i++) {

    if (tabela_simbola->simboli[i]->tvf->dohvati_sekciju(tabela_simbola->simboli[i]) == SHN_UNDEF) {
      return tabela_simbola->simboli[i];
    }
  }

  return NULL;
}

void obrisi_tabelu_simbol(TabelaSimbola* ts) {


  for (int i = 0; i < ts->trenutni_id; i++) {
    ts->simboli[i]->tvf->obrisi_simbol(ts->simboli[i]);
  }

  free(ts->simboli);
  
  for (int i = 0; i < BROJ_SIMBOLA; i++) {
    while (ts->ulazi[i]) {
      UlazSimbol* stari = ts->ulazi[i];
      ts->ulazi[i] = ts->ulazi[i];
      free(stari);
    }
  }

  free(ts);
}