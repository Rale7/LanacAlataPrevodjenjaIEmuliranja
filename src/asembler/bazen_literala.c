#include <stdlib.h>
#include <stdio.h>
#include "../../inc/asembler/bazen_literala.h"
#include "../../inc/asembler/asembler.h"

static Obracanje* init_obracanja(int lokacija) {

  Obracanje* novo = (Obracanje*) malloc(sizeof(Obracanje));
  if (novo == NULL) {
    exit(1);
  }

  novo->sledeci = NULL;
  novo->lokacija = lokacija;

  return novo;

}

static UlazLiteral* init_UL(int literal, int lokacija) {

  UlazLiteral* novi = (UlazLiteral*) malloc(sizeof(UlazLiteral));
  if (novi == NULL) {
    exit(1);
  }

  novi->vrednost = literal;
  novi->sledeci = NULL;
  
  Obracanje* novo_obracanje = init_obracanja(lokacija);

  novi->prvi = novo_obracanje;
  novi->indirect = &(novo_obracanje->sledeci);

  return novi;
}

static void obrisi_UL(UlazLiteral* ul) {

  while (ul->prvi) {
    Obracanje* stari = ul->prvi;
    ul->prvi = ul->prvi->sledeci;
    free(stari);
  }

  free(ul);
}

static UlazSimbol* init_US(Simbol* simbol, int lokacija) {

  UlazSimbol* novi = (UlazSimbol*) malloc(sizeof(UlazSimbol));
  if (novi == NULL) {
    exit(1);
  }

  novi->simbol = simbol;
  novi->sledeci = NULL;
  novi->lokacija = 0;
  
  Obracanje* novo_obracanje = init_obracanja(lokacija);

  novi->prvi = novo_obracanje;
  novi->indirect = &(novo_obracanje->sledeci);

  return novi;

}

static void obrisi_US(UlazSimbol* us) {

   while (us->prvi) {
    Obracanje* stari = us->prvi;
    us->prvi = us->prvi->sledeci;
    free(stari);
  }

  free(us);

}

BazenLiterala* init_bazen() {

  BazenLiterala* novi = (BazenLiterala*) malloc(sizeof(BazenLiterala));
  if (novi == NULL) {
    exit(1);
  }

  novi->prviLiteral = NULL;
  novi->indirectLiteral = &(novi->prviLiteral);
  novi->prviSimbol = NULL;
  novi->indirectSimbol = &(novi->prviSimbol);

  return novi;
}

void obrisi_bazen(BazenLiterala* bl) {

  while (bl->prviLiteral) {
    UlazLiteral* stari = bl->prviLiteral;
    bl->prviLiteral = bl->prviLiteral->sledeci;
    obrisi_UL(stari);
  }

  while (bl->prviSimbol) {
    UlazSimbol* stari = bl->prviSimbol;
    bl->prviSimbol = bl->prviSimbol->sledeci;
    obrisi_US(stari);
  }

  free(bl);

}

int dohvati_najmanju_lokaciju(BazenLiterala* bl, int* lokacija) {

  if (bl->prviLiteral == NULL && bl->prviSimbol == NULL) {
    return -1;
  }

  if (bl->prviLiteral == NULL) {
    *lokacija = bl->prviSimbol->prvi->lokacija;
  } else if (bl->prviSimbol == NULL) {
    *lokacija = bl->prviLiteral->prvi->lokacija;
  } else {
    int a = bl->prviSimbol->prvi->lokacija;
    int b = bl->prviLiteral->prvi->lokacija;
    *lokacija = (a > b ? b : a);
  }

  return 0;
}

void dodaj_obracanje_literal(BazenLiterala* bl, int literal, int lokacija) {

  for (UlazLiteral* trenutni = bl->prviLiteral; trenutni; trenutni = trenutni->sledeci) {
    if (trenutni->vrednost == literal) {
      Obracanje* novo_obracanja = init_obracanja(lokacija);
      *(trenutni->indirect) = novo_obracanja;
      trenutni->indirect = &(novo_obracanja->sledeci);
      return;
    }
  }

  UlazLiteral* novi = init_UL(literal, lokacija);

  *(bl->indirectLiteral) = novi;
  bl->indirectLiteral = &(novi->sledeci);

}

int provera_postoji_literal(BazenLiterala* bl, int literal, int* lokacija) {

  for (UlazLiteral* trenutni = bl->prviLiteral; trenutni; trenutni = trenutni->sledeci) {
    if (trenutni->vrednost == literal) {
      *lokacija = trenutni->lokacija;
      return 1;
    }
  }

  return 0;
}

void dodaj_obracanje_simbol(BazenLiterala* bl, Simbol* sim, int lokacija) {

  for (UlazSimbol* trenutni = bl->prviSimbol; trenutni; trenutni = trenutni->sledeci) {
    if (trenutni->simbol->redosled == sim->redosled) {
      Obracanje* novo_obracanje = init_obracanja(lokacija);
      *(trenutni->indirect) = novo_obracanje;
      trenutni->indirect = &(novo_obracanje->sledeci);
      return;
    }
  }

  UlazSimbol* novi = init_US(sim, lokacija);

  *(bl->indirectSimbol) = novi;
  bl->indirectSimbol = &(novi->sledeci);

}

int provera_postoji_simbol(BazenLiterala* bl, Simbol* sim, int* lokacija) {

  for (UlazSimbol* trenutni = bl->prviSimbol; trenutni; trenutni = trenutni->sledeci) {
    if (trenutni->simbol->redosled == sim->redosled && trenutni->lokacija != 0) {
      *lokacija = trenutni->lokacija;
      return 1;
    }
  }

  return 0;
}
static void razresi_pomeraje_za_literale(Obracanje* prvi, int trenutna_lokacija, Sekcija* sekcija) {

  for (Obracanje* trenutni = prvi; trenutni; trenutni = trenutni->sledeci) {

    int pomeraj = trenutna_lokacija - (trenutni->lokacija + 4);

    char prva_vrednost = *dohvati_sadrzaj(sekcija->sadrzaj, trenutni->lokacija + 2);
    prva_vrednost = (char)(prva_vrednost & 0xF0) | ((pomeraj & 0xF00) >> 8);
    postavi_sadrzaj(sekcija->sadrzaj, trenutni->lokacija + 2, &prva_vrednost, 1);

    char druga_vrednost = (char) (pomeraj & 0xFF);
    postavi_sadrzaj(sekcija->sadrzaj, trenutni->lokacija + 3, &druga_vrednost, 1);
    
  }
}

static int razresi_pomeraje_za_simbole(UlazSimbol* us, int trenutna_lokacija, Sekcija* sekcija) {
  int popunio = 0;

  for (Obracanje* trenutni = us->prvi; trenutni; trenutni = trenutni->sledeci) {
    if (us->simbol->tvf->nrz(us->simbol, sekcija, trenutna_lokacija, trenutni->lokacija)) {
      popunio = 1;
    }
  }

  return popunio;
}

static void upisivanje_literala(BazenLiterala* bl, Sekcija* sekcija, UlazLiteral* trenutni_literal) {
  postavi_sadrzaj(sekcija->sadrzaj, sekcija->location_counter, (const char*) &trenutni_literal->vrednost, 4);
  razresi_pomeraje_za_literale(trenutni_literal->prvi, sekcija->location_counter, sekcija);
  trenutni_literal->lokacija = sekcija->location_counter;
  sekcija->location_counter += 4;
}

static void upisivanje_simbola(BazenLiterala* bl, Sekcija* sekcija, UlazSimbol* trenutni_simbol) {
  razresi_pomeraje_za_simbole(trenutni_simbol, sekcija->location_counter, sekcija);
  postavi_sadrzaj(sekcija->sadrzaj, sekcija->location_counter, (const char*) &trenutni_simbol->simbol->vrednost, 4);
  trenutni_simbol->lokacija = sekcija->location_counter;
  sekcija->location_counter += 4;
}

static void upisivanje(BazenLiterala* bl, Sekcija* sekcija) {

  UlazLiteral* trenutni_literal = bl->prviLiteral;
  UlazSimbol* trenutni_simbol = bl->prviSimbol;

  while (trenutni_literal && trenutni_simbol) {

    if (trenutni_literal->prvi->lokacija < trenutni_simbol->prvi->lokacija) {
      upisivanje_literala(bl, sekcija, trenutni_literal);
      trenutni_literal = trenutni_literal->sledeci;
    } else {
      upisivanje_simbola(bl, sekcija,trenutni_simbol);
      trenutni_simbol = trenutni_simbol->sledeci;
    }
  }

  while (trenutni_literal) {
    upisivanje_literala(bl, sekcija, trenutni_literal);
    trenutni_literal = trenutni_literal->sledeci;
  }

  while (trenutni_simbol) {
    upisivanje_simbola(bl, sekcija,trenutni_simbol);
    trenutni_simbol = trenutni_simbol->sledeci;
  }
}

void dodaj_jmp_instrukciju(int lokacija_inst, int lokacija_skoka, SadrzajSekcije* ss) {

  char oc = 0x30;
  postavi_sadrzaj(ss, lokacija_inst, &oc, 1);

  char registri = (char)(0xF << 4);
  postavi_sadrzaj(ss, lokacija_inst + 1, &oc, 1);

  int pomeraj = lokacija_skoka - (lokacija_inst + 4);
  char prvi_deo = (char)((pomeraj & 0xF00) >> 8);
  char drugi_deo = (char)(pomeraj & 0xFF);

  postavi_sadrzaj(ss, lokacija_inst + 2, &prvi_deo, 1);
  postavi_sadrzaj(ss, lokacija_inst + 3, &drugi_deo, 1);

  printf("Dodata instrukcija skoka zbog bazena literala\n");
}

void upisi_bazen(BazenLiterala* bl, int kraj_sekcije) {

  Sekcija* sekcija = dohvati_asembler()->trenutna_sekcija;
  int stari_lc;

  if (!kraj_sekcije) {
    stari_lc = sekcija->location_counter;
    sekcija->location_counter += 4;
  }

  upisivanje(bl, sekcija);

  if (!kraj_sekcije) {
    dodaj_jmp_instrukciju(stari_lc, sekcija->location_counter, sekcija->sadrzaj);
  }
}