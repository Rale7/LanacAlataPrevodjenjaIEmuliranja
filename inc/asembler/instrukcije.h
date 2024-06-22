#ifndef INSTRUKCIJE_H
#define INSTRUKCIJE_H

struct simbol;

enum Registar {
            R0, R1, R2, R3, R4, R5, R6, R7, 
            R8, R9, R10, R11, R12, R13, R14, R15,
};

enum CsrRegistar {
  HANDLER, STATUS, CAUSE
};

void provera_prekoracenja_bazena_inst();

void bezadresna_inst(int kod_operacije);

void instrukcija_sa_pomerajem(char kod_operacije, enum Registar ra, enum Registar rb, enum Registar rc, int pomeraj);

void instrukcija_sa_simbol_bazen(char oc, enum Registar ra, enum Registar rb, enum Registar rc, struct simbol* simbol);

void skok_literal(int kod_operacije, enum Registar rb, enum Registar rc, int literal);

void skok_simbol(int kod_operacije, enum Registar rb, enum Registar rc, const char* naziv_simbola);

void jednoadr_inst(int kod_operacije, enum Registar r1);

void dvoadr_inst(int kod_operacije, enum Registar r1, enum Registar r2);

void ld_imm_simbol(enum Registar r1, const char* simbol);

void ld_imm_literal(enum Registar r1, int literal);

void ld_mem_simbol(enum Registar r1, const char* simbol);

void ld_mem_literal(enum Registar r1, int literal);

void ld_reg_simbol(enum Registar r1, enum Registar r2, const char* simbol);

void ld_reg_literal(enum Registar r1, enum Registar r2, int pomeraj);

void st_mem_simbol(enum Registar r1, const char* simbol);

void st_mem_literal( enum Registar r1, int literal);

void st_reg_simbol(enum Registar r1, enum Registar r2, const char* simbol);

void st_reg_literal(enum Registar r1, enum Registar r2, int pomeraj);

void push_inst(enum Registar r1);

void pop_inst(enum Registar r1);

void csr_inst(int kod_operacije, enum Registar r1, enum Registar r2);

void iret_inst();

char transliraj_instrukciju_direktno(char oc);

char transliraj_instrukciju_pomeraj(char oc);

void instrukcija_prenosa_registara(enum Registar r1, enum Registar r2);

#endif