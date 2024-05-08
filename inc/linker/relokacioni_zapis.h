#ifndef RELOKACIONI_ZAPIS_H
#define RELOKACIONI_ZAPIS_H

struct simbol;

typedef struct relokacioni_zapis {
  int offset;
  struct simbol* simbol;
  int addend;
  struct relokacioni_zapis* sledeci;
} RelokacioniZapis;

RelokacioniZapis* init_relokacioni_zapis(int offset, struct simbol* simbol, int addend);

#endif

