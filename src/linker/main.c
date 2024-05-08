#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "../../inc/linker/linker.h"

extern int optind;

int main(int argc, char* argv[]) {

  Linker* linker = init_linker();
  const char* ime_izlazne_datoteke;

  int opt;

  while ((opt = getopt(argc, argv, "o:")) != -1) {
    
    switch (opt)
    {
    case 'o':
      ime_izlazne_datoteke = optarg;
      break;          
    default:
      printf("Nepoznata opcija\n");
      exit(1);
    }
  }

  for (int i = optind; i < argc; i++) {
    procesiraj_ulazni_fajl(linker, argv[i]);
  }

  napravi_izvrsni_fajl(linker, ime_izlazne_datoteke);

  ispisi_strukture(linker);

  return 0;
}