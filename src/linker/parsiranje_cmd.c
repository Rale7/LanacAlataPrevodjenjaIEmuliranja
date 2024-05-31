#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "../../inc/linker/parsiranje_cmd.h"
#include "../../inc/linker/linker.h"

CmdSekcija* init_cmd_sekcija(const char* ime, int va) {

  CmdSekcija* nova = (CmdSekcija*) malloc(sizeof(CmdSekcija));
  if (nova == NULL) {
    exit(1);
  }

  nova->ime = ime;
  nova->va = va;
  nova->sledeci = NULL;

  return nova;

}

int parsiraj_broj(const char* broj) {
  int n = strlen(broj);

  if (n > 2 && broj[0] == '0' && (broj[1] == 'x' || broj[1] == 'X')) {
    return strtol(broj + 2, NULL, 16);
  } else if (n > 2 && broj[0] == '0' && (broj[1] == 'b' || broj[1] == 'B')) {
    return strtol(broj + 2, NULL, 2);
  } else if (n > 1 && broj[0] == '0') {
    return strtol(broj + 1, NULL, 8);
  } else {
    return atoi(broj);
  }

}

void place_parsiranje(Linker* linker, char* argument) {

  int cnt = 0;

  while (argument[cnt++] != '=');
  const char* ime_sekcije = argument + cnt;

  while (argument[cnt] != '@') cnt++;
  argument[cnt++] = '\0';

  CmdSekcija* novi =  init_cmd_sekcija(ime_sekcije, parsiraj_broj(argument + cnt));
  *linker->indirect = novi;
  linker->indirect = &novi->sledeci;

}

enum TipLinkovanja parsiraj(Linker* linker, int argc, char* argv[], const char** izlazna_datoteka) {

  enum TipLinkovanja tip = NEPOZNATO;

  for (int i = 1; i < argc; i++) {

    if (strcmp(argv[i], "-o") == 0) {
      if (++i > argc) {
        printf("Nevalidni argumenti\n");
        exit(1);
      }
      *izlazna_datoteka = argv[i];
    } else if (strncmp(argv[i], "-place", sizeof("-place") - 1) == 0) {
      place_parsiranje(linker, argv[i]);
    } else if (strcmp(argv[i], "-hex") == 0) {
      if (tip != NEPOZNATO) {
        printf("Nevalidni argumenti");
        exit(1);
      }

      tip = IZVRSNI;
    } else if (strcmp(argv[i], "-relocatable") == 0) {
      if (tip != NEPOZNATO) {
        printf("Nevalidni argumenti");
        exit(1);
      }

      tip = RELOKATIVNI;
    } else {
      procesiraj_ulazni_fajl(linker, argv[i]);
    }
  } 

  return tip;

}