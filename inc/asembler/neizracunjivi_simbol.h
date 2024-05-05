#ifndef NEIZRACUNJIVI_SIMBOLI
#define NEIZRACUNJIVI_SIMBOLI

#include "simbol.h"

struct izraz;

typedef struct obracanje_instrukcije {
  int lokacija;
  struct sekcija* sekcija;
  struct obracanje_instrukcije* sledeci;
} ObracanjeInstrukcije;

typedef struct neizracunjivi_simbol {
  Simbol simbol;
  int neizracunjivi_id;
  struct izraz* izraz;
  ObracanjeInstrukcije* prvi;
} NeizracunjiviSimbol;

NeizracunjiviSimbol* init_neizracunjivi_simbol(const char*, struct sekcija*, struct izraz*);

Simbol* prebaci_u_neizracunjiv(Simbol* simbol, struct izraz* izraz);

#endif