#include <stdlib.h>
#include <stdio.h>
#include "../../inc/asembler/simbol.h"
#include "../../inc/asembler/sekcija.h"
#include "../../inc/asembler/instrukcije.h"
#include "../../inc/asembler/relokacioni_zapis.h"
#include "../../inc/asembler/izraz.h"

static int provera_ugradjivanje(Simbol* simbol, char oc, enum Registar r1, Sekcija* sekcija) {

  if (sekcija->simbol->redosled == simbol->sekcija->simbol->redosled) {

    int pomeraj = simbol->vrednost - (sekcija->location_counter + 4);

    if (pomeraj >= -2048) {

      instrukcija_sa_pomerajem(oc, r1, R15, R0, pomeraj);
      return 1;
    }
  }

  return 0;
}

static void definisan_skok(Simbol* simbol, int kod_operacije, enum Registar rb, enum Registar rc, Sekcija* sekcija) {

  if (sekcija->simbol->redosled == simbol->sekcija->simbol->redosled) {

    int pomeraj = simbol->vrednost - (sekcija->location_counter + 4);

    if (pomeraj >= -2048) {
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

    int pomeraj = simbol->vrednost - (sekcija->location_counter + 4);

    if (pomeraj >= -2048) {
      instrukcija_sa_pomerajem(0x80, R15, R0, r1, pomeraj);
      return;
    }
  }

  instrukcija_sa_simbol_bazen(0x82, R15, R0, r1, simbol);
}

static RelokacioniZapis* definisan_nrz(Simbol* simbol, Sekcija* sekcija, int lokacija, int obracanje) {

  int pomeraj = simbol->vrednost - (obracanje + 4);

  if (simbol->sekcija->simbol->redosled == sekcija->simbol->redosled) {

    if (pomeraj >= -2048 && pomeraj < 2047) {

        char oc = *dohvati_sadrzaj(sekcija->sadrzaj, obracanje);
        oc = transliraj_instrukciju_direktno(oc);
        postavi_sadrzaj(sekcija->sadrzaj, obracanje, &oc, 1);

        ugradi_pomeraj_simbol(sekcija, obracanje, pomeraj);
        return NULL;
    }
  }

  pomeraj = lokacija - (obracanje + 4);
  ugradi_pomeraj_simbol(sekcija, obracanje, pomeraj);
  
  if (sekcija->trz->poslednji && sekcija->trz->poslednji->offset == lokacija) {
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

static enum Relokatibilnost definisana_relokatibilnost(Simbol* simbol, Sekcija** sekcija) {

  *sekcija = simbol->sekcija;
  return RELOKATIVAN;
}

static Simbol_TVF definisan_simbol_tvf = { 
  .skok = &definisan_skok,
  .imm = &definisan_load_imm,
  .ld_mem = &definisan_load_mem,
  .ld_reg = &definisan_load_reg,
  .st_mem = &definisan_st_mem,
  .st_reg = &definisan_st_reg,
  .nrz = &definisan_nrz,
  .sdw = &definisan_sdw,
  .dohvati_relokatibilnost = &definisana_relokatibilnost,
  .neizracunjivi_indeks = &definisan_neizracunjivi_indeks
};

Simbol *init_definisan_simbol(const char* naziv, int vrednost, Sekcija *sekcija) {
  
  Simbol* novi = init_simbol(naziv, vrednost, sekcija);
  novi->tvf = &definisan_simbol_tvf;

  return novi;
}

void prebaci_u_definisan(Simbol* simbol, Sekcija* sekcija, int vrednost) {

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
  
  simbol->vrednost = vrednost;
  simbol->sekcija = sekcija;
  simbol->tvf = &definisan_simbol_tvf;
}