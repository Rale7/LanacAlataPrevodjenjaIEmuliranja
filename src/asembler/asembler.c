#include "asembler/asembler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "asembler/bazen_literala.h"
#include "asembler/tabela_neizracunjivih_simbola.h"

static Asembler *init_asembler() {
  Asembler *asembler = (Asembler *)malloc(sizeof(Asembler));
  if (asembler == NULL) {
    exit(1);
  }

  asembler->tabel_simbola = init_ts();
  asembler->tabela_neizrazunljivih_simbola = init_TNS();
  asembler->sekcije = NULL;
  asembler->indirect = &(asembler->sekcije);

  asembler->undefined = init_sekcija("UND", NULL);

  return asembler;
}

Asembler *dohvati_asembler() {
  static Asembler *primerak = NULL;

  if (primerak == NULL) {
    primerak = init_asembler();
  }

  return primerak;
}

Sekcija *napravi_novu_sekciju(Asembler *asembler, const char *ime) {
  if (dohvati_vrednost_simbola(asembler->tabel_simbola, ime) != NULL) {
    printf("Greska simbol %s vec postoji\n", ime);
    exit(1);
  }

  if (asembler->trenutna_sekcija != NULL) {
    upisi_bazen(asembler->trenutna_sekcija->bazen_literala, 1);
    obrisi_bazen(asembler->trenutna_sekcija->bazen_literala);
  }

  asembler->trenutna_sekcija = init_sekcija(ime, NULL);

  SekcijaElem *elem = (SekcijaElem *)malloc(sizeof(SekcijaElem));
  if (elem == NULL) {
    exit(1);
  }

  elem->sekcija = asembler->trenutna_sekcija;
  elem->sledeci = NULL;

  *(asembler->indirect) = elem;
  asembler->indirect = &(elem->sledeci);

  return asembler->trenutna_sekcija;
}

void obrisi_asembler(Asembler *asembler) {
  while (asembler->sekcije) {
    SekcijaElem *stari = asembler->sekcije;
    asembler->sekcije = asembler->sekcije->sledeci;
    obrisi_sekciju(stari->sekcija);
    free(stari);
  }

  obrisi_bazen(asembler->undefined->bazen_literala);
  obrisi_sekciju(asembler->undefined);

  obrisi_TNS(asembler->tabela_neizrazunljivih_simbola);
  obrisi_tabelu_simbola(asembler->tabel_simbola);

  obrisi_simbole();

  free(asembler);
}
