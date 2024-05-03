#ifndef SADRZAJ_SEKCIJE_H
#define SADRZAJ_SEKCIJE_H

typedef struct
{
  char* byte;
  int kapacitet;

} SadrzajSekcije;

SadrzajSekcije* init_sadrzaj_sekcije();

char* dohvati_sadrzaj(SadrzajSekcije*, int);

void popuni_nulama(SadrzajSekcije* ss, int lokacija, int velicina);

void postavi_sadrzaj(SadrzajSekcije*, int lokacija, const char*, int );

#endif