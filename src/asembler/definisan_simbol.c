#include <stdlib.h>
#include <stdio.h>
#include "../../inc/asembler/simbol.h"
#include "../../inc/asembler/sekcija.h"
#include "../../inc/asembler/instrukcije.h"
#include "../../inc/asembler/relokacioni_zapis.h"

static int provera_ugradjivanje(Simbol* simbol, char oc, enum Registar r1, Sekcija* sekcija) {

  if (sekcija->simbol->redosled == simbol->sekcija->simbol->redosled) {

    int pomeraj = simbol->vrednost - sekcija->location_counter;

    if (pomeraj >= -1024) {

      instrukcija_sa_pomerajem(oc, r1, R15, R0, pomeraj);
      return 1;
    }
  }

  return 0;
}

static void definisan_skok(Simbol* simbol, int kod_operacije, enum Registar rb, enum Registar rc, Sekcija* sekcija) {

  if (sekcija->simbol->redosled == simbol->sekcija->simbol->redosled) {

    int pomeraj = simbol->vrednost - sekcija->location_counter;

    if (pomeraj >= -1024) {
      char oc = (char) kod_operacije & 0xFF;
      instrukcija_sa_pomerajem(oc, R15, rb, rc, pomeraj);
      return;
    }
  }

  char oc = (char) kod_operacije & 0xFF;
  oc |= (char)(kod_operacije == 0x20 ? 1 : 8);

  
  instrukcija_sa_simbol_bazen(oc, R15, rb, rc, simbol);
}

static void definisan_load_imm(Simbol* simbol, enum Registar r1, Sekcija* sekcija) {

  if (!provera_ugradjivanje(simbol, 0x91, r1, sekcija)) {

    instrukcija_sa_simbol_bazen(0x92, r1, R15, R0, simbol);
  }

}

static void definisan_load_mem(Simbol* simbol, enum Registar r1, Sekcija* sekcija) {
  
  if (!provera_ugradjivanje(simbol, 0x92, r1, sekcija)) {

    definisan_load_imm(simbol, r1, sekcija);
    instrukcija_sa_pomerajem(0x92, r1, r1, R0, 0);
  }
}

static void definisan_load_reg(Simbol* simbol, enum Registar r1, enum Registar r2, Sekcija* sekcija) {

  printf("Greska simbol %s je relokativan", simbol->naziv);
}

static void definisan_st_reg(Simbol* simbol, enum Registar r1, enum Registar r2, Sekcija* sekcija) {

  printf("Greska simbol %s je relokativan", simbol->naziv);
}

static void definisan_st_mem(Simbol* simbol, enum Registar r1, Sekcija* sekcija) {

  if (simbol->sekcija->simbol->redosled == sekcija->simbol->redosled) {

    int pomeraj = simbol->vrednost - sekcija->location_counter;

    if (pomeraj >= -1024) {
      instrukcija_sa_pomerajem(0x80, R15, R0, r1, pomeraj);
      return;
    }
  }

  instrukcija_sa_simbol_bazen(0x82, R15, R0, r1, simbol);
}

static char* tabela_instrukcija_init() {
  static char tabela_instrukcija[256];
  static int first = 0;

  if (first) return tabela_instrukcija;

  tabela_instrukcija[0x21] = 0x20;
  tabela_instrukcija[0x38] = 0x30;
  tabela_instrukcija[0x39] = 0x31;
  tabela_instrukcija[0x3A] = 0x32;
  tabela_instrukcija[0x3B] = 0x33;
  tabela_instrukcija[0x82] = 0x80;
  tabela_instrukcija[0x92] = 0x91;

  first = 1;

  return tabela_instrukcija;

}

static int vrati_transliranu_instrukciju(char oc) {
  char* tabela_instrukcija = tabela_instrukcija_init();

  return tabela_instrukcija[oc];
}

static RelokacioniZapis* definisan_nrz(Simbol* simbol, Sekcija* sekcija, int lokacija, int obracanje) {

  if (simbol->sekcija->simbol->redosled == sekcija->simbol->redosled) {

    int pomeraj = simbol->vrednost - obracanje;

    if (pomeraj >= -1024 && pomeraj < 1023) {

        char oc = *dohvati_sadrzaj(sekcija->sadrzaj, obracanje);
        oc = vrati_transliranu_instrukciju(oc);
        postavi_sadrzaj(sekcija->sadrzaj, obracanje, &oc, 1);

        char reg_pom = *dohvati_sadrzaj(sekcija->sadrzaj, obracanje + 2);
        reg_pom = (char)((reg_pom & 0xF0) | ((pomeraj & 0xF00) >> 8));
        postavi_sadrzaj(sekcija->sadrzaj, obracanje + 2, &reg_pom, 1);
        
        char pom = (char) (pomeraj & 0xFF);
        postavi_sadrzaj(sekcija->sadrzaj, obracanje + 3, &pom, 1);
        return NULL;
    }
  }
  
  if (sekcija->trz->poslednji->offset == lokacija) {
    return sekcija->trz->poslednji;
  } else {
    RelokacioniZapis* rz = init_RZ(lokacija, simbol);
    dodaj_relokacioni_zapis(sekcija->trz, rz);
    return rz;
  }
}

static void definisan_sdw(Simbol* simbol, Sekcija* sekcija, int lokacija) {

  postavi_sadrzaj(sekcija->sadrzaj, lokacija, (const char*) &simbol->vrednost, 4);

  if (simbol->sekcija->simbol->redosled != sekcija->simbol->redosled) {
    RelokacioniZapis* rz = init_RZ(lokacija, simbol);
    dodaj_relokacioni_zapis(sekcija->trz, rz);
  }
}

static Simbol_TVF definisan_simbol_tvf = { 
  &definisan_skok,
  &definisan_load_imm,
  &definisan_load_mem,
  &definisan_load_reg,
  &definisan_st_mem,
  &definisan_st_reg,
  &definisan_nrz,
  &definisan_sdw,
};

Simbol *init_definisan_simbol(const char* naziv, int vrednost, Sekcija *sekcija) {
  
  Simbol* novi = init_simbol(naziv, vrednost, sekcija);
  novi->tvf = &definisan_simbol_tvf;

  return novi;
}

void prebaci_u_definisan(Simbol* simbol, Sekcija* sekcija) {

  if (simbol->tvf == &definisan_simbol_tvf) {
    printf("Visestruka definicija simbola %s\n", simbol->naziv);
    exit(1);
  }

  while (simbol->oulista) {
    ObracanjeUnapred* stari = simbol->oulista;
    simbol->oulista = simbol->oulista->sledeci;

    definisan_sdw(simbol, stari->sekcija, stari->lokacija);
    free(stari);
  }
  
  simbol->sekcija = sekcija;
  simbol->tvf = &definisan_simbol_tvf;
}