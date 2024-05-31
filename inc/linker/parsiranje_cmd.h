#ifndef PARSIRANJE_CMD_H
#define PARSIRANJE_CMD_H

struct linker;

enum TipLinkovanja {NEPOZNATO, IZVRSNI, RELOKATIVNI};

typedef struct cmd_sekcija {
  const char* ime;
  int va;
  struct cmd_sekcija* sledeci;
} CmdSekcija;

CmdSekcija* init_cmd_sekcija(const char* ime, int va);

enum TipLinkovanja parsiraj(struct linker*, int argc, char* arg[], const char** izlazna_datoteka);

#endif