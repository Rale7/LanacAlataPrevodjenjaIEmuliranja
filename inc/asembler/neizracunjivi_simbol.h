#ifndef NEIZRACUNJIVI_SIMBOLI
#define NEIZRACUNJIVI_SIMBOLI

#include "simbol.h"

struct simbol;

enum Operator {SABIRANJE, ODUZIMANJE, MNOZENJE, DELJENJE};

typedef struct izraz
{
  
  enum Operator oper;
  struct simbol* simbol;
  struct izraz* sledeci;

} Izraz;

typedef struct {

  struct simbol* simbol;
  Izraz* prvi;
  Izraz** indirect;

} NeizracunjiviSimbol;

NeizracunjiviSimbol* init_ns(struct simbol*);

void dodaj_izraz(NeizracunjiviSimbol*, enum Operator, struct simbol*);

int razresi_izraze(NeizracunjiviSimbol*);

#endif