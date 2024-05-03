#ifndef RELOKACIONI_ZAPIS_H
#define RELOKACIONI_ZAPIS_H

struct simbol;

#include "simbol.h"

typedef struct rz {

  int offset;
  struct simbol* simbol;
  struct rz* sledeci;

} RelokacioniZapis;

typedef struct tabela_rz{

  RelokacioniZapis* prvi;
  RelokacioniZapis* poslednji;

} TabelaRZ;

RelokacioniZapis* init_RZ(int offset, struct simbol*);

TabelaRZ* init_TRZ();

void dodaj_relokacioni_zapis(TabelaRZ*, RelokacioniZapis*);

#endif