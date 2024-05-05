#include <stdlib.h>
#include <stdio.h>
#include "../../inc/asembler/simbol.h"
#include "../../inc/asembler/sekcija.h"
#include "../../inc/asembler/instrukcije.h"
#include "../../inc/asembler/relokacioni_zapis.h"
#include "../../inc/asembler/izraz.h"

static void nedefinisan_skok(Simbol* simbol, int kod_operacije, enum Registar rb, enum Registar rc, Sekcija* sekcija) {

  char oc = (char) kod_operacije;
  oc = transliraj_instrukciju_pomeraj(oc);
  instrukcija_sa_simbol_bazen(oc, R15, rb, rc, simbol);
}

static void nedefinisan_load_imm(Simbol* simbol, enum Registar r1, Sekcija* sekcija) {

  instrukcija_sa_simbol_bazen(0x92, r1, R15, R0, simbol);
}

static void nedefinisan_load_mem(Simbol* simbol, enum Registar r1, Sekcija* sekcija) {

  instrukcija_sa_simbol_bazen(0x92, r1, R15, R0, simbol);
  instrukcija_sa_pomerajem(0x92, r1, r1, R0, 0);
}

static void nedefinisan_load_reg(Simbol* simbol, enum Registar r1, enum Registar r2, Sekcija* sekcija) {

  printf("Greska simbol %s je relokativan", simbol->naziv);
}

static void nedefinisan_st_reg(Simbol* simbol, enum Registar r1, enum Registar r2, Sekcija* sekcija) {

  printf("Greska simbol %s je relokativan", simbol->naziv); 
}

static void nedefinisan_st_mem(Simbol* simbol, enum Registar r1, Sekcija* sekcija) {

  instrukcija_sa_simbol_bazen(0x82, R15, R0, r1, simbol);
}

static RelokacioniZapis* nedefinisan_nrz(Simbol* simbol, Sekcija* sekcija, int lokacija, int obracanje) {

  int pomeraj = lokacija - (obracanje + 4);
  ugradi_pomeraj_simbol(sekcija, obracanje, pomeraj);

  if (sekcija->trz->poslednji && sekcija->trz->poslednji->offset == lokacija) {
    return sekcija->trz->poslednji;
  } else {
    RelokacioniZapis* rz = init_RZ(lokacija, simbol);
    dodaj_relokacioni_zapis(sekcija->trz, rz);
    return rz;
  }
}

static void nedefinisan_sdw(Simbol* simbol, Sekcija* sekcija, int lokacija) {

  ObracanjeUnapred* novo = (ObracanjeUnapred*) malloc(sizeof(ObracanjeUnapred));
  if (novo == NULL) {
    exit(1);
  }
  
  novo->lokacija = lokacija;
  novo->sekcija = sekcija;
  novo->sledeci = simbol->oulista;
  simbol->oulista = novo;
}

static enum Relokatibilnost nedefinisana_relokatibilnost(Simbol* simbol, Sekcija** sekcija) {
  
  return NEIZRACUNJIV;
}

static Simbol_TVF nedefinisan_simbol_tvf = { 
  .skok = &nedefinisan_skok,
  .imm = &nedefinisan_load_imm,
  .ld_mem = &nedefinisan_load_mem,
  .ld_reg = &nedefinisan_load_reg,
  .st_mem = &nedefinisan_st_mem,
  .st_reg = &nedefinisan_st_reg,
  .nrz = &nedefinisan_nrz,
  .sdw = &nedefinisan_sdw,
  .dohvati_relokatibilnost = &nedefinisana_relokatibilnost,
  .neizracunjivi_indeks = &definisan_neizracunjivi_indeks
};

Simbol* init_nedefinisan_simbol(const char* naziv) {

  Simbol* novi = init_simbol(naziv, 0, NULL);
  novi->tvf = &nedefinisan_simbol_tvf;

  return novi;
}