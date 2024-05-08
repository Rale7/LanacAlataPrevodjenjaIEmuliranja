#include <stdlib.h>
#include "../../inc/linker/relokacioni_zapis.h"
#include "../../inc/linker/simbol.h"

RelokacioniZapis* init_relokacioni_zapis(int offset, Simbol* simbol, int addend) {

  RelokacioniZapis* novi = (RelokacioniZapis*) malloc(sizeof(RelokacioniZapis));
  if (novi == NULL) {
    exit(1);
  }

  novi->addend = addend;
  novi->offset = offset;
  novi->simbol = simbol;
  novi->sledeci = NULL;

  return novi;
}