#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../inc/asembler/direktive.h"
#include "../../inc/asembler/asembler.h"
#include "../../inc/asembler/simbol.h"
#include "../../inc/asembler/relokacioni_zapis.h"

void dodaj_labelu(const char* naziv_simbola) {

  TabelaSimbola* ts = dohvati_asembler()->tabel_simbola;
  Simbol* simbol;
  Sekcija* sekcija = dohvati_asembler()->trenutna_sekcija;

  if (sekcija == NULL) {
    printf("Labela mora biti definisana u sekciji");
  }

  if ((simbol = dohvati_vrednost_simbola(ts, naziv_simbola)) == NULL) {
    simbol = init_nedefinisan_simbol(naziv_simbola);
    dodaj_simbol(ts, simbol);
  }
  
  prebaci_u_definisan(simbol, sekcija);
}

void global_dir(const char* naziv_simbola) {

  TabelaSimbola* ts = dohvati_asembler()->tabel_simbola;
  Simbol* simbol;

  if ((simbol = dohvati_vrednost_simbola(ts, naziv_simbola)) == NULL) {
    simbol = init_nedefinisan_simbol(naziv_simbola);
    dodaj_simbol(ts, simbol);
  }

  prebaci_u_globalni(simbol);
}

void extern_dir(const char* naziv_simbola) {

  TabelaSimbola* ts = dohvati_asembler()->tabel_simbola;
  Simbol* simbol;

  if ((simbol = dohvati_vrednost_simbola(ts, naziv_simbola)) == NULL) {
    simbol = init_definisan_simbol(naziv_simbola, 0, NULL);
    dodaj_simbol(ts, simbol);
  } else if (simbol->sekcija != NULL && simbol->sekcija->simbol->redosled != 0) {
    printf("Greska visestruko definisanje simbola %s\n", naziv_simbola);
    exit(1);
  }

  simbol->sekcija = dohvati_asembler()->undefined;
  prebaci_u_globalni(simbol);

}

void section_dir(const char* sekcija) {

  napravi_novu_sekciju(dohvati_asembler(), sekcija);

}

void word_dir_sim(const char* naziv_simbola) {

  TabelaSimbola* ts = dohvati_asembler()->tabel_simbola;
  Simbol* simbol;
  Sekcija* trenutna_sekcija = dohvati_asembler()->trenutna_sekcija;

  if (trenutna_sekcija == NULL) {
    printf("direktiva word se mora staviti unutar neke sekcije\n");
    exit(1);
  }

  if ((simbol = dohvati_vrednost_simbola(ts, naziv_simbola)) == NULL) {
    simbol = init_nedefinisan_simbol(naziv_simbola);
    dodaj_simbol(ts, simbol);
  }
  
  simbol->tvf->sdw(simbol, trenutna_sekcija, trenutna_sekcija->location_counter);
  trenutna_sekcija->location_counter += 4;
}

void word_dir_literal(int literal) {

  Sekcija* trenutna = dohvati_asembler()->trenutna_sekcija;

  if (trenutna == NULL) {
    printf("direktiva word se mora staviti unutar neke sekcije\n");
    exit(1);
  }

  postavi_sadrzaj(trenutna->sadrzaj, trenutna->location_counter, (const char*) &literal, sizeof(int));
  trenutna->location_counter += sizeof(int);

}

void skip_dir(int literal) {

  Sekcija* trenutna = dohvati_asembler()->trenutna_sekcija;

  if (trenutna == NULL) {
    printf("direktiva skip se mora staviti unutar neke sekcije\n");
    exit(1);
  }

  popuni_nulama(trenutna->sadrzaj, trenutna->location_counter, literal);
  trenutna->location_counter += literal;

}

void ascii_dir(const char* string) {

  Sekcija* trenutna = dohvati_asembler()->trenutna_sekcija;

  if (trenutna == NULL) {
    printf("direktiva ascii se mora staviti unutar neke sekcije\n");
    exit(1);
  }

  int n = strlen(string) + 1;

  postavi_sadrzaj(trenutna->sadrzaj, trenutna->location_counter, string, n);

  trenutna->location_counter += n;

}

void end_dir() {

  Asembler* asembler = dohvati_asembler();

  ispisi_simbole();

  for (SekcijaElem* sekcija = asembler->sekcije; sekcija; sekcija = sekcija->sledeci) {
    Sekcija* moja_sekcija = sekcija->sekcija;

    printf("Sekcija %s", moja_sekcija->simbol->naziv);

    for (int i = 0; i < moja_sekcija->location_counter; i++) {
      if (i % 4 == 0) printf("\n%d:\t", i);
      printf("0x%02hhx ", moja_sekcija->sadrzaj->byte[i]);
    }

    printf("\nRelokacinoni zapisi za sekciju %s\n", moja_sekcija->simbol->naziv);
    printf("offset\tsimbol\taddend\n");
    for (RelokacioniZapis* rz = moja_sekcija->trz->prvi; rz; rz = rz->sledeci) {
      rz->simbol->tip_tvf->ispis_relokacionog_zapisa(rz->simbol, rz);
    }
    printf("\n");
  }

}