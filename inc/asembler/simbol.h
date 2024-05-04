#ifndef SIMBOL_H
#define SIMBOL_H

struct sekcija;
struct simbol;
struct rz;

enum Registar;

typedef void (*SimbolInstSkok) (struct simbol*, int, enum Registar, enum Registar, struct sekcija*);
typedef void (*SimbolInstLdImm) (struct simbol*, enum Registar, struct sekcija*);
typedef void (*SimbolInstLdMem) (struct simbol*, enum Registar, struct sekcija*);
typedef void (*SimbolInstLdReg) (struct simbol*, enum Registar, enum Registar, struct sekcija*);
typedef void (*SimbolInstStMem) (struct simbol*, enum Registar, struct sekcija*);
typedef void (*SimbolInstStReg) (struct simbol*, enum Registar, enum Registar, struct sekcija*);
typedef struct rz* (*NapraviRelokacioniZapis) (struct simbol*, struct sekcija* sekcija, int lokacija, int obracanje);
typedef void (*SimbolDirWord) (struct simbol*, struct sekcija*, int lokacija);

typedef struct {
  SimbolInstSkok skok;
  SimbolInstLdImm imm;
  SimbolInstLdImm ld_mem;
  SimbolInstLdReg ld_reg;
  SimbolInstStMem st_mem;
  SimbolInstStReg st_reg;
  NapraviRelokacioniZapis nrz;
  SimbolDirWord sdw;
} Simbol_TVF;

typedef void (*IspisiSimbol) (struct simbol*);
typedef void (*IspisiRelokacioniZapis)(struct simbol*, struct rz*);
typedef int (*DohvatiDodavanje) (struct simbol*);
typedef char (*DohvatiBind) (struct simbol*);
typedef char (*DohvatiTip) (struct simbol*);
typedef int (*DohvatiSimbolRel) (struct simbol*);

typedef struct {
  IspisiSimbol ispis_simbola;
  IspisiRelokacioniZapis ispis_relokacionog_zapisa;
  DohvatiDodavanje dohvati_dodavanje;
  DohvatiTip dohvati_tip;
  DohvatiBind dohvati_bind;
  DohvatiSimbolRel dohvati_simbol_rel;
} Tip_TVF;

typedef struct obracanje_unapred {
  struct obracanje_unapred* sledeci;
  int lokacija;
  struct sekcija* sekcija;
} ObracanjeUnapred;

typedef struct simbol {
  int redosled;
  int vrednost;
  struct simbol* sledeci;
  const char* naziv;
  struct sekcija* sekcija;
  ObracanjeUnapred* oulista;
  Simbol_TVF* tvf;
  Tip_TVF* tip_tvf;
} Simbol;

typedef struct elem {
  
  struct elem* sledeci;
  Simbol* simbol;

} SimbolElem;

Simbol* init_simbol(const char*, int, struct sekcija*);

Simbol* init_definisan_simbol(const char*, int, struct sekcija*);

Simbol* init_nedefinisan_simbol(const char*);

void prebaci_u_definisan(Simbol*, struct sekcija*, int vrednost);

void prebaci_u_globalni(Simbol*);

void ispisi_simbole();

void ugradi_pomeraj_simbol(struct sekcija* sekcija, int obracanje, int pomeraj);

Simbol* dohvati_prvi_simbol();

char dohvati_tip_nedefinisan(Simbol* simbol);

#endif