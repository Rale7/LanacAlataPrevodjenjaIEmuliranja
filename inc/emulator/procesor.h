#ifndef PROCESOR_H
#define PROCESOR_H

#define R0 0
#define R1 1
#define R2 2
#define R3 3
#define R4 4
#define R5 5
#define R6 6
#define R7 7
#define R8 8
#define R9 9
#define RA 10
#define RB 11
#define RC 12
#define RD 13
#define RE 14
#define SP 14
#define RF 15
#define PC 15
#define STATUS 0
#define HANDLER 1
#define CAUSE 2
#define INVALID_OC 1
#define TIMER 2
#define TERMINAL 3
#define SOFTWARE_INT 4

struct procesor;
struct memorija;

typedef int (*Instrukcija)(struct procesor*, struct memorija*, char instrukcija[4]);

typedef struct procesor {
  Instrukcija* instrukcije;
  int gpr[16];
  int csr[3];
} Procesor;

Procesor* init_procesor();

#endif