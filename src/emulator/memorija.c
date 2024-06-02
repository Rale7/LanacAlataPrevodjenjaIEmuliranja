#include <stdio.h>
#include <stdlib.h>
#include "../../inc/emulator/memorija.h"

#define DEFAULT_SEGMENT_SIZE 400U

int dohvati_vrednost_memorija(Segment* segment, unsigned int adresa) {

  return segment->sadrzaj[adresa - segment->pocetna_adresa];
}

void postavi_vrednost_memorija(Segment* segment, unsigned int adresa, int vrednost) {
  segment->sadrzaj[adresa - segment->pocetna_adresa] = vrednost;
}

Segment* init_segment(int pocetna_adresa, int krajnja_adresa) {
  static SegmentTVF memorija_tvf = {
    .dohvati_vrednost = &dohvati_vrednost_memorija,
    .postavi_vrednost = &postavi_vrednost_memorija
  }; 

  Segment* novi = (Segment*) malloc(sizeof(Segment));
  if (novi == NULL) {
    print("Greska pri alokaciji memorie\n");
    exit(1);
  }

  novi->desni = NULL;
  novi->levi = NULL;
  novi->pocetna_adresa = pocetna_adresa;
  novi->krajnja_adresa = krajnja_adresa;
  novi->visina = 1;
  novi->tvf = memorija_tvf;
  novi->sadrzaj = (int*) calloc(krajnja_adresa - pocetna_adresa, sizeof(int));
  if (novi->sadrzaj == NULL) {
    printf("Greska u alokaciji memroije\n");
    exit(1);
  }

  return novi;
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

static int uporedi(Segment* prvi, Segment* drugi) {

  return prvi->pocetna_adresa - drugi->pocetna_adresa;
}

static Segment* ubaci(Segment* cvor, Segment* novi) {

  if (cvor == NULL) {
    return novi;
  }

  if (uporedi(cvor, novi) < 0) {
    cvor->levi = ubaci(cvor->levi, novi);
  } else {
    cvor->desni = ubaci(cvor->desni, novi);
  }

  cvor->visina = 1 + max(visina(cvor->levi), visina(cvor->desni));

  int balans = dohvati_balans(cvor);

  if (balans > 1 && uporedi(cvor, novi) < 0) {
    return desna_rotacija(cvor);
  }

  if (balans < -1 && uporedi(cvor, novi) > 0) {
    return leva_rotacija(cvor);
  }

  if (balans > 1 && uporedi(cvor, novi) > 0) {
    cvor->levi = leva_rotacija(cvor->levi);
    return desna_rotacija(cvor);
  }

  if (balans < -1 && uporedi(cvor, novi) < 0) {
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
    
    if (tekuci->pocetna_adresa < adresa && adresa < tekuci->krajnja_adresa) {
      return tekuci;
    } else if (tekuci->pocetna_adresa < adresa) {
      sledbenik = tekuci;
      tekuci = tekuci->levi;
    } else {
      prethodnik = tekuci;
      tekuci = tekuci->desni;
    }
  }

  if (prethodnik && sledbenik) {

    if (sledbenik->pocetna_adresa - prethodnik->krajnja_adresa < DEFAULT_SEGMENT_SIZE) {
      ubaci_segment(memorija, init_segment(prethodnik->krajnja_adresa + 1, sledbenik->pocetna_adresa - 1));
    } else {
      ubaci(memorija, init_segment(
        max(prethodnik->krajnja_adresa - 1, adresa - DEFAULT_SEGMENT_SIZE / 2),
        min(sledbenik->pocetna_adresa - 1, adresa + DEFAULT_SEGMENT_SIZE / 2)  
      ));
    }
  } else if (sledbenik) {
    if (sledbenik->pocetna_adresa < DEFAULT_SEGMENT_SIZE) {
      ubaci_segment(memorija, init_segment(0, sledbenik->pocetna_adresa - 1));
    } else {
      ubaci(memorija, init_segment(
        max(0, adresa - DEFAULT_SEGMENT_SIZE / 2),
        min(sledbenik->pocetna_adresa - 1, adresa + DEFAULT_SEGMENT_SIZE / 2)
      ));
    }
  } else if (prethodnik) {
    if (0xFFFFFFFFU - prethodnik->pocetna_adresa < DEFAULT_SEGMENT_SIZE) {
      ubaci_segment(memorija, init_segment(prethodnik->pocetna_adresa + 1, 0xFFFFFFFFU));
    } else {
      ubaci(memorija, init_segment(
        min(prethodnik->krajnja_adresa + 1, adresa + DEFAULT_SEGMENT_SIZE / 2),
        max(0xFFFFFFFFU,adresa + DEFAULT_SEGMENT_SIZE / 2) 
      ));
    }
  } else {
    ubaci(memorija, init_segment(adresa - DEFAULT_SEGMENT_SIZE / 2, adresa + DEFAULT_SEGMENT_SIZE / 2));
  }

  return pretrazi(memorija, adresa);
  
}

int dohvati_vrednost(Memorija* memorija, unsigned int adresa) {

  Segment* trazeni = pretrazi(memorija, adresa);
  return trazeni->tvf.dohvati_vrednost(trazeni, adresa);
}

void postavi_vrednost(Memorija* memorija, unsigned int adresa, int vrednost) {

  Segment* trazeni = pretrazi(memorija, adresa);
  trazeni->tvf.postavi_vrednost(memorija, adresa, vrednost);
}

