#ifndef RACUNAR_H
#define RACUNAR_H

#include "procesor.h"
#include "memorija.h"

typedef struct racunar {
  Procesor* procesor;
  Memorija* memorija;
} Racunar;

Racunar* init_racunar(const char*);

void* rad_racunara(void*);

void obrisi_racunar(Racunar*);


#endif