#include <stdlib.h>
#include <stdio.h>
#include "../../inc/emulator/procesor.h"
#include "../../inc/emulator/memorija.h"

#define BROJ_INSTRUKCIJA 256

static inline void push(Procesor* procesor, Memorija* memorija, char broj_registra) {
  procesor->gpr[SP]--;
  postavi_vrednost(memorija, procesor->gpr[SP], procesor->gpr[broj_registra]);
}

static inline void pop(Procesor* procesor, Memorija* memorija, char broj_registra) {
  procesor->gpr[broj_registra] = dohvati_vrednost(memorija, procesor->gpr[SP]);
  procesor->gpr[SP]++;
}

static inline void dohvati_registre_pomeraj(char instrukcija[4], char* ra, char* rb, char* rc, int* disp) {

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
    *disp = ((instrukcija[2] & 0x0F)<< 4) | instrukcija[3];
    if (*disp & 0x400) {
      *disp = (int) (((unsigned int) (*disp))| 0xFFFFF000U);
    } else {
      *disp &= 0xFFF;
    }
  }

}

static int halt(Procesor* procesor, Memorija* memorija, char instrukcija[4]) {
  return 1;
}

static int inc(Procesor* procesor, Memorija* memorija, char instrukcija[4]) {
  procesor->gpr[SP]--;
  postavi_vrednost(memorija, procesor->gpr[SP], procesor->csr[STATUS]);
  push(procesor, memorija, PC);
  procesor->csr[CAUSE] = SOFTWARE_INT;
  procesor->csr[STATUS] = procesor->gpr[STATUS] & (~0x01);
  procesor->gpr[PC] = procesor->csr[HANDLER];

  return 0;
}

static int call_direct(Procesor* procesor, Memorija* memorija, char instrukcija[4]) {
  push(procesor, memorija, PC);
  char ra, rb, rc;
  int disp;
  dohvati_registre_pomeraj(instrukcija, &ra, &rb, NULL, &disp);
  procesor->gpr[PC] = procesor->gpr[ra] + procesor->gpr[rb] + disp;

  return 0;
}

static int call_indirect(Procesor* procesor, Memorija* memorija, char instrukcija[4]) {
  push(procesor, memorija, PC);
  char ra, rb, rc;
  int disp;
  dohvati_registre_pomeraj(instrukcija, &ra, &rb, NULL, &disp);
  procesor->gpr[PC] = dohvati_vrednost(memorija, procesor->gpr[ra] + procesor->gpr[rb] + disp);

  return 0;
}

static int branch_direct(Procesor* procesor, Memorija* memrorija, char instrukcija[4]) {
  char ra;
  int disp;
  dohvati_registre_pomeraj(instrukcija, &ra, NULL, NULL, &disp);
  procesor->gpr[PC] = procesor->gpr[ra] + disp;

  return 0;
}

static int branch_indirect(Procesor* procesor, Memorija* memorija, char instrukcija[4]) {
  char ra;
  int disp;
  dohvati_registre_pomeraj(instrukcija, &ra, NULL, NULL, &disp);
  procesor->gpr[PC] = dohvati_vrednost(memorija, procesor->gpr[ra] + disp);

  return 0;
}

typedef int (*CMP_REG)(int a, int b);

static int cmp_beq(int a, int b) {
  return a == b;
}

static int cmp_bneq(int a, int b) {
  return a != b;
}

static int cmp_bgt(int a, int b) {
  return a > b;
}

static int uslovni_skok_direct(Procesor* procesor, Memorija* memorija, char instrukcija[4], CMP_REG cmp) {
  char ra, rb, rc;
  int disp;
  dohvati_registre_pomeraj(instrukcija, &ra, &rb, &rc, &disp);

  if (cmp(procesor->gpr[rb], procesor->gpr[rc])) {
    procesor->gpr[PC] = procesor->gpr[ra] + disp;
  }

  return 0;
}

static int uslovni_skok_indirect(Procesor* procesor, Memorija* memorija, char instrukcija[4], CMP_REG cmp) {
  char ra, rb, rc;
  int disp;
  dohvati_registre_pomeraj(instrukcija, &ra, &rb, &rc, &disp);

  if (cmp(procesor->gpr[rb], procesor->gpr[rc])) {
    procesor->gpr[PC] = dohvati_vrednost(memorija, procesor->gpr[ra] + disp); 
  }

  return 0;
}

static int beq_direct(Procesor* procesor, Memorija* memorija, char instrukcija[4]) {
  return uslovni_skok_direct(procesor, memorija, instrukcija, &cmp_beq);
}

static int beq_indirect(Procesor* procesor, Memorija* memorija, char instrukcija[4]) {
  return uslovni_skok_indirect(procesor, memorija, instrukcija, &cmp_beq);
}

static int bneq_direct(Procesor* procesor, Memorija* memorija, char instrukcija[4]) {
  return uslovni_skok_direct(procesor, memorija, instrukcija, &cmp_bneq);  
}

static int bneq_indirect(Procesor* procesor, Memorija* memorija, char instrukcija[4]) {
  return uslovni_skok_indirect(procesor, memorija, instrukcija, &cmp_bneq);  
}

static int bgt_direct(Procesor* procesor, Memorija* memorija, char instrukcija[4]) {
  return uslovni_skok_direct(procesor, memorija, instrukcija, &cmp_bgt); 
}

static int bneq_indirect(Procesor* procesor, Memorija* memorija, char instrukcija[4]) {
  return uslovni_skok_indirect(procesor, memorija, instrukcija, &cmp_bgt);
}

static int amoswap(Procesor* procesor, Memorija* memorija, char instrukcija[4]) {
  char rb, rc;
  dohvati_registre_pomeraj(instrukcija, NULL, &rb, &rc, NULL);
  int temp = procesor->gpr[rb];
  procesor->gpr[rb] = procesor->gpr[rc];
  procesor->gpr[rc] = temp;

  return 0;
}

typedef int (*Operacija)(int a, int b);

static int operacije(Procesor* procesor, char instrukcija[4], Operacija operacija) {

  char ra, rb, rc;

  dohvati_registre_pomeraj(instrukcija, &ra, &rb, &rc, NULL);

  procesor->gpr[ra] = operacija(procesor->gpr[rb], procesor->gpr[rc]);

  return 0;
}

static Instrukcija* init_instrukcije() {
  Instrukcija* instrukcije = (Instrukcija*) calloc(BROJ_INSTRUKCIJA, sizeof(Instrukcija));
  if (instrukcije == NULL) {
    printf("Greska u alokaciji memorije\n");
    exit(1);
  }

  instrukcije[0x00] = &halt;
  instrukcije[0x08] = &inc;

  return instrukcije;
}

Procesor* init_procesor() {
  static Instrukcija* instrukcije = NULL;

  if (instrukcije == NULL) {
    instrukcije = init_instrukcije();
  }

  Procesor* novi = (Procesor*) malloc(sizeof(Procesor));
  if (novi == NULL) {
    printf("Greska u alokaciji memorije\n");
    exit(1);
  }

  return novi;
}