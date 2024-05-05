#include <stdlib.h>
#include <stdio.h>
#include "../../inc/asembler/instrukcije.h"
#include "../../inc/asembler/sekcija.h"
#include "../../inc/asembler/asembler.h"
#include "../../inc/asembler/bazen_literala.h"

void provera_prekoracenja_bazena_inst() {

  Sekcija* trenutna = dohvati_asembler()->trenutna_sekcija;

  int najmanje_obracanje;
  if (dohvati_najmanju_lokaciju(trenutna->bazen_literala, &najmanje_obracanje) == 0 &&
  trenutna->location_counter - najmanje_obracanje >= 2036) {

    if (trenutna->prethodni_bl != NULL) {
      obrisi_bazen(trenutna->prethodni_bl);
    }

    trenutna->prethodni_bl = trenutna->bazen_literala;
    upisi_bazen(trenutna->bazen_literala, 0);
    trenutna->bazen_literala = init_bazen();
  }

}

void bezadresna_inst(int kod_operacije) {

  Sekcija* trenutna = dohvati_asembler()->trenutna_sekcija;

  char vrednost = 0;
  vrednost = (char) (kod_operacije & 0xFF);

  postavi_sadrzaj(trenutna->sadrzaj, trenutna->location_counter, (const char*)&vrednost, 1);
  trenutna->location_counter += 1;
  popuni_nulama(trenutna->sadrzaj, trenutna->location_counter, 3);
  trenutna->location_counter += 3;


}

void instrukcija_sa_pomerajem(char kod_operacije, enum Registar ra, enum Registar rb, enum Registar rc, int pomeraj) {

  Sekcija* trenutna_sekcija = dohvati_asembler()->trenutna_sekcija;

  postavi_sadrzaj(trenutna_sekcija->sadrzaj, trenutna_sekcija->location_counter, &kod_operacije, 1);
  trenutna_sekcija->location_counter += 1;

  char registri = (char)(((ra & 0xF) << 4) | (rb & 0xF));
  postavi_sadrzaj(trenutna_sekcija->sadrzaj, trenutna_sekcija->location_counter, &registri, 1);
  trenutna_sekcija->location_counter += 1;

  char reg_pom = (char)(((rc & 0xF) << 4) | ((pomeraj & 0xF00) >> 8));
  postavi_sadrzaj(trenutna_sekcija->sadrzaj, trenutna_sekcija->location_counter, &reg_pom, 1);
  trenutna_sekcija->location_counter += 1;

  char pom = (char)(pomeraj & 0xFF);
  postavi_sadrzaj(trenutna_sekcija->sadrzaj, trenutna_sekcija->location_counter, &pom, 1);
  trenutna_sekcija->location_counter += 1;
}

void instrukcija_sa_literalom_bazen(char oc, enum Registar ra, enum Registar rb, enum Registar rc, int literal) {

  Sekcija* trenutna_sekcija = dohvati_asembler()->trenutna_sekcija;
  int lokacija_literala;

  if (trenutna_sekcija->prethodni_bl &&
    provera_postoji_literal(trenutna_sekcija->prethodni_bl, literal, &lokacija_literala)) {

    int pomeraj = lokacija_literala - (trenutna_sekcija->location_counter + 4);

    if (pomeraj > - 2048) {
      instrukcija_sa_pomerajem(oc, ra, rb, rc, pomeraj);
      return;
    }
  }

  dodaj_obracanje_literal(trenutna_sekcija->bazen_literala, literal, trenutna_sekcija->location_counter);
  instrukcija_sa_pomerajem(oc, ra, rb, rc, 0);
  
}

void instrukcija_sa_simbol_bazen(char oc, enum Registar ra, enum Registar rb, enum Registar rc, Simbol* simbol) {

  Sekcija* trenutna_sekcija = dohvati_asembler()->trenutna_sekcija;
  int lokacija_literala;

  if (trenutna_sekcija->prethodni_bl &&
    provera_postoji_simbol(trenutna_sekcija->prethodni_bl, simbol, &lokacija_literala)) {

    int pomeraj = lokacija_literala - (trenutna_sekcija->location_counter + 4);

    if (pomeraj > - 2048) {
      instrukcija_sa_pomerajem(oc, ra, rb, rc, pomeraj);
      return;
    }
  }

  dodaj_obracanje_simbol(trenutna_sekcija->bazen_literala, simbol, trenutna_sekcija->location_counter);
  instrukcija_sa_pomerajem(oc, ra, rb, rc, 0);
}


void skok_literal(int kod_operacije, enum Registar rb, enum Registar rc, int literal) {

  if (literal > 2047 || literal <= -2048) {
    char oc = (char) (kod_operacije & 0xFF);
    oc |= (char)(kod_operacije == 0x20 ? 1 : 8);

    instrukcija_sa_literalom_bazen(oc, R15, rb, rc, literal);
  } else {
    char oc = (char) (kod_operacije & 0xFF);
    instrukcija_sa_pomerajem(oc, R0, rb, rc, literal);
  }

}

void skok_simbol(int kod_operacije, enum Registar rb, enum Registar rc, const char* naziv_simbola) {

  TabelaSimbola* ts = dohvati_asembler()->tabel_simbola;
  Simbol* simbol;
  Sekcija* sekcija = dohvati_asembler()->trenutna_sekcija;

  if ((simbol = dohvati_vrednost_simbola(ts, naziv_simbola)) == NULL) {
    simbol = init_nedefinisan_simbol(naziv_simbola);
    dodaj_simbol(ts, simbol);
  }

  simbol->tvf->skok(simbol, kod_operacije, rb, rc, sekcija);
}

void jednoadr_inst(int kod_operacije, enum Registar r1) {

  Sekcija* trenutna = dohvati_asembler()->trenutna_sekcija;
  
  char vrednost = (char) (kod_operacije & 0xFF);
  postavi_sadrzaj(trenutna->sadrzaj, trenutna->location_counter, (const char*)&vrednost, 1);
  trenutna->location_counter += 1;

  vrednost = (char) ((r1 & 0xF) << 4) | (r1 & 0xF);
  postavi_sadrzaj(trenutna->sadrzaj, trenutna->location_counter, (const char*)&vrednost, 1);
  trenutna->location_counter += 1;
  
  popuni_nulama(trenutna->sadrzaj, trenutna->location_counter, 2);
  trenutna->location_counter += 2;
}

void dvoadr_inst(int kod_operacije, enum Registar r1, enum Registar r2) {

  Sekcija* trenutna = dohvati_asembler()->trenutna_sekcija;

  
  char vrednost = (char) (kod_operacije & 0xFF);
  postavi_sadrzaj(trenutna->sadrzaj, trenutna->location_counter, (const char*)&vrednost, 1);
  trenutna->location_counter += 1;

  vrednost = (char) ((r2 & 0xF) << 4) | (r2 & 0xF);
  postavi_sadrzaj(trenutna->sadrzaj, trenutna->location_counter, (const char*)&vrednost, 1);
  trenutna->location_counter += 1;

  vrednost = (char) ((r1 & 0xF) << 4);
  postavi_sadrzaj(trenutna->sadrzaj, trenutna->location_counter, (const char*)&vrednost, 1);
  trenutna->location_counter += 1;

  popuni_nulama(trenutna->sadrzaj, trenutna->location_counter, 1);
  trenutna->location_counter += 1;

}

void ld_imm_simbol(enum Registar r1, const char* naziv_simbola) {

  TabelaSimbola* ts = dohvati_asembler()->tabel_simbola;
  Simbol* simbol;
  Sekcija* sekcija = dohvati_asembler()->trenutna_sekcija;

  if ((simbol = dohvati_vrednost_simbola(ts, naziv_simbola)) == NULL) {
    simbol = init_nedefinisan_simbol(naziv_simbola);
    dodaj_simbol(ts, simbol);
  }

  simbol->tvf->imm(simbol, r1, sekcija);

}

void ld_imm_literal(enum Registar r1, int literal) {

  Sekcija* trenutna_sekcija = dohvati_asembler()->trenutna_sekcija;

  if (literal > 2047 || literal < -2047) {

    char oc = (char) 0x92;
    instrukcija_sa_literalom_bazen(oc, r1, R15, R0, literal);
  } else {
    char oc = (char) 0x91;
    instrukcija_sa_pomerajem(oc, r1, R0, R0, literal);
  }

}

void ld_mem_simbol(enum Registar r1, const char* naziv_simbola) {

  TabelaSimbola* ts = dohvati_asembler()->tabel_simbola;
  Simbol* simbol;
  Sekcija* sekcija = dohvati_asembler()->trenutna_sekcija;

  if ((simbol = dohvati_vrednost_simbola(ts, naziv_simbola)) == NULL) {
    simbol = init_nedefinisan_simbol(naziv_simbola);
    dodaj_simbol(ts, simbol);
  }

  simbol->tvf->ld_mem(simbol, r1, sekcija);

}

void ld_mem_literal(enum Registar r1, int literal) {

  ld_imm_literal(r1, literal);
  instrukcija_sa_pomerajem(0x92, r1, r1, R0, 0);
}

void ld_reg_simbol(enum Registar r1, enum Registar r2, const char* naziv_simbola) {

  TabelaSimbola* ts = dohvati_asembler()->tabel_simbola;
  Simbol* simbol;
  Sekcija* sekcija = dohvati_asembler()->trenutna_sekcija;

  if ((simbol = dohvati_vrednost_simbola(ts, naziv_simbola)) == NULL) {
    simbol = init_nedefinisan_simbol(naziv_simbola);
    dodaj_simbol(ts, simbol);
  }

  simbol->tvf->ld_reg(simbol, r1, r2, sekcija);

}

void ld_reg_literal(enum Registar r1, enum Registar r2, int pomeraj) {

  if (pomeraj > 2047 || pomeraj <= -2047) {
    printf("Pomeraj mora biti manji od 12 bitova");
    exit(1);
  }

  char kod_operacije = 0x92;
  instrukcija_sa_pomerajem(kod_operacije, r1, r2, R0, pomeraj);

}

void st_mem_simbol(enum Registar r1, const char* naziv_simbola) {

  TabelaSimbola* ts = dohvati_asembler()->tabel_simbola;
  Simbol* simbol;
  Sekcija* sekcija = dohvati_asembler()->trenutna_sekcija;

  if ((simbol = dohvati_vrednost_simbola(ts, naziv_simbola)) == NULL) {
    simbol = init_nedefinisan_simbol(naziv_simbola);
    dodaj_simbol(ts, simbol);
  }

  simbol->tvf->st_mem(simbol, r1, sekcija);

}

void st_mem_literal(enum Registar r1, int literal) {

  if (literal > 2047 || literal <= -2048) {
    char oc = 0x82;
    instrukcija_sa_literalom_bazen(oc, R15, R0, r1, literal);
  } else {
    char oc = (char) 0x80;
    instrukcija_sa_pomerajem(oc, R0, R0, r1, literal);
  }
}

void st_reg_simbol(enum Registar r1, enum Registar r2, const char* naziv_simbola) {

  TabelaSimbola* ts = dohvati_asembler()->tabel_simbola;
  Simbol* simbol;
  Sekcija* sekcija = dohvati_asembler()->trenutna_sekcija;

  if ((simbol = dohvati_vrednost_simbola(ts, naziv_simbola)) == NULL) {
    simbol = init_nedefinisan_simbol(naziv_simbola);
    dodaj_simbol(ts, simbol);
  }

  simbol->tvf->st_reg(simbol,r1, r2, sekcija);
}

void st_reg_literal(enum Registar r1, enum Registar r2, int pomeraj) {

  if (pomeraj > 2047 || pomeraj <= -2048) {
    printf("Pomeraj mora biti manji od 12 bitova");
    exit(1);
  }

  char kod_operacije = 0x80;
  instrukcija_sa_pomerajem(kod_operacije, r2, R0, r1, pomeraj);

}

void push_inst(enum Registar r1) {

  char kod_operacije = (char) 0x81;
  instrukcija_sa_pomerajem(kod_operacije, R14, R0, r1, -4);
}

void pop_inst(enum Registar r1) {

  char kod_operacije = (char) 0x93;
  instrukcija_sa_pomerajem(kod_operacije, r1, R14, R0, 4); 
}

void iret_inst() {

  instrukcija_sa_pomerajem(0x96, STATUS % 16, R14, R0, 4);
  instrukcija_sa_pomerajem(0x93, R15, R14, R0, 8);

}

static char* tabela_instrukcija_pomeraj_init() {
  static char tabela_instrukcija[256];
  static int first = 0;

  if (first) return tabela_instrukcija;

  tabela_instrukcija[0x20] = 0x21;
  tabela_instrukcija[0x21] = 0x21;
  tabela_instrukcija[0x30] = 0x38;
  tabela_instrukcija[0x38] = 0x38;
  tabela_instrukcija[0x31] = 0x39;
  tabela_instrukcija[0x39] = 0x39;
  tabela_instrukcija[0x32] = 0x3A;
  tabela_instrukcija[0x3A] = 0x3A;
  tabela_instrukcija[0x33] = 0x3B;
  tabela_instrukcija[0x3B] = 0x3B;
  tabela_instrukcija[0x80] = 0x82;
  tabela_instrukcija[0x82] = 0x82;
  tabela_instrukcija[0x91] = 0x92;
  tabela_instrukcija[0x92] = 0x92;
  first = 1;

  return tabela_instrukcija;
}

static char* tabela_instrukcija_direktno_init() {
  static char tabela_instrukcija[256];
  static int first = 0;
  
  if (first) return tabela_instrukcija;

  tabela_instrukcija[0x20] = 0x20;
  tabela_instrukcija[0x21] = 0x20;
  tabela_instrukcija[0x30] = 0x30;
  tabela_instrukcija[0x38] = 0x30;
  tabela_instrukcija[0x31] = 0x31;
  tabela_instrukcija[0x39] = 0x31;
  tabela_instrukcija[0x32] = 0x32;
  tabela_instrukcija[0x3A] = 0x32;
  tabela_instrukcija[0x33] = 0x33;
  tabela_instrukcija[0x3B] = 0x33;
  tabela_instrukcija[0x80] = 0x80;
  tabela_instrukcija[0x82] = 0x80;
  tabela_instrukcija[0x91] = 0x91;
  tabela_instrukcija[0x92] = 0x91;
  first = 1;

  return tabela_instrukcija;
}

char transliraj_instrukciju_direktno(char oc) {
  char* tabela_instrukcija = tabela_instrukcija_direktno_init();
  unsigned char index = (unsigned char) oc;

  return tabela_instrukcija[index];
}

char transliraj_instrukciju_pomeraj(char oc) {
  char* table_instrukcija = tabela_instrukcija_pomeraj_init();
  unsigned char index = (unsigned char) oc;

  return table_instrukcija[index];
}

void csr_inst(int kod_operacije, enum Registar r1, enum Registar r2) {
  Sekcija* trenutna = dohvati_asembler()->trenutna_sekcija;

  char vrednost = (char) (kod_operacije & 0xFF);
  postavi_sadrzaj(trenutna->sadrzaj, trenutna->location_counter, (const char*)&vrednost, 1);
  trenutna->location_counter += 1;

  vrednost = (char) ((r2 & 0xF) << 4) | (r1 & 0xF);
  postavi_sadrzaj(trenutna->sadrzaj, trenutna->location_counter, (const char*)&vrednost, 1);
  trenutna->location_counter += 1;

  popuni_nulama(trenutna->sadrzaj, trenutna->location_counter, 2);
  trenutna->location_counter += 2;
}