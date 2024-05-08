#include <stdlib.h>
#include <string.h>
#include "../../inc/asembler/moja_string_sekcija.h"

MojaStringSekcija *init_moja_string_sekcija() {
  MojaStringSekcija* nova = (MojaStringSekcija*) malloc(sizeof(MojaStringSekcija));
  if (nova == NULL) {
    exit(1);
  }

  nova->kapacitet = 50;
  nova->velicina = 1;
  nova->slova = (char*) calloc(50, sizeof(char));
  if (nova->slova == NULL) {
    exit(1);
  }

  return nova;
}

int dodaj_string(MojaStringSekcija* mss, const char* novi) {

  int n = strlen(novi) + 1;

  while (mss->kapacitet < mss->velicina + n) {
    mss->kapacitet = (mss->kapacitet * 3) / 2;
    mss->slova = (char*) realloc(mss->slova, mss->kapacitet);
    if (mss->slova == NULL) {
      exit(1);
    }
  }

  for (int i = 0; i < n; i++) {
    mss->slova[i + mss->velicina] = novi[i];
  }

  int ret = mss->velicina;
  mss->velicina += n;
  return ret;
}

int dodaj_string_povezano(MojaStringSekcija* mss, const char* prvi, const char* drugi){

  int n1 = strlen(prvi) + 1;
  int n2 = strlen(drugi) + 1;

  while (mss->kapacitet < mss->velicina + n1 + n2) {
    mss->kapacitet = (mss->kapacitet * 3) / 2;
    mss->slova = (char*) realloc(mss->slova, mss->kapacitet);
    if (mss->slova == NULL) {
      exit(1);
    }
  }

  int i;
  for (i = 0; i < n1 - 1; i++) {
    mss->slova[i + mss->velicina] = prvi[i];
  }
  mss->slova[i + mss->velicina] = '.';

  int ret = mss->velicina;
  mss->velicina += n1;

  for (i = 0; i < n2; i++) {
    mss->slova[i + mss->velicina] = drugi[i];
  }
  mss->velicina += n2;

  return ret;

}

void obrisi_moju_string_sekciju(MojaStringSekcija* mss) {

  free(mss->slova);
  free(mss);
}