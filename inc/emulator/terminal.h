#ifndef TERMINAL_H
#define TERMINAL_H

#include "memorija.h"

struct procesor;

typedef struct terminal {
  Segment segment;
  char karakter;
  struct procesor* procesor;
} Terminal;

struct segment* init_segment_terminal_out(int, int);

struct segment* init_segment_terminal(int, int, struct procesor*);

void* terminal_radi(void* terminal_arg);

#endif