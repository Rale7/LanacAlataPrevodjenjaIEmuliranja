#ifndef MEMORY_H
#define MEMORY_H

struct segment;

typedef int (*DohvatiVrednost)(struct segment*, unsigned int address);
typedef void (*PostaviVrednost)(struct segment*, unsigned int address, int value);

typedef struct segment_tvf {
  DohvatiVrednost dohvati_vrednost;
  PostaviVrednost postavi_vrednost;
} SegmentTVF; 

typedef struct segment {
  unsigned int pocetna_adresa;
  unsigned int krajnja_adresa;
  int visina;
  int* sadrzaj;
  struct segment* levi;
  struct segment* desni;
  SegmentTVF tvf;
} Segment;


typedef struct memorija {
  Segment* koren;
} Memorija;

Segment* init_segment(int pocetna_adresa, int krajnja_adresa);

void ubaci_segment(Memorija*, Segment*);

int dohvati_vrednost(Memorija*, unsigned int adresa);

void postavi_vrednost(Memorija*, unsigned int adresa, int vrednost);

#endif