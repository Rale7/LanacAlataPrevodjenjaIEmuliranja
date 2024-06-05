#ifndef MEMORY_H
#define MEMORY_H

struct segment;

typedef int (*DohvatiVrednost)(struct segment*, unsigned int address);
typedef void (*PostaviVrednost)(struct segment*, unsigned int address, int value);
typedef void (*ObrisiSegment)(struct segment*);

typedef struct segment_tvf {
  DohvatiVrednost dohvati_vrednost;
  PostaviVrednost postavi_vrednost;
  ObrisiSegment obrisi_segment;
} SegmentTVF; 

typedef struct segment {
  unsigned int pocetna_adresa;
  unsigned int krajnja_adresa;
  int visina;
  char* sadrzaj;
  struct segment* levi;
  struct segment* desni;
  SegmentTVF *tvf;
} Segment;


typedef struct memorija {
  Segment* koren;
} Memorija;

Segment* init_segment(unsigned int pocetna_adresa, unsigned int krajnja_adresa);

Segment* init_segment_sadrzaj(unsigned int, unsigned int, char*);

Memorija* init_memorija();

void ubaci_segment(Memorija*, Segment*);

int dohvati_vrednost(Memorija*, unsigned int adresa);

void postavi_vrednost(Memorija*, unsigned int adresa, int vrednost);

void inorder_memorija(Segment* segment);

void obrisi_memoriju(Memorija* memorija);

#endif