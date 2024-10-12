#include "emulator/procesor.h"

#include <stdio.h>
#include <stdlib.h>

#include "emulator/memorija.h"

#define BROJ_INSTRUKCIJA 256

static inline void push(Procesor* procesor, Memorija* memorija,
                        char broj_registra) {
  procesor->gpr[SP] -= 4;
  postavi_vrednost(memorija, procesor->gpr[SP], procesor->gpr[broj_registra]);
}

static inline void pop(Procesor* procesor, Memorija* memorija,
                       char broj_registra) {
  procesor->gpr[broj_registra] = dohvati_vrednost(memorija, procesor->gpr[SP]);
  procesor->gpr[SP] += 4;
}

static inline void dohvati_registre_pomeraj(char instrukcija[4], char* ra,
                                            char* rb, char* rc, int* disp) {
  if (ra) {
    *ra = (instrukcija[1] & 0xF0) >> 4;
    *ra &= 0xF;
  }

  if (rb) {
    *rb = (instrukcija[1] & 0x0F);
  }

  if (rc) {
    *rc = (instrukcija[2] & 0xF0) >> 4;
    *rc &= 0xF;
  }

  if (disp) {
    *disp =
        ((((int)instrukcija[2]) & 0x0F) << 8) | ((int)instrukcija[3] & 0xFF);
    if (*disp & 0x800) {
      *disp = (int)(((unsigned int)(*disp)) | 0xFFFFF000U);
    } else {
      *disp &= 0xFFF;
    }
  }
}

static int halt(Procesor* procesor, Memorija* memorija, char instrukcija[4]) {
  return 1;
}

static int inc(Procesor* procesor, Memorija* memorija, char instrukcija[4]) {
  procesor->gpr[SP] -= 4;
  postavi_vrednost(memorija, procesor->gpr[SP], procesor->csr[STATUS]);
  push(procesor, memorija, PC);
  procesor->csr[CAUSE] = SOFTWARE_INT;
  procesor->csr[STATUS] = procesor->gpr[STATUS] & (~0x01);
  procesor->gpr[PC] = procesor->csr[HANDLER];

  return 0;
}

static int call_direct(Procesor* procesor, Memorija* memorija,
                       char instrukcija[4]) {
  push(procesor, memorija, PC);
  char ra, rb, rc;
  int disp;
  dohvati_registre_pomeraj(instrukcija, &ra, &rb, NULL, &disp);
  procesor->gpr[PC] = procesor->gpr[ra] + procesor->gpr[rb] + disp;

  return 0;
}

static int call_indirect(Procesor* procesor, Memorija* memorija,
                         char instrukcija[4]) {
  push(procesor, memorija, PC);
  char ra, rb, rc;
  int disp;
  dohvati_registre_pomeraj(instrukcija, &ra, &rb, NULL, &disp);
  procesor->gpr[PC] =
      dohvati_vrednost(memorija, procesor->gpr[ra] + procesor->gpr[rb] + disp);

  return 0;
}

static int branch_direct(Procesor* procesor, Memorija* memrorija,
                         char instrukcija[4]) {
  char ra;
  int disp;
  dohvati_registre_pomeraj(instrukcija, &ra, NULL, NULL, &disp);
  procesor->gpr[PC] = procesor->gpr[ra] + disp;

  return 0;
}

static int branch_indirect(Procesor* procesor, Memorija* memorija,
                           char instrukcija[4]) {
  char ra;
  int disp;
  dohvati_registre_pomeraj(instrukcija, &ra, NULL, NULL, &disp);
  procesor->gpr[PC] = dohvati_vrednost(memorija, procesor->gpr[ra] + disp);

  return 0;
}

typedef int (*CMP_REG)(int a, int b);

static int cmp_beq(int a, int b) { return a == b; }

static int cmp_bneq(int a, int b) { return a != b; }

static int cmp_bgt(int a, int b) { return a > b; }

static int uslovni_skok_direct(Procesor* procesor, Memorija* memorija,
                               char instrukcija[4], CMP_REG cmp) {
  char ra, rb, rc;
  int disp;
  dohvati_registre_pomeraj(instrukcija, &ra, &rb, &rc, &disp);

  if (cmp(procesor->gpr[rb], procesor->gpr[rc])) {
    procesor->gpr[PC] = procesor->gpr[ra] + disp;
  }

  return 0;
}

static int uslovni_skok_indirect(Procesor* procesor, Memorija* memorija,
                                 char instrukcija[4], CMP_REG cmp) {
  char ra, rb, rc;
  int disp;
  dohvati_registre_pomeraj(instrukcija, &ra, &rb, &rc, &disp);

  if (cmp(procesor->gpr[rb], procesor->gpr[rc])) {
    procesor->gpr[PC] = dohvati_vrednost(memorija, procesor->gpr[ra] + disp);
  }

  return 0;
}

static int beq_direct(Procesor* procesor, Memorija* memorija,
                      char instrukcija[4]) {
  return uslovni_skok_direct(procesor, memorija, instrukcija, &cmp_beq);
}

static int beq_indirect(Procesor* procesor, Memorija* memorija,
                        char instrukcija[4]) {
  return uslovni_skok_indirect(procesor, memorija, instrukcija, &cmp_beq);
}

static int bneq_direct(Procesor* procesor, Memorija* memorija,
                       char instrukcija[4]) {
  return uslovni_skok_direct(procesor, memorija, instrukcija, &cmp_bneq);
}

static int bneq_indirect(Procesor* procesor, Memorija* memorija,
                         char instrukcija[4]) {
  return uslovni_skok_indirect(procesor, memorija, instrukcija, &cmp_bneq);
}

static int bgt_direct(Procesor* procesor, Memorija* memorija,
                      char instrukcija[4]) {
  return uslovni_skok_direct(procesor, memorija, instrukcija, &cmp_bgt);
}

static int bgt_indirect(Procesor* procesor, Memorija* memorija,
                        char instrukcija[4]) {
  return uslovni_skok_indirect(procesor, memorija, instrukcija, &cmp_bgt);
}

static int amoswap(Procesor* procesor, Memorija* memorija,
                   char instrukcija[4]) {
  char rb, rc;
  dohvati_registre_pomeraj(instrukcija, NULL, &rb, &rc, NULL);
  int temp = procesor->gpr[rb];
  procesor->gpr[rb] = procesor->gpr[rc];
  procesor->gpr[rc] = temp;

  return 0;
}

typedef int (*Operacija)(int a, int b);

static int operacije(Procesor* procesor, char instrukcija[4],
                     Operacija operacija) {
  char ra, rb, rc;

  dohvati_registre_pomeraj(instrukcija, &ra, &rb, &rc, NULL);

  if (ra == 0) {
    return 0;
  }

  procesor->gpr[ra] = operacija(procesor->gpr[rb], procesor->gpr[rc]);

  return 0;
}

static int sabiranje_op(int a, int b) { return a + b; }

static int sabiranje(Procesor* procesor, Memorija* memorija,
                     char instrukcija[4]) {
  return operacije(procesor, instrukcija, &sabiranje_op);
}

static int oduzimanje_op(int a, int b) { return a - b; }

static int oduzimanje(Procesor* procesor, Memorija* memorija,
                      char instrukcija[4]) {
  return operacije(procesor, instrukcija, &oduzimanje_op);
}

static int mnozenje_op(int a, int b) { return a * b; }

static int mnozenje(Procesor* procesor, Memorija* memorija,
                    char instruckija[4]) {
  return operacije(procesor, instruckija, &mnozenje_op);
}

static int deljenje_op(int a, int b) { return a / b; }

static int deljenje(Procesor* procesor, Memorija* memorija,
                    char instrukcija[4]) {
  return operacije(procesor, instrukcija, &deljenje_op);
}

static int logicko_ne(Procesor* procesor, Memorija* memorija,
                      char instrukcija[4]) {
  char ra, rb;

  dohvati_registre_pomeraj(instrukcija, &ra, &rb, NULL, NULL);

  if (ra != 0) {
    procesor->gpr[ra] = procesor->gpr[rb];
  }

  return 0;
}

static int iop(int a, int b) { return a & b; }

static int logicko_i(Procesor* procesor, Memorija* memorija,
                     char instrukcija[4]) {
  return operacije(procesor, instrukcija, &iop);
}

static int iliop(int a, int b) { return a | b; }

static int logicko_ili(Procesor* procesor, Memorija* memorija,
                       char instrukcija[4]) {
  return operacije(procesor, instrukcija, &iliop);
}

static int xor (int a, int b) { return a ^ b; }

    static int logicko_xor(Procesor* procesor, Memorija* memroia,
                           char instrukcija[4]) {
  return operacije(procesor, instrukcija, &xor);
}

static int shl(int a, int b) { return a << b; }

static int pomeranje_ulevo(Procesor* procesor, Memorija* memorija,
                           char instrukcija[4]) {
  return operacije(procesor, instrukcija, &shl);
}

static int shr(int a, int b) { return a >> b; }

static int pomeranje_udesno(Procesor* procesor, Memorija* memorija,
                            char instrukcija[4]) {
  return operacije(procesor, instrukcija, &shr);
}

static int st_mem_dir(Procesor* procesor, Memorija* memorija,
                      char instrukcija[4]) {
  char ra, rb, rc;
  int disp;

  dohvati_registre_pomeraj(instrukcija, &ra, &rb, &rc, &disp);

  int adresa = procesor->gpr[ra] + procesor->gpr[rb] + disp;

  postavi_vrednost(memorija, adresa, procesor->gpr[rc]);

  return 0;
}

static int st_mem_ind(Procesor* procesor, Memorija* memorija,
                      char instrukcija[4]) {
  char ra, rb, rc;
  int disp;

  dohvati_registre_pomeraj(instrukcija, &ra, &rb, &rc, &disp);

  int adresa = procesor->gpr[ra] + procesor->gpr[rb] + disp;
  adresa = dohvati_vrednost(memorija, adresa);

  postavi_vrednost(memorija, adresa, procesor->gpr[rc]);

  return 0;
}

static int st_regpom(Procesor* procesor, Memorija* memorija,
                     char instrukcija[4]) {
  char ra, rc;
  int disp;

  dohvati_registre_pomeraj(instrukcija, &ra, NULL, &rc, &disp);

  if (ra != 0) {
    procesor->gpr[ra] = procesor->gpr[ra] + disp;
  }

  postavi_vrednost(memorija, procesor->gpr[ra], procesor->gpr[rc]);

  return 0;
}

static int mov_csr(Procesor* procesor, Memorija* memorija,
                   char instrukcija[4]) {
  char ra, csr;

  dohvati_registre_pomeraj(instrukcija, &ra, &csr, NULL, NULL);

  if (ra != 0) {
    procesor->gpr[ra] = procesor->csr[csr];
  }

  return 0;
}

static int mov_disp(Procesor* procesor, Memorija* memorija,
                    char instrukcija[4]) {
  char ra, rb;
  int disp;

  dohvati_registre_pomeraj(instrukcija, &ra, &rb, NULL, &disp);

  if (ra != 0) {
    procesor->gpr[ra] = procesor->gpr[rb] + disp;
  }

  return 0;
}

static int ld_mem(Procesor* procesor, Memorija* memorija, char instrukcija[4]) {
  char ra, rb, rc;
  int disp;

  dohvati_registre_pomeraj(instrukcija, &ra, &rb, &rc, &disp);

  if (ra != 0) {
    unsigned int adresa = procesor->gpr[rb] + procesor->gpr[rc] + disp;
    procesor->gpr[ra] = dohvati_vrednost(memorija, adresa);
  }

  return 0;
}

static int ld_mem_disp(Procesor* procesor, Memorija* memorija,
                       char instrukcija[4]) {
  char ra, rb;
  int disp;

  dohvati_registre_pomeraj(instrukcija, &ra, &rb, NULL, &disp);

  if (ra != 0) {
    procesor->gpr[ra] = dohvati_vrednost(memorija, procesor->gpr[rb]);
  }

  if (rb != 0) {
    procesor->gpr[rb] += disp;
  }

  return 0;
}

static int ld_csr(Procesor* procesor, Memorija* memorija, char instrukcija[4]) {
  char csr, ra;

  dohvati_registre_pomeraj(instrukcija, &csr, &ra, NULL, NULL);

  procesor->csr[csr] = procesor->gpr[ra];

  return 0;
}

static int ld_csr_mask(Procesor* procesor, Memorija* memorija,
                       char instrukcija[4]) {
  char csr, rb;
  int disp;

  dohvati_registre_pomeraj(instrukcija, &csr, &rb, NULL, &disp);

  procesor->csr[csr] = procesor->gpr[rb] | disp;

  return 0;
}

static int ld_csr_mem(Procesor* procesor, Memorija* memorija,
                      char instrukcija[4]) {
  char csr, rb, rc;
  int disp;

  dohvati_registre_pomeraj(instrukcija, &csr, &rb, &rc, &disp);

  int adresa = procesor->gpr[rb] + procesor->gpr[rc] + disp;

  procesor->csr[csr] = dohvati_vrednost(memorija, adresa);

  return 0;
}

static int ld_csr_mem_disp(Procesor* procesor, Memorija* memorija,
                           char instrukcija[4]) {
  char csr, rb;
  int disp;

  dohvati_registre_pomeraj(instrukcija, &csr, &rb, NULL, &disp);

  procesor->csr[csr] = dohvati_vrednost(memorija, procesor->gpr[rb]);

  if (rb != 0) {
    procesor->gpr[rb] += disp;
  }

  return 0;
}

static Instrukcija* init_instrukcije() {
  Instrukcija* instrukcije =
      (Instrukcija*)calloc(BROJ_INSTRUKCIJA, sizeof(Instrukcija));
  if (instrukcije == NULL) {
    printf("Greska u alokaciji memorije\n");
    exit(1);
  }

  instrukcije[0x00] = &halt;
  instrukcije[0x10] = &inc;
  instrukcije[0x20] = &call_direct;
  instrukcije[0x21] = &call_indirect;
  instrukcije[0x30] = &branch_direct;
  instrukcije[0x31] = &beq_direct;
  instrukcije[0x32] = &bneq_direct;
  instrukcije[0x33] = &bgt_direct;
  instrukcije[0x38] = &branch_indirect;
  instrukcije[0x39] = &beq_indirect;
  instrukcije[0x3A] = &bneq_indirect;
  instrukcije[0x3B] = &bgt_indirect;
  instrukcije[0x40] = &amoswap;
  instrukcije[0x50] = &sabiranje;
  instrukcije[0x51] = &oduzimanje;
  instrukcije[0x52] = &mnozenje;
  instrukcije[0x53] = &deljenje;
  instrukcije[0x60] = &logicko_ne;
  instrukcije[0x61] = &logicko_i;
  instrukcije[0x62] = &logicko_ili;
  instrukcije[0x63] = &logicko_xor;
  instrukcije[0x70] = &pomeranje_ulevo;
  instrukcije[0x71] = &pomeranje_udesno;
  instrukcije[0x80] = &st_mem_dir;
  instrukcije[0x81] = &st_regpom;
  instrukcije[0x82] = &st_mem_ind;
  instrukcije[0x90] = &mov_csr;
  instrukcije[0x91] = &mov_disp;
  instrukcije[0x92] = &ld_mem;
  instrukcije[0x93] = &ld_mem_disp;
  instrukcije[0x94] = &ld_csr;
  instrukcije[0x95] = &ld_csr_mask;
  instrukcije[0x96] = &ld_csr_mem;
  instrukcije[0x97] = &ld_csr_mem_disp;

  return instrukcije;
}

Procesor* init_procesor() {
  static Instrukcija* instrukcije = NULL;

  if (instrukcije == NULL) {
    instrukcije = init_instrukcije();
  }

  Procesor* novi = (Procesor*)malloc(sizeof(Procesor));
  if (novi == NULL) {
    printf("Greska u alokaciji memorije\n");
    exit(1);
  }

  novi->instrukcije = instrukcije;
  sem_init(&novi->semaphore[0], 0, 1);
  sem_init(&novi->semaphore[1], 0, 1);
  novi->irq[0] = 0;
  novi->irq[1] = 0;
  novi->working = 1;

  return novi;
}

void obrisi_procesor(Procesor* procesor) {
  sem_destroy(&procesor->semaphore[0]);
  sem_destroy(&procesor->semaphore[1]);

  free(procesor);
}