#include <stdlib.h>
#include <stdio.h>
#include "../../inc/asembler/izraz.h"
#include "../../inc/asembler/asembler.h"
#include "../../inc/asembler/tabela_simbola.h"
#include "../../inc/asembler/stek_izraz.h"
#include "../../inc/asembler/neizracunjivi_simbol.h"
#include "../../inc/asembler/tabela_neizracunjivih_simbola.h"

static SekcijaRel* init_sekcija_rel(int broj_pojavljivanja, Sekcija* sekcija) {
  SekcijaRel* novi = (SekcijaRel*) malloc(sizeof(SekcijaRel));
  if (novi == NULL) {
    exit(1);
  }

  novi->broj_pojavljivanja = broj_pojavljivanja;
  novi->sekcija = sekcija;

  return novi;
}

static SekcijaRel* dodaj_pojavljivanje(SekcijaRel* prvi, Sekcija* sekcija) {
  SekcijaRel** indirect;

  for (indirect = &prvi; *indirect; indirect = &(*indirect)->sledeci) {
    if ((*indirect)->sekcija->simbol->redosled == sekcija->simbol->redosled && sekcija->simbol->redosled == 0) {
      printf("Izraz nije pravilan\n");
      exit(1);
    }
    if ((*indirect)->sekcija->simbol->redosled == sekcija->simbol->redosled) {
      (*indirect)->broj_pojavljivanja++;
      return prvi;
    }
    if ((*indirect)->broj_pojavljivanja == 0) {
      SekcijaRel* stari;
      (*indirect) = (*indirect)->sledeci;
      free(stari);
    }
  }

  (*indirect) = init_sekcija_rel(1, sekcija);

  return prvi;
}

static SekcijaRel* skloni_pojavljivanje(SekcijaRel* prvi, Sekcija* sekcija) {
  SekcijaRel** indirect;

  for (indirect = &prvi; *indirect; indirect = &(*indirect)->sledeci) {
    if ((*indirect)->sekcija->simbol->redosled == sekcija->simbol->redosled && sekcija->simbol->redosled == 0) {
      printf("Izraz nije pravilan\n");
      exit(1);
    }
    if ((*indirect)->sekcija->simbol->redosled == sekcija->simbol->redosled) {
      (*indirect)->broj_pojavljivanja--;
      return prvi;
    }
    if ((*indirect)->broj_pojavljivanja == 0) {
      SekcijaRel* stari;
      (*indirect) = (*indirect)->sledeci;
      free(stari);
    }
  }

  (*indirect) = init_sekcija_rel(-1, sekcija);

  return prvi;
}

static int sab_oduz_rp() {
  return 2;
}

static int mno_delj_rp() {
  return 3;
}

static ClanIzraza* sab_oper(ClanIzraza* op1, ClanIzraza* op2) {
  int vrednost1 = (op1->klasifikator == SIMBOL ? op1->deo.simbol->vrednost : op1->deo.literal);
  int vrednost2 = (op2->klasifikator == SIMBOL ? op2->deo.simbol->vrednost : op2->deo.literal);

  ClanIzraza* novi = init_clan_izraza_literal(vrednost1 + vrednost2);

  return novi;
}

static ClanIzraza* oduz_oper(ClanIzraza* op1, ClanIzraza* op2) {
  int vrednost1 = (op1->klasifikator == SIMBOL ? op1->deo.simbol->vrednost : op1->deo.literal);
  int vrednost2 = (op2->klasifikator == SIMBOL ? op2->deo.simbol->vrednost : op2->deo.literal);

  ClanIzraza* novi = init_clan_izraza_literal(vrednost1 - vrednost2);

  return novi;
}

static ClanIzraza* mno_oper(ClanIzraza* op1, ClanIzraza* op2) {
  int vrednost1 = (op1->klasifikator == SIMBOL ? op1->deo.simbol->vrednost : op1->deo.literal);
  int vrednost2 = (op2->klasifikator == SIMBOL ? op2->deo.simbol->vrednost : op2->deo.literal);

  ClanIzraza* novi = init_clan_izraza_literal(vrednost1 * vrednost2);

  return novi;
}

static ClanIzraza* delj_oper(ClanIzraza* op1, ClanIzraza* op2) {
  int vrednost1 = (op1->klasifikator == SIMBOL ? op1->deo.simbol->vrednost : op1->deo.literal);
  int vrednost2 = (op2->klasifikator == SIMBOL ? op2->deo.simbol->vrednost : op2->deo.literal);

  ClanIzraza* novi = init_clan_izraza_literal(vrednost1 / vrednost2);

  return novi;
}

static SekcijaRel* greska_relokatibilnost(SekcijaRel* sekcija_rel, Sekcija* sekcija) {

  printf("Simbol ne moze biti relokativan u odnosu na mnozenje i deljenje\n");
  exit(1);
}

static Operator_TVF optvf[] = {
  {
    .irp = &sab_oduz_rp,
    .srp = &sab_oduz_rp,
    .operacija = &sab_oper,
    .dodaj_relokatibilnost = &dodaj_pojavljivanje
  }, {
    .irp = &sab_oduz_rp,
    .srp = &sab_oduz_rp,
    .operacija = &oduz_oper,
    .dodaj_relokatibilnost = &skloni_pojavljivanje,
  } , {
    .irp = &mno_delj_rp,
    .srp = &mno_delj_rp,
    .operacija = &mno_oper,
    .dodaj_relokatibilnost = &greska_relokatibilnost
  } , {
    .irp = &mno_delj_rp,
    .srp = &mno_delj_rp,
    .operacija = &delj_oper,
    .dodaj_relokatibilnost = &greska_relokatibilnost
  }
};

Izraz* init_Izraz() {

  Izraz* novi = (Izraz*) malloc(sizeof(Izraz));
  if (novi == NULL) {
    exit(1);
  }

  novi->prvi = NULL;
  novi->indirect = &(novi->prvi);
  novi->relokatibilan = NULL;
}

void obrisi_izraz(Izraz* izraz) {
  while(izraz->prvi) {
    ClanIzraza* stari = izraz->prvi;
    izraz->prvi = izraz->prvi->sledeci;
    free(stari);
  }

  free(izraz);
}

static ClanIzraza* init_clan_izraza() {

  ClanIzraza* novi = (ClanIzraza*) malloc(sizeof(ClanIzraza));
  if (novi == NULL) {
    exit(1);
  }

  novi->sledeci = NULL;

  return novi;
}

ClanIzraza* init_clan_izraza_simbol(const char* naziv_simbola) {

  Simbol* simbol;

  if ((simbol = dohvati_vrednost_simbola(dohvati_asembler()->tabel_simbola, naziv_simbola)) == NULL) {
    simbol = init_nedefinisan_simbol(naziv_simbola);
    dodaj_simbol(dohvati_asembler()->tabel_simbola, simbol);
  }
  
  ClanIzraza* novi = init_clan_izraza();

  novi->deo.simbol = simbol;
  novi->klasifikator = SIMBOL;

  return novi;
}

ClanIzraza* init_clan_izraza_literal(int literal) {

  ClanIzraza* novi = init_clan_izraza();

  novi->deo.literal = literal;
  novi->klasifikator = LITERAL;

  return novi;

}

ClanIzraza* init_clan_izraza_operator(enum Operator operator) {

  ClanIzraza* novi = init_clan_izraza();

  novi->deo.operator = &optvf[operator];
  novi->klasifikator = OPERATOR;

  return novi;
}

void dodaj_clan(Izraz* izraz, ClanIzraza* clan_izraza) {

  *(izraz->indirect) = clan_izraza;
  izraz->indirect = &(clan_izraza->sledeci);

}

void prebaci_postfix(Izraz* izraz) {

  StekCvor* vrh = NULL;
  ClanIzraza* trenutni = izraz->prvi;
  izraz->prvi = NULL;
  izraz->indirect = &(izraz->prvi);

  for (; trenutni; trenutni = trenutni->sledeci) {

    if (trenutni->klasifikator != OPERATOR) {
      dodaj_clan(izraz, trenutni);
    } else {
      while (!prazan_stek(vrh) && trenutni->deo.operator->irp() <= top_stek(vrh)->deo.operator->srp()) {
        ClanIzraza* sa_vrha = top_stek(vrh);
        vrh = pop_stek(vrh);
        dodaj_clan(izraz, sa_vrha);
      }
      vrh = push_stek(vrh, trenutni);
    }
  }

  while (!prazan_stek(vrh)) {
    ClanIzraza* sa_vrha = top_stek(vrh);
    sa_vrha->sledeci = NULL;
    vrh = pop_stek(vrh);
    dodaj_clan(izraz, sa_vrha);
  }
}

Simbol* proveri_relokatibilnost_init_simbol(Izraz* izraz, Sekcija* sekcija, const char* naziv_simbola) {

  Sekcija* sekcija_rel;
  enum Relokatibilnost status = proveri_relokatibilnost(izraz, &sekcija_rel);

  if (status == NEIZRACUNJIV) {
    NeizracunjiviSimbol* novi = init_neizracunjivi_simbol(naziv_simbola, sekcija, izraz);
    dodaj_neizracunjivi_simbol(dohvati_asembler()->tabela_neizrazunljivih_simbola, novi); 
    return novi->simbol;
  } else if (status == APSOLUTAN) {
    return init_simbolicka_konstanta(naziv_simbola, izraz, sekcija);
  } else if (status == RELOKATIVAN) {
    return init_definisan_simbol(naziv_simbola, izracunaj_vrednost_izraza(izraz), sekcija_rel);
  }

}

enum Relokatibilnost proveri_relokatibilnost(Izraz* izraz, Sekcija** sekcija) {
  StekCvor* vrh = NULL;
  SekcijaRel* sekcija_rel = NULL;

  for (ClanIzraza* trenutni = izraz->prvi; trenutni; trenutni = trenutni->sledeci) {
    if (trenutni->klasifikator == OPERATOR) {
      ClanIzraza* op1 = top_stek(vrh);
      vrh = pop_stek(vrh);

      if (op1->klasifikator == LITERAL) continue;

      Sekcija* relokativan;
      enum Relokatibilnost status = op1->deo.simbol->tvf->dohvati_relokatibilnost(op1->deo.simbol, &relokativan);

      if (status == NEIZRACUNJIV) {
        return NEIZRACUNJIV;
      } else if (status == RELOKATIVAN) {
        sekcija_rel = trenutni->deo.operator->dodaj_relokatibilnost(sekcija_rel, relokativan);
      }
    } else {
      vrh = push_stek(vrh, trenutni);
    }
  }

  while (!prazan_stek(vrh)) {
    ClanIzraza* op1 = top_stek(vrh);
    vrh = pop_stek(vrh);

    if (op1->klasifikator == LITERAL) continue;

    Sekcija* relokativan;
    enum Relokatibilnost status = op1->deo.simbol->tvf->dohvati_relokatibilnost(op1->deo.simbol, &relokativan);

    if (status == NEIZRACUNJIV) {
      return NEIZRACUNJIV;
    } else if (status == RELOKATIVAN) {
      sekcija_rel = dodaj_pojavljivanje(sekcija_rel, relokativan);
    }
  }

  if (sekcija_rel == NULL) {
    return APSOLUTAN;
  } else if (sekcija_rel->sledeci == NULL && sekcija_rel->broj_pojavljivanja == 1) {
    *sekcija = sekcija_rel->sekcija;
    return RELOKATIVAN;
  } else {
    printf("Izraz nije relokativan\n");
  }

}

int izracunaj_vrednost_izraza(Izraz* izraz) {

  StekCvor* vrh = NULL;
  Izraz* tmp_izraz = init_Izraz();

  for (ClanIzraza* trenutni = izraz->prvi; trenutni; trenutni = trenutni->sledeci) {
    if (trenutni->klasifikator == OPERATOR) {
      ClanIzraza* op1 = top_stek(vrh);
      vrh = pop_stek(vrh);
      ClanIzraza* op2 = top_stek(vrh);
      vrh = pop_stek(vrh);

      ClanIzraza* novi = trenutni->deo.operator->operacija(op2, op1);
      vrh = push_stek(vrh, novi);
      dodaj_clan(izraz, novi);
      
    } else {
      vrh = push_stek(vrh, trenutni);
    }
  }

  ClanIzraza* clan = top_stek(vrh);
  int vrednost;
  if (clan->klasifikator == SIMBOL) {
    vrednost = clan->deo.simbol->vrednost;
  } else {
    vrednost = clan->deo.literal;
  }
  vrh = pop_stek(vrh);

  obrisi_izraz(tmp_izraz);

  return vrednost;

}