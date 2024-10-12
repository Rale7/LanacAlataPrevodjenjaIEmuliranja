#include <elf.h>
#include <stdio.h>
#include <stdlib.h>

#include "asembler/direktive.h"
#include "asembler/instrukcije.h"
#include "asembler/izraz.h"
#include "asembler/neizracunjivi_simbol.h"
#include "asembler/relokacioni_zapis.h"
#include "asembler/sekcija.h"
#include "asembler/simbol.h"
#include "asembler/stek_izraz.h"

void lokalni_ispis(Simbol* simbol);
char lokalni_tip(Simbol* simbol);

static void simkonst_skok(Simbol* simbol, int kod_operacije, enum Registar r1,
                          enum Registar r2, Sekcija* sekcija) {
  skok_literal(kod_operacije, r1, r2, simbol->vrednost);
}

static void simkonst_ldimm(Simbol* simbol, enum Registar r1, Sekcija* sekcija) {
  ld_imm_literal(r1, simbol->vrednost);
}

static void simkonst_ldmem(Simbol* simbol, enum Registar r1, Sekcija* sekcija) {
  ld_mem_literal(r1, simbol->vrednost);
}

static void simkonst_ldreg(Simbol* simbol, enum Registar r1, enum Registar r2,
                           Sekcija* sekcija) {
  ld_reg_literal(r1, r2, simbol->vrednost);
}

static void simkonst_stmem(Simbol* simbol, enum Registar r1, Sekcija* sekcija) {
  st_mem_literal(r1, simbol->vrednost);
}

static void simkonst_streg(Simbol* simbol, enum Registar r1, enum Registar r2,
                           Sekcija* sekcija) {
  st_reg_literal(r1, r2, simbol->vrednost);
}

static RelokacioniZapis* simkonst_napr_rel_zapis(Simbol* simbol,
                                                 Sekcija* sekcija, int lokacija,
                                                 int obracanje) {
  if (simbol->vrednost >= -2048 || simbol->vrednost < 2047) {
    char oc = *dohvati_sadrzaj(sekcija->sadrzaj, obracanje);
    oc = transliraj_instrukciju_direktno(oc);

    char reg_reg = *dohvati_sadrzaj(sekcija->sadrzaj, lokacija + 1);
    reg_reg = (char)((oc & 0xF0 == 0x90) ? (reg_reg & 0xF0) | (R0)
                                         : (reg_reg & 0xF0) | (R0));
    postavi_sadrzaj(sekcija->sadrzaj, lokacija + 1, &reg_reg, 1);

    postavi_sadrzaj(sekcija->sadrzaj, obracanje, &oc, sizeof(oc));

    ugradi_pomeraj_simbol(sekcija, obracanje, simbol->vrednost);
  } else {
    int pomeraj = lokacija - (obracanje + 4);

    ugradi_pomeraj_simbol(sekcija, lokacija, pomeraj);
  }

  return NULL;
}

static void simkonst_word(Simbol* simbol, Sekcija* sekcija, int lokacija) {
  word_dir_literal(simbol->vrednost);
}

static enum Relokatibilnost simkonst_relokitibilnost(Simbol* simbol,
                                                     Sekcija** sekcija) {
  return APSOLUTAN;
}

static Simbol_TVF simbolicka_konstanta_TVF = {
    .skok = &simkonst_skok,
    .imm = &simkonst_ldimm,
    .ld_mem = &simkonst_ldmem,
    .ld_reg = &simkonst_ldreg,
    .st_mem = &simkonst_stmem,
    .st_reg = &simkonst_streg,
    .nrz = &simkonst_napr_rel_zapis,
    .sdw = &simkonst_word,
    .dohvati_relokatibilnost = &simkonst_relokitibilnost,
};

static void simkonst_ispis_rz(Simbol* simbol,
                              RelokacioniZapis* relokacioni_zapis) {}

static int simkonst_addend_rz(Simbol* simbol) { return simbol->vrednost; }

static char simbolicki_tip(Simbol* simbol) {
  if (simbol->tip == STB_GLOBAL) {
    return STB_GLOBAL;
  } else {
    return STB_LOCAL;
  }
}

static int simbkonst_referisana_sekcija(Simbol* simbol) { return SHN_ABS; }

static int simkonst_simbol_rel(Simbol* simbol) { return -1; }

static Tip_TVF simbolicki_tip_TVF = {
    .ispis_simbola = &lokalni_ispis,
    .ispis_relokacionog_zapisa = &simkonst_ispis_rz,
    .dohvati_dodavanje = &simkonst_addend_rz,
    .dohvati_bind = &simbolicki_tip,
    .dohvati_tip = &dohvati_tip_nedefinisan,
    .dohvati_simbol_rel = &simkonst_simbol_rel,
    .dohvati_referisanu_sekciju = &simbkonst_referisana_sekcija};

Simbol* init_simbolicka_konstanta(const char* simbol, Izraz* izraz,
                                  Sekcija* sekcija) {
  int vrednost = izracunaj_vrednost_izraza(izraz);

  Simbol* novi = init_simbol(simbol, vrednost, sekcija);
  novi->tvf = &simbolicka_konstanta_TVF;
  novi->tip_tvf = &simbolicki_tip_TVF;

  return novi;
}

void prebaci_u_simbolicku_konstantu(Simbol* simbol, int vrednost) {
  simbol->vrednost = vrednost;
  simbol->tvf = &simbolicka_konstanta_TVF;
  simbol->tip_tvf = &simbolicki_tip_TVF;

  while (simbol->oulista) {
    ObracanjeUnapred* stari = simbol->oulista;
    simbol->oulista = simbol->oulista->sledeci;

    postavi_sadrzaj(stari->sekcija->sadrzaj, stari->lokacija,
                    (const char*)&simbol->vrednost, 4);
    free(stari);
  }

  razresavanje_neizracunjivog_simbola_konstanta(simbol);
}

void razresavanje_neizracunjivog_simbola_konstanta(Simbol* simbol) {
  if (simbol->vrednost < -2048 || simbol->vrednost > 2047) {
    return;
  }

  for (ObracanjeInstrukcije* obracanje = simbol->oilista; obracanje;
       obracanje = obracanje->sledeci) {
    char oc =
        *dohvati_sadrzaj(obracanje->sekcija->sadrzaj, obracanje->lokacija);
    oc = transliraj_instrukciju_direktno(oc);
    int pomeraj = simbol->vrednost;

    postavi_sadrzaj(obracanje->sekcija->sadrzaj, obracanje->lokacija, &oc,
                    sizeof(oc));

    char reg_reg =
        *dohvati_sadrzaj(obracanje->sekcija->sadrzaj, obracanje->lokacija + 1);
    reg_reg = (char)((((oc & 0xF0) == 0x90) || ((oc & 0xF0) == 0x80))
                         ? (reg_reg & 0xF0) | (R0)
                         : (reg_reg & 0xF0) | (R0));
    postavi_sadrzaj(obracanje->sekcija->sadrzaj, obracanje->lokacija + 1,
                    &reg_reg, 1);

    char reg_pom =
        *dohvati_sadrzaj(obracanje->sekcija->sadrzaj, obracanje->lokacija + 2);
    reg_pom = (char)(reg_pom & 0xF0) | ((pomeraj & 0xF00) >> 8);
    postavi_sadrzaj(obracanje->sekcija->sadrzaj, obracanje->lokacija + 2,
                    &reg_pom, 1);

    char pom = (char)(pomeraj & 0xFF);
    postavi_sadrzaj(obracanje->sekcija->sadrzaj, obracanje->lokacija + 3, &pom,
                    1);
  }
}