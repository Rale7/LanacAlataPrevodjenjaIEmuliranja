#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../inc/asembler/direktive.h"
#include "../../inc/asembler/asembler.h"
#include "../../inc/asembler/simbol.h"
#include "../../inc/asembler/relokacioni_zapis.h"
#include "../../inc/asembler/bazen_literala.h"
#include "../../inc/asembler/neizracunjivi_simbol.h"
#include "../../inc/asembler/tabela_neizracunjivih_simbola.h"

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
  
  prebaci_u_definisan(simbol, sekcija, sekcija->location_counter);
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

static void provera_prekoracenja_bazena_dir(int prostor) {

  Sekcija* trenutna = dohvati_asembler()->trenutna_sekcija;

  int najmanje_obracanje;
  if (dohvati_najmanju_lokaciju(trenutna->bazen_literala, &najmanje_obracanje) == 0 &&
  trenutna->location_counter + prostor - najmanje_obracanje >= 2036) {

    if (trenutna->prethodni_bl != NULL) {
      obrisi_bazen(trenutna->prethodni_bl);
    }

    trenutna->prethodni_bl = trenutna->bazen_literala;
    upisi_bazen(trenutna->bazen_literala, 0);
    trenutna->bazen_literala = init_bazen();
  }

}

void word_dir_sim(const char* naziv_simbola) {

  provera_prekoracenja_bazena_dir(4);

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

  provera_prekoracenja_bazena_dir(4);

  Sekcija* trenutna = dohvati_asembler()->trenutna_sekcija;

  if (trenutna == NULL) {
    printf("direktiva word se mora staviti unutar neke sekcije\n");
    exit(1);
  }

  postavi_sadrzaj(trenutna->sadrzaj, trenutna->location_counter, (const char*) &literal, sizeof(int));
  trenutna->location_counter += sizeof(int);

}

void skip_dir(int literal) {

  provera_prekoracenja_bazena_dir(literal);

  Sekcija* trenutna = dohvati_asembler()->trenutna_sekcija;

  if (trenutna == NULL) {
    printf("direktiva skip se mora staviti unutar neke sekcije\n");
    exit(1);
  }

  popuni_nulama(trenutna->sadrzaj, trenutna->location_counter, literal);
  trenutna->location_counter += literal;

}

void equ_dir(const char* naziv_simbola, Izraz* izraz) {

  prebaci_postfix(izraz);
  Simbol *simbol;

  if ((dohvati_vrednost_simbola(dohvati_asembler()->tabel_simbola, naziv_simbola)) != NULL) {
    prebaci_u_neizracunjiv(simbol, izraz);
  } else {
    Simbol* simbol = proveri_relokatibilnost_init_simbol(izraz, dohvati_asembler()->trenutna_sekcija, naziv_simbola);
    dodaj_simbol(dohvati_asembler()->tabel_simbola, simbol);
  }

  printf("\n");
}

void ascii_dir(const char* string) {

  int n = strlen(string) + 1;
  provera_prekoracenja_bazena_dir(n);

  Sekcija* trenutna = dohvati_asembler()->trenutna_sekcija;

  if (trenutna == NULL) {
    printf("direktiva ascii se mora staviti unutar neke sekcije\n");
    exit(1);
  }

  postavi_sadrzaj(trenutna->sadrzaj, trenutna->location_counter, string, n);

  trenutna->location_counter += n;

}

void end_dir() {

  Asembler* asembler = dohvati_asembler();

  if (asembler->trenutna_sekcija != NULL) {
    upisi_bazen(asembler->trenutna_sekcija->bazen_literala, 1);
    obrisi_bazen(asembler->trenutna_sekcija->bazen_literala);
  }

  razresi_TNS(dohvati_asembler()->tabela_neizrazunljivih_simbola);

  ispisi_simbole();

  for (SekcijaElem* sekcija = asembler->sekcije; sekcija; sekcija = sekcija->sledeci) {
    Sekcija* moja_sekcija = sekcija->sekcija;

    printf("Sekcija %s", moja_sekcija->simbol->naziv);

    for (int i = 0; i < moja_sekcija->location_counter; i++) {
      if (i % 4 == 0) printf("\n%-5d:\t", i);
      printf("0x%02hhx ", moja_sekcija->sadrzaj->byte[i]);
    }

    printf("\nRelokacinoni zapisi za sekciju %s\n", moja_sekcija->simbol->naziv);
    printf("offset\t\tsimbol\t\taddend\n");
    for (RelokacioniZapis* rz = moja_sekcija->trz->prvi; rz; rz = rz->sledeci) {
      rz->simbol->tip_tvf->ispis_relokacionog_zapisa(rz->simbol, rz);
      printf("\n");
    }
    printf("\n");
  }

}