#ifndef TABELA_NEIZRACUNJIVIH_SIMBOLA_H
#define TABELA_NEIZRACUNJIVIH_SIMBOLA_H

#include "neizracunjivi_simbol.h"

typedef struct neizracunjvi_elem {
  NeizracunjiviSimbol* simbol;
  struct neizracunjvi_elem* sledeci;
} NeizracunjiviElem;

typedef struct st_tns{
  NeizracunjiviElem* prvi;
  int broj_simbola;
} TNS;

TNS* init_TNS();

void dodaj_neizracunjivi_simbol(TNS*, NeizracunjiviSimbol*);

void razresi_TNS(TNS*);

#endif