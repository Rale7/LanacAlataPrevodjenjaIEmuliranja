#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "asembler/asembler.h"
#include "asembler/neizracunjivi_simbol.h"
#include "asembler/sadrzaj_sekcije.h"
#include "asembler/sekcija.h"
#include "asembler/simbol.h"
#include "asembler/tabela_simbola.h"

int yyparse();

extern int optind;

int main(int argc, char* argv[]) {
  int opt;
  const char* ime_izlazne_datoteke;
  const char* ime_ulazne_datoteke;

  while ((opt = getopt(argc, argv, "o:")) != -1) {
    switch (opt) {
      case 'o':
        ime_izlazne_datoteke = optarg;
        break;
      default:
        printf("Nepoznata opcija\n");
        exit(1);
    }
  }

  ime_ulazne_datoteke = argv[optind];

  int file_dsc = open(ime_ulazne_datoteke, O_RDONLY);

  if (file_dsc < 0) {
    printf("Opening file error\n");
    exit(1);
  }

  close(STDIN_FILENO);
  dup(file_dsc);

  int status = yyparse();

  close(file_dsc);

  if (status == 0) {
    napravi_elf_file(dohvati_asembler(), ime_izlazne_datoteke);
  }

  obrisi_asembler(dohvati_asembler());

  return status;
}