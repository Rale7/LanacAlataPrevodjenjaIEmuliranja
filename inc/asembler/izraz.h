#ifndef IZRAZ_H
#define IZRAZ_H

struct simbol;
struct sekcija;
struct clan_izraza;
struct sekcija_rel;

enum Operator {SABIRANJE, ODUZIMANJE, MNOZENJE, DELJENJE, OTVORENA_ZAGRADA, ZATVORENA_ZAGRADA};

enum KlasifikatorDelaIzraza {SIMBOL, OPERATOR, LITERAL};

typedef int (*IRP)();
typedef int (*SRP)();
typedef struct clan_izraza* (*Operacija)(struct clan_izraza* op1, struct clan_izraza* op2);
typedef struct sekcija_rel* (*DodajRelokatibilnost)(struct sekcija_rel*, struct sekcija*);

typedef struct {
  IRP irp;
  SRP srp;
  Operacija operacija;
  DodajRelokatibilnost dodaj_relokatibilnost;
} Operator_TVF;

typedef union {
  struct simbol* simbol;
  Operator_TVF* operator;
  int literal;
} DeoIzraza;

typedef struct clan_izraza {
  enum KlasifikatorDelaIzraza klasifikator;
  DeoIzraza deo;
  struct clan_izraza* sledeci;
} ClanIzraza;

typedef struct izraz {
  struct sekcija* relokatibilan;
  ClanIzraza* prvi;
  ClanIzraza** indirect;
} Izraz;

typedef struct sekcija_rel {
  int broj_pojavljivanja;
  struct sekcija* sekcija;
  struct sekcija_rel* sledeci;
} SekcijaRel;

Izraz* init_Izraz();

ClanIzraza* init_clan_izraza_simbol(const char* simbol);

ClanIzraza* init_clan_izraza_literal(int literal);

ClanIzraza* init_clan_izraza_operator(enum Operator);

void obrisi_izraz(Izraz*);

void dodaj_clan(Izraz*, ClanIzraza*);

void prebaci_postfix(Izraz*);

enum Relokatibilnost {APSOLUTAN, RELOKATIVAN, NEIZRACUNJIV};

enum Relokatibilnost proveri_relokatibilnost(Izraz* izraz, struct sekcija** sekcija);

struct simbol* proveri_relokatibilnost_init_simbol(Izraz*, struct sekcija*, const char*);

int izracunaj_vrednost_izraza(Izraz*);

struct simbol* dohvati_referisani_simbol(Izraz* izraz);

#endif