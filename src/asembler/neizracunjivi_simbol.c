#include <stdlib.h>
#include "../../inc/asembler/neizracunjivi_simbol.h"

NeizracunjiviSimbol* init_ns(Simbol* s) {

  NeizracunjiviSimbol* novi = (NeizracunjiviSimbol*) malloc(sizeof(NeizracunjiviSimbol));
  if (novi == NULL) {
    exit(1);
  }

  novi->simbol = s;
  novi->prvi = NULL;
  novi->indirect = &(novi->prvi);

  return novi;

}

void dodaj_izraz(NeizracunjiviSimbol* ns,enum Operator op, Simbol* simbol) {

  Izraz* izraz = (Izraz*) malloc(sizeof(Izraz));

  if (izraz == NULL) {
    exit(1);
  }

  izraz->simbol = simbol;
  izraz->oper = op;
  izraz->sledeci = NULL;

  *(ns->indirect) = izraz->sledeci;
  ns->indirect = &(izraz->sledeci);

}

int razresi_izraze(NeizracunjiviSimbol* ns) {
  // TODO
  return 0;

}