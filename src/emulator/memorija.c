#include <stdio.h>
#include <stdlib.h>
#include "../../inc/emulator/memorija.h"

#define DEFAULT_SEGMENT_SIZE 400U

static int dohvati_vrednost_memorija(Segment* segment, unsigned int adresa) {

  int* adresa_sadrzaja = (int*)(segment->sadrzaj + adresa - segment->pocetna_adresa);

  return *adresa_sadrzaja;
}

static void postavi_vrednost_memorija(Segment* segment, unsigned int adresa, int vrednost) {
  *((int*)(segment->sadrzaj + adresa - segment->pocetna_adresa)) = vrednost;
}

static void obrisi_obican_segment(Segment* segment) {
  free(segment->sadrzaj);
  free(segment);
}

Segment* init_segment_sadrzaj(unsigned int pocetna_adresa, unsigned int krajnja_adresa, char* sadrzaj) {
  static SegmentTVF memorija_tvf = {
    .dohvati_vrednost = &dohvati_vrednost_memorija,
    .postavi_vrednost = &postavi_vrednost_memorija,
    .obrisi_segment = &obrisi_obican_segment,
  }; 

  Segment* novi = (Segment*) malloc(sizeof(Segment));
  if (novi == NULL) {
    perror("Greska pri alokaciji memorie\n");
    exit(1);
  }

  novi->desni = NULL;
  novi->levi = NULL;
  novi->pocetna_adresa = pocetna_adresa;
  novi->krajnja_adresa = krajnja_adresa;
  novi->visina = 1;
  novi->tvf = &memorija_tvf;
  novi->sadrzaj = sadrzaj;

  return novi; 
}

Segment* init_segment(unsigned int pocetna_adresa, unsigned int krajnja_adresa) {


  char* sadrzaj = (char*) calloc(krajnja_adresa - pocetna_adresa, sizeof(char));
  if (sadrzaj == NULL) {
    printf("Greska u alokaciji memorije\n");
    exit(1);
  }

  return init_segment_sadrzaj(pocetna_adresa, krajnja_adresa, sadrzaj);
}

Memorija* init_memorija() {

  Memorija* nova = (Memorija*) malloc(sizeof(Memorija));
  if (nova == NULL) {
    perror("Greska u alokaciji\n");
    exit(1);
  }

  nova->koren = NULL;

  return nova;
}

static inline int max(int a, int b) {
  return a > b ? a : b;
}
static inline int min(int a, int b) {
  return a < b ? a : b;
}
int visina(Segment* segment) {
  if (segment == NULL) {
    return 0;
  }
  return segment->visina;
}

int dohvati_balans(Segment* segment) {
  if (segment != NULL) {
    return visina(segment->levi) - visina(segment->desni);
  } else {
    return 0;
  }
}

static Segment* leva_rotacija(Segment* x) {
  Segment* y = x->desni;
  Segment* T2 = y->levi;

  y->levi = x;
  x->desni = T2;

  x->visina = max(visina(x->levi), visina(x->desni)) + 1;
  y->visina = max(visina(y->levi), visina(y->desni)) + 1;

  return y;
}

static Segment* desna_rotacija(Segment* y) {
  Segment* x = y->levi;
  Segment* T2 = x->desni;

  x->desni = y;
  y->levi = T2;

  x->visina = max(visina(x->levi), visina(x->desni)) + 1;
  y->visina = max(visina(y->levi), visina(y->desni)) + 1;

  return x;
}

static Segment* ubaci(Segment* cvor, Segment* novi) {

  if (cvor == NULL) {
    return novi;
  }

  if (novi->pocetna_adresa < cvor->pocetna_adresa) {
    cvor->levi = ubaci(cvor->levi, novi);
  } else {
    cvor->desni = ubaci(cvor->desni, novi);
  }

  cvor->visina = 1 + max(visina(cvor->levi), visina(cvor->desni));

  int balans = dohvati_balans(cvor);

  if (balans > 1 && novi->pocetna_adresa < cvor->pocetna_adresa) {
    return desna_rotacija(cvor);
  }

  if (balans < -1 && novi->pocetna_adresa < cvor->pocetna_adresa) {
    return leva_rotacija(cvor);
  }

  if (balans > 1 && novi->pocetna_adresa < cvor->pocetna_adresa) {
    cvor->levi = leva_rotacija(cvor->levi);
    return desna_rotacija(cvor);
  }

  if (balans < -1 && novi->pocetna_adresa < cvor->pocetna_adresa) {
    cvor->desni = desna_rotacija(cvor->desni);
    return leva_rotacija(cvor);
  }

  return cvor; 
}

void ubaci_segment(Memorija* memorija, Segment* novi_segment) {
  memorija->koren = ubaci(memorija->koren, novi_segment);
}

static Segment* pretrazi(Memorija* memorija, unsigned int adresa) {

  Segment* tekuci = memorija->koren;
  Segment* prethodnik = NULL;
  Segment* sledbenik = NULL;

  while (tekuci) {
    
    if (tekuci->pocetna_adresa <= adresa && adresa <= tekuci->krajnja_adresa) {
      return tekuci;
    } else if (tekuci->pocetna_adresa > adresa) {
      sledbenik = tekuci;
      tekuci = tekuci->levi;
    } else {
      prethodnik = tekuci;
      tekuci = tekuci->desni;
    }
  }

  Segment* novi = NULL;
  if (prethodnik && sledbenik) {
    if (sledbenik->pocetna_adresa - prethodnik->krajnja_adresa < DEFAULT_SEGMENT_SIZE) {
      ubaci_segment(memorija, (novi = init_segment(prethodnik->krajnja_adresa + 1, sledbenik->pocetna_adresa - 1)));
    } else {
      ubaci_segment(memorija, (
        novi = init_segment(
        max(prethodnik->krajnja_adresa + 1, adresa - DEFAULT_SEGMENT_SIZE / 2),
        min(sledbenik->pocetna_adresa - 1, adresa + DEFAULT_SEGMENT_SIZE / 2)  
      )));
    }
  } else if (sledbenik) {
    if (sledbenik->pocetna_adresa < DEFAULT_SEGMENT_SIZE) {
      ubaci_segment(memorija, (novi = init_segment(0, sledbenik->pocetna_adresa - 1)));
    } else {
      ubaci_segment(memorija, novi = init_segment(
        max(0, adresa - DEFAULT_SEGMENT_SIZE / 2),
        min(sledbenik->pocetna_adresa - 1, adresa + DEFAULT_SEGMENT_SIZE / 2)
      ));
    }
  } else if (prethodnik) {
    if (0xFFFFFEFFU - prethodnik->pocetna_adresa < DEFAULT_SEGMENT_SIZE) {
      ubaci_segment(memorija, (novi = init_segment(prethodnik->pocetna_adresa + 1, 0xFFFFFEFFU)));
    } else {
      ubaci_segment(memorija, novi = init_segment(
        max(prethodnik->krajnja_adresa + 1, adresa + DEFAULT_SEGMENT_SIZE / 2),
        min(0xFFFFFEFFU,adresa + DEFAULT_SEGMENT_SIZE / 2) 
      ));
    }
  } else {
    ubaci_segment(memorija, novi = init_segment(adresa - DEFAULT_SEGMENT_SIZE / 2, adresa + DEFAULT_SEGMENT_SIZE / 2));
  }

  return pretrazi(memorija, adresa);
  
}

int dohvati_vrednost(Memorija* memorija, unsigned int adresa) {

  Segment* trazeni = pretrazi(memorija, adresa);
  return trazeni->tvf->dohvati_vrednost(trazeni, adresa);
}

void postavi_vrednost(Memorija* memorija, unsigned int adresa, int vrednost) {

  Segment* trazeni = pretrazi(memorija, adresa);
  trazeni->tvf->postavi_vrednost(trazeni, adresa, vrednost);
}

void inorder_memorija(Segment* segment) {
  if (segment != NULL) {
    inorder_memorija(segment->levi);
    printf("Segment pocetna-%x, kranja-%x\n", segment->pocetna_adresa, segment->krajnja_adresa);
    inorder_memorija(segment->desni);
  }
}

void obrisi_segment_sinove(Segment* cvor) {
  if (cvor) {
    Segment* stari = cvor;
    obrisi_segment_sinove(cvor->levi);
    obrisi_segment_sinove(cvor->desni);
    stari->tvf->obrisi_segment(stari);
  }
}

void obrisi_memoriju(Memorija* memorija) {
  obrisi_segment_sinove(memorija->koren);
  free(memorija);
}