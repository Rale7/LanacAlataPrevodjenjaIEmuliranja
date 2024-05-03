#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "../../inc/asembler/neizracunjivi_simbol.h"
#include "../../inc/asembler/sadrzaj_sekcije.h"
#include "../../inc/asembler/sekcija.h"
#include "../../inc/asembler/simbol.h"
#include "../../inc/asembler/tabela_simbola.h"
#include "../../inc/asembler/asembler.h"

int yyparse();

int main() {
  int file_dsc = open("./tests/main.s", O_RDONLY);
  if (file_dsc < 0) {
    printf("Opening file error\n");
    exit(1);
  }

  close(STDIN_FILENO);
  dup(file_dsc);

  int status = yyparse();

  close(file_dsc);

  return status;

}