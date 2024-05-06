#include <stdio.h>
#include <stdlib.h>
#include "../../inc/asembler/tabela_neizracunjivih_simbola.h"
#include "../../inc/asembler/izraz.h"
#include "../../inc/asembler/sekcija.h"

TNS* init_TNS() {

  TNS* novi = (TNS*) malloc(sizeof(TNS));
  if (novi == NULL) {
    exit(1);
  }

  novi->broj_simbola = 0;
  novi->prvi = NULL;
}

static NeizracunjiviElem* init_NE(NeizracunjiviSimbol* ns) {
  
  NeizracunjiviElem* novi = (NeizracunjiviElem*) malloc(sizeof(NeizracunjiviElem));
  if (novi == NULL) {
    exit(1);
  }

  novi->sledeci = NULL;
  novi->simbol = ns;

  return novi;
}

void obrisi_TNS(TNS* tns) {

  while (tns->prvi) {
    NeizracunjiviElem* stari = tns->prvi;
    tns->prvi = tns->prvi->sledeci;
    obrisi_izraz(stari->simbol->izraz);
    free(stari->simbol);
    free(stari);
  }

  free(tns);

}

void dodaj_neizracunjivi_simbol(TNS* tns, NeizracunjiviSimbol* simbol) {

  NeizracunjiviElem* ne = init_NE(simbol);
  ne->sledeci = tns->prvi;
  tns->prvi = ne;
  tns->broj_simbola++;
}

typedef struct stek_NE {
  struct stek_NE* sledeci;
  NeizracunjiviSimbol* ns;
} StekNE;

static StekNE* init_stek_NE(NeizracunjiviSimbol* ns) {

  StekNE* novi = (StekNE*) malloc(sizeof(StekNE));
  if (novi == NULL) {
    exit(1);
  }

  novi->ns = ns;
  return novi;
}

static void stek_push_NE(StekNE** vrh, NeizracunjiviSimbol* ns) {

  StekNE* novi = init_stek_NE(ns);
  novi->sledeci = *vrh;
  *vrh = novi;
}

static NeizracunjiviSimbol* stek_pop_NE(StekNE** vrh) {
  StekNE* stari = *vrh;
  *vrh = (*vrh)->sledeci;
  NeizracunjiviSimbol* ret = stari->ns;
  free(stari);
  return ret;
}

static int prazan_stek_NE(StekNE* vrh) {
  return vrh == NULL;
}

int topolosko_sortiranje_util(NeizracunjiviElem**graf, NeizracunjiviSimbol** simboli,
 int indeks, int* obidjeni, StekNE** stek, int* rek_stek) {

  if (!obidjeni[indeks]) {
    obidjeni[indeks] = 1;
    rek_stek[indeks] = 1;

    for (NeizracunjiviElem* tekuci = graf[indeks]; tekuci; tekuci = tekuci->sledeci) {
      
      if (!obidjeni[tekuci->simbol->neizracunjivi_id] &&
        topolosko_sortiranje_util(graf, simboli, tekuci->simbol->neizracunjivi_id, obidjeni, stek, rek_stek)) {
          return 1;
      } else if (rek_stek[tekuci->simbol->neizracunjivi_id]) {
        return 1;
      }
    }

  }

  stek_push_NE(stek, simboli[indeks]);

  rek_stek[indeks] = 0;
  return 0;
}

void topolosko_sortiranje(NeizracunjiviElem** graf, NeizracunjiviSimbol** simboli, int velicina) {
  StekNE* stek = NULL;
  int* obidjeni = (int*) calloc(velicina, sizeof(int));
  int* rec_stek = (int*) calloc(velicina, sizeof(int));

  for (int i = 0; i < velicina; i++) {
    if (!obidjeni[i]) {
      int status = topolosko_sortiranje_util(graf, simboli, i, obidjeni, &stek, rec_stek);
      if (status == 1) {
        printf("Simboli su medjusobno ciklicno zavisni jedni od drugih\n");
        exit(1);
      }
    }
  }

  while (!prazan_stek_NE(stek)) {
    NeizracunjiviSimbol* ns = stek_pop_NE(&stek);
    Sekcija* sekcija_rel;
    enum Relokatibilnost status = proveri_relokatibilnost(ns->izraz, &sekcija_rel);

    if (status == APSOLUTAN) {
      prebaci_u_simbolicku_konstantu(ns->simbol, izracunaj_vrednost_izraza(ns->izraz));
      razresavanje_neizracunjivog_simbola_konstanta(ns->simbol);
    } else if (status == RELOKATIVAN) {
      prebaci_u_definisan(ns->simbol, sekcija_rel, izracunaj_vrednost_izraza(ns->izraz));
    }
  }

  free(obidjeni);
  free(rec_stek);
}

void razresi_TNS(TNS* tns) {

  NeizracunjiviElem** graf = (NeizracunjiviElem**) calloc(tns->broj_simbola, sizeof(NeizracunjiviElem*));
  NeizracunjiviSimbol** simboli = (NeizracunjiviSimbol**) calloc(tns->broj_simbola,sizeof(NeizracunjiviSimbol));
  if (graf == NULL) {
    exit(1);
  }

  int cnt = 0;
  for (NeizracunjiviElem* tekuci = tns->prvi; tekuci; tekuci = tekuci->sledeci) {
      simboli[tns->broj_simbola - ++cnt] = tekuci->simbol;

      for (ClanIzraza* clan = tekuci->simbol->izraz->prvi; clan; clan = clan->sledeci) {

        int indeks;
        if (clan->klasifikator == SIMBOL 
        && (indeks = clan->deo.simbol->tvf->neizracunjivi_indeks(clan->deo.simbol)) != -1) {

          NeizracunjiviElem* ne = init_NE(tekuci->simbol);
          ne->sledeci = graf[indeks];
          graf[indeks] = ne;
        }
      }
  }

  topolosko_sortiranje(graf, simboli, tns->broj_simbola);

  for (int i = 0; i < tns->broj_simbola; i++) {
    while (graf[i]) {
      NeizracunjiviElem* stari = graf[i];
      graf[i] = graf[i]->sledeci;
      free(stari);
    }
  }
  free(graf);
  free(simboli);
}