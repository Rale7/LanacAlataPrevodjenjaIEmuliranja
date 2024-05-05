#ifndef DIREKTIVE_H
#define DIREKTIVE_H

#include "izraz.h"

typedef void (*CBC)(const char* simbol);

void dodaj_labelu(const char* simbol);

void global_dir(const char* simbol);

void extern_dir(const char* simbol);

void section_dir(const char* simbol);

void word_dir_sim(const char* simbol);

void word_dir_literal(int literal);

void skip_dir(int literal);

void equ_dir(const char* simbol, Izraz* izraz);

void ascii_dir(const char*);

void end_dir();

#endif
