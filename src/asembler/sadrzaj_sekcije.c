#include <stdlib.h>
#include <string.h>
#include "../../inc/asembler/sadrzaj_sekcije.h"

#define POCETNI_KAPACITET 50

static inline SadrzajSekcije* safe_realloc(SadrzajSekcije* ss) {

  ss->kapacitet = (ss->kapacitet * 3) / 2;
  ss->byte = (char*) realloc(ss->byte, ss->kapacitet * sizeof(char));

  if (ss == NULL) {
    exit(1);
  }

  return ss;
}

SadrzajSekcije* init_sadrzaj_sekcije() {

  SadrzajSekcije* novi = (SadrzajSekcije*) malloc(sizeof(SadrzajSekcije));

  if (!novi) {
    return NULL;
  }

  novi->byte = (char*) malloc(POCETNI_KAPACITET * sizeof(char));
  novi->kapacitet = POCETNI_KAPACITET;

  return novi;

}

char* dohvati_sadrzaj(SadrzajSekcije* ss, int lokacija) {

  if (!ss || lokacija < 0 || lokacija >= ss->kapacitet) {
    return NULL;
  }

  return &(ss->byte[lokacija]);

}

void popuni_nulama(SadrzajSekcije* ss, int lokacija, int velicina) {

  if (!ss) {
    return;
  }

  while (lokacija + velicina > ss->kapacitet) {
    ss = safe_realloc(ss);
  }

  for (int i = lokacija; i < lokacija + velicina; i++) {
    ss->byte[i] = '\0';
  }

}

void postavi_sadrzaj(SadrzajSekcije* ss, int lokacija, const char* sadrzaj, int velicina) {

  if (!ss) {
    return;
  }

  while (lokacija + velicina > ss->kapacitet) {
    ss = safe_realloc(ss);
  }

  memcpy(ss->byte + lokacija, sadrzaj, velicina);

}