#include <stdlib.h>
#include "../../inc/emulator/racunar.h"

Racunar* init_racunar() {

  Racunar* novi = (Racunar*) malloc(sizeof(Racunar));
  if (novi == NULL) {
    printf("Greska pri alokaciji memorije\n");
    exit(1);
  }
}