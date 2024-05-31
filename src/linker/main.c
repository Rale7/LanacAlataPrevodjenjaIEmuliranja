#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "../../inc/linker/linker.h"
#include "../../inc/linker/parsiranje_cmd.h"

extern int optind;

int main(int argc, char* argv[]) {

  Linker* linker = init_linker();
  const char* ime_izlazne_datoteke;

  enum TipLinkovanja tip = parsiraj(linker, argc, argv, &ime_izlazne_datoteke);

  if (tip == NEPOZNATO) {
    printf("Nije naveden tip linkovanja\n");
    exit(1);
  } else if (tip == IZVRSNI) {
    napravi_izvrsni_fajl(linker, ime_izlazne_datoteke);
  } else if (tip == RELOKATIVNI) {
    napravi_relokativni_fajl(linker, ime_izlazne_datoteke);
  }

  return 0;
}