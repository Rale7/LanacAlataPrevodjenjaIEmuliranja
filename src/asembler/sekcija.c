
#include <stdlib.h>
#include "../../inc/asembler/sekcija.h"
#include "../../inc/asembler/relokacioni_zapis.h"
#include "../../inc/asembler/bazen_literala.h"

Sekcija* init_sekcija(const char* ime, Simbol* simbol) {

  Sekcija* nova = (Sekcija*) malloc(sizeof(Sekcija));

  if (nova == NULL) {
    exit(1);
  }

  nova->location_counter = 0;
  nova->sadrzaj = init_sadrzaj_sekcije();
  nova->trz = init_TRZ();
  nova->bazen_literala = init_bazen();
  nova->prethodni_bl = NULL;
  
  if (simbol == NULL) {
      nova->simbol = init_simbol(ime, 0, nova);
  } else {
    nova->simbol = simbol;
  }

  return nova;
}