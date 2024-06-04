#ifndef TIMER_H
#define TIMER_H

#include "memorija.h"

struct procesor;

typedef struct timer {
  Segment segment;
  int tim_cfg;
  struct procesor* procesor;
} Timer;

Segment* init_timer(unsigned int, unsigned int, struct procesor*);

void* rad_tajmera(void*);

#endif