#include <stdlib.h>
#include <stdio.h>
#include "../../inc/asembler/neizracunjivi_simbol.h"
#include "../../inc/asembler/izraz.h"
#include "../../inc/asembler/sekcija.h"
#include "../../inc/asembler/instrukcije.h"
#include "../../inc/asembler/relokacioni_zapis.h"
#include "../../inc/asembler/tabela_neizracunjivih_simbola.h"
#include "../../inc/asembler/asembler.h"

static void neizracunjivi_skok (Simbol* simbol, int kod_operacije, enum Registar r1, enum Registar r2,Sekcija* sekcija) {
  NeizracunjiviSimbol* neizracunjivi_simbol = simbol->neizracunjivi;

  ObracanjeInstrukcije* novo_obracanje = init_obracanje_instrukcija(sekcija, sekcija->location_counter);
  novo_obracanje->sledeci = simbol->oilista;
  simbol->oilista = novo_obracanje;

  char oc = (char) kod_operacije;
  oc = transliraj_instrukciju_pomeraj(oc);
  instrukcija_sa_simbol_bazen(oc, R15, r1, r2, simbol);
}

static void neizracunjivi_ldimm (Simbol* simbol, enum Registar r1, Sekcija* sekcija) {
  NeizracunjiviSimbol* neizracunjivi_simbol = simbol->neizracunjivi;;

  ObracanjeInstrukcije* novo_obracanje = init_obracanje_instrukcija(sekcija, sekcija->location_counter);
  novo_obracanje->sledeci = simbol->oilista;
  simbol->oilista = novo_obracanje;

  instrukcija_sa_simbol_bazen(0x92, r1, R15, R0, simbol);
}

static void neizracunjivi_ldmem (Simbol* simbol, enum Registar r1, Sekcija* sekcija) {
  NeizracunjiviSimbol* neizracunjivi_simbol = simbol->neizracunjivi;;

  ObracanjeInstrukcije* novo_obracanje = init_obracanje_instrukcija(sekcija, sekcija->location_counter);
  novo_obracanje->sledeci = simbol->oilista;
  simbol->oilista = novo_obracanje;

  instrukcija_sa_simbol_bazen(0x92, r1, R15, R0, simbol);
  instrukcija_sa_pomerajem(0x92, r1, r1, R0, 0);
}

static void neizracunjivi_ldreg (Simbol* simbol, enum Registar r1, enum Registar r2, Sekcija* sekcija) {
  NeizracunjiviSimbol* neizracunjivi_simbol = simbol->neizracunjivi;;

  ObracanjeInstrukcije* novo_obracanje = init_obracanje_instrukcija(sekcija, sekcija->location_counter);
  novo_obracanje->sledeci = simbol->oilista;
  simbol->oilista = novo_obracanje;
}

static void neizracunjvi_streg(Simbol* simbol, enum Registar r1, enum Registar r2, Sekcija* sekcija) {

  printf("Greska simbol %s je relokativan", simbol->naziv); 
}

static void neizracunjiv_stmem(Simbol* simbol, enum Registar r1, Sekcija* sekcija) {
  NeizracunjiviSimbol* neizracunjivi_simbol = simbol->neizracunjivi;

  ObracanjeInstrukcije* novo_obracanje = init_obracanje_instrukcija(sekcija, sekcija->location_counter);
  novo_obracanje->sledeci = simbol->oilista;
  simbol->oilista = novo_obracanje;

  instrukcija_sa_simbol_bazen(0x82, R15, R0, r1, simbol);
}

static RelokacioniZapis* neizracunjiv_nrz(Simbol* simbol, Sekcija* sekcija, int lokacija, int obracanje) {

  ObracanjeInstrukcije* novo_obracanje = init_obracanje_instrukcija(sekcija, sekcija->location_counter);
  novo_obracanje->sledeci = simbol->oilista;
  simbol->oilista = novo_obracanje;

  ObracanjeUnapred* novo = (ObracanjeUnapred*) malloc(sizeof(ObracanjeUnapred));
  if (novo == NULL) {
    exit(1);
  }

  int pomeraj = lokacija - (obracanje + 4);
  ugradi_pomeraj_simbol(sekcija, obracanje, pomeraj);
  
  novo->lokacija = lokacija;
  novo->sekcija = sekcija;
  novo->sledeci = simbol->oulista;
  simbol->oulista = novo;

  if (sekcija->trz->poslednji && sekcija->trz->poslednji->offset == lokacija) {
    return sekcija->trz->poslednji;
  } else {
    RelokacioniZapis* rz = init_RZ(lokacija, simbol);
    dodaj_relokacioni_zapis(sekcija->trz, rz);
    return rz;
  }
}

static void neizracunjiv_sdw(Simbol* simbol, Sekcija* sekcija, int lokacija) {

  ObracanjeUnapred* novo = (ObracanjeUnapred*) malloc(sizeof(ObracanjeUnapred));
  if (novo == NULL) {
    exit(1);
  }
  
  novo->lokacija = lokacija;
  novo->sekcija = sekcija;
  novo->sledeci = simbol->oulista;
  simbol->oulista = novo;
}

static enum Relokatibilnost neizracunjiva_relokatibilnost(Simbol* simbol, Sekcija** sekcija) {
  
  return NEIZRACUNJIV;
}

static int neizracunjivi_simbol_indeks(Simbol* simbol) {
  NeizracunjiviSimbol* ns = simbol->neizracunjivi;

  return ns->neizracunjivi_id;
}

static Simbol_TVF neizracunjiv_tvf = {
  .skok = &neizracunjivi_skok,
  .imm = &neizracunjivi_ldimm,
  .ld_mem = &neizracunjivi_ldmem,
  .ld_reg = &neizracunjivi_ldreg,
  .st_mem = &neizracunjiv_stmem,
  .st_reg = &neizracunjvi_streg,
  .nrz = &neizracunjiv_nrz,
  .sdw = &neizracunjiv_sdw,
  .dohvati_relokatibilnost = &neizracunjiva_relokatibilnost,
  .neizracunjivi_indeks = &neizracunjivi_simbol_indeks
};

static int inicijalizator = 0;

NeizracunjiviSimbol* init_neizracunjivi_simbol(const char* naziv_simbola, Sekcija* sekcija, Izraz* izraz) {

  Simbol* simbol = init_simbol(naziv_simbola, 0, sekcija);
  NeizracunjiviSimbol* novi = (NeizracunjiviSimbol*) malloc(sizeof(NeizracunjiviSimbol));
  if (novi == NULL) {
    exit(1);
  }
  
  novi->simbol = simbol;
  novi->simbol->neizracunjivi = novi;
  novi->simbol->tvf = &neizracunjiv_tvf;

  novi->neizracunjivi_id = inicijalizator++;
  novi->izraz = izraz;

  return novi;
}

Simbol* prebaci_u_neizracunjiv(Simbol* simbol, Izraz* izraz, Sekcija* sekcija) {

  NeizracunjiviSimbol* neizracunjivi_simbol = (NeizracunjiviSimbol*) malloc(sizeof(NeizracunjiviSimbol));
  if (neizracunjivi_simbol == NULL) {
    exit(1);
  }

  neizracunjivi_simbol->simbol = simbol;
  simbol->neizracunjivi = neizracunjivi_simbol;
  neizracunjivi_simbol->simbol->sekcija = sekcija;
  
  dodaj_neizracunjivi_simbol(dohvati_asembler()->tabela_neizrazunljivih_simbola, neizracunjivi_simbol);

  neizracunjivi_simbol->izraz = izraz;
  neizracunjivi_simbol->neizracunjivi_id = inicijalizator++;
  neizracunjivi_simbol->simbol->tvf = &neizracunjiv_tvf;

  return (Simbol*)neizracunjivi_simbol;
}