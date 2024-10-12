#include "asembler/neizracunjivi_simbol.h"

#include <stdio.h>
#include <stdlib.h>

#include "asembler/asembler.h"
#include "asembler/instrukcije.h"
#include "asembler/izraz.h"
#include "asembler/relokacioni_zapis.h"
#include "asembler/sekcija.h"
#include "asembler/tabela_neizracunjivih_simbola.h"

extern Simbol_TVF nedefinisan_simbol_tvf;

static int neizracunjivi_simbol_indeks(Simbol* simbol) {
  NeizracunjiviSimbol* ns = simbol->neizracunjivi;

  return ns->neizracunjivi_id;
}

static int inicijalizator = 0;

NeizracunjiviSimbol* init_neizracunjivi_simbol(const char* naziv_simbola,
                                               Sekcija* sekcija, Izraz* izraz) {
  Simbol* simbol = init_simbol(naziv_simbola, 0, sekcija);
  NeizracunjiviSimbol* novi =
      (NeizracunjiviSimbol*)malloc(sizeof(NeizracunjiviSimbol));
  if (novi == NULL) {
    exit(1);
  }

  novi->simbol = simbol;
  novi->simbol->neizracunjivi = novi;
  novi->simbol->tvf = &nedefinisan_simbol_tvf;

  novi->neizracunjivi_id = inicijalizator++;
  novi->izraz = izraz;

  return novi;
}

Simbol* prebaci_u_neizracunjiv(Simbol* simbol, Izraz* izraz, Sekcija* sekcija) {
  NeizracunjiviSimbol* neizracunjivi_simbol =
      (NeizracunjiviSimbol*)malloc(sizeof(NeizracunjiviSimbol));
  if (neizracunjivi_simbol == NULL) {
    exit(1);
  }

  neizracunjivi_simbol->simbol = simbol;
  simbol->neizracunjivi = neizracunjivi_simbol;
  neizracunjivi_simbol->simbol->sekcija = sekcija;

  dodaj_neizracunjivi_simbol(dohvati_asembler()->tabela_neizrazunljivih_simbola,
                             neizracunjivi_simbol);

  neizracunjivi_simbol->izraz = izraz;
  neizracunjivi_simbol->neizracunjivi_id = inicijalizator++;
  neizracunjivi_simbol->simbol->tvf = &nedefinisan_simbol_tvf;

  return (Simbol*)neizracunjivi_simbol;
}