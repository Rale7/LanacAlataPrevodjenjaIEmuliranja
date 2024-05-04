#ifndef BAZEN_LITERALA_H
#define BAZEN_LITERALA_H

struct simbol;

typedef struct obracanje {

  int lokacija;
  struct obracanje* sledeci;

} Obracanje;

typedef struct ulaz_literal
{
  int vrednost;
  Obracanje* prvi;
  Obracanje** indirect;
  struct ulaz_literal* sledeci;
  int lokacija;

} UlazLiteral;

typedef struct ulaz_simbol{

  struct simbol* simbol;
  Obracanje* prvi;
  Obracanje** indirect;
  struct ulaz_simbol* sledeci;
  int lokacija;

} UlazSimbol;

typedef struct bazen_literala {
  UlazSimbol* prviSimbol;
  UlazSimbol** indirectSimbol;
  UlazLiteral* prviLiteral;
  UlazLiteral** indirectLiteral;

} BazenLiterala;

BazenLiterala* init_bazen();


/*
  vraca -1 u slucaju da je bazen prazan,
  a ukoliko postoji nesto u bazenu vraca 0
  i na adresu specificiranu drugim parametrom
  upisuje najmanju lokaciju obracanja
*/
int dohvati_najmanju_lokaciju(BazenLiterala*, int*);

void dodaj_obracanje_literal(BazenLiterala*, int literal, int lokacija);

int provera_postoji_literal(BazenLiterala*, int literal, int*);

void dodaj_obracanje_simbol(BazenLiterala*, struct simbol*, int lokacija);

int provera_postoji_simbol(BazenLiterala*, struct simbol*, int*);

void upisi_bazen(BazenLiterala*, int);

void obrisi_bazen(BazenLiterala*);

#endif