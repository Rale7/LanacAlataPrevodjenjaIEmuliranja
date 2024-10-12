#include "emulator/terminal.h"

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#include "emulator/memorija.h"
#include "emulator/procesor.h"

static int dohvati_vrednost_terminal_out(Segment* segment,
                                         unsigned int adresa) {
  fprintf(stderr, "Pokusaj citanja sa nedozvoljene adrese %d\n", adresa);
  exit(1);
}

static void postavi_vrednost_terminal_out(Segment* segment, unsigned int adresa,
                                          int vrednost) {
  putchar((char)vrednost);
  fflush(stdout);
}

static void obrisi_terminal_out(Segment* segment) { free(segment); }

Segment* init_segment_terminal_out(int pocetna_adresa, int krajnja_adresa) {
  static SegmentTVF terminal_tvf = {
      .dohvati_vrednost = &dohvati_vrednost_terminal_out,
      .postavi_vrednost = &postavi_vrednost_terminal_out,
      .obrisi_segment = &obrisi_terminal_out,
  };

  Segment* novi = (Segment*)malloc(sizeof(Segment));
  if (novi == NULL) {
    perror("Greska u alokaciji\n");
    exit(1);
  }

  novi->desni = NULL;
  novi->levi = NULL;
  novi->pocetna_adresa = pocetna_adresa;
  novi->krajnja_adresa = krajnja_adresa;
  novi->visina = 1;
  novi->tvf = &terminal_tvf;

  return novi;
}

static int dohvati_vrednost_terminal(Segment* segment, unsigned int adresa) {
  Terminal* terminal = (Terminal*)segment;

  return (int)terminal->karakter & 0xFF;
}

static void postavi_vrednost_terminal(Segment* segment, unsigned int adresa,
                                      int vrednost) {}

static void obrisi_terminal(Segment* segment) {
  Terminal* terminal = (Terminal*)segment;

  free(terminal);
}

struct termios stari;

void vrati_terminal() { tcsetattr(STDIN_FILENO, TCSANOW, &stari); }

void handle_signal(int signal) {
  vrati_terminal();
  exit(EXIT_FAILURE);
}

void podesi_terminal() {
  struct termios moj_terminal;

  if (!isatty(STDIN_FILENO)) {
    perror("Nema terminala\n");
    exit(1);
  }

  tcgetattr(STDIN_FILENO, &stari);
  atexit(vrati_terminal);

  tcgetattr(STDIN_FILENO, &moj_terminal);
  moj_terminal.c_lflag &= ~(ICANON | ECHO | ECHONL | IEXTEN);
  moj_terminal.c_cc[VMIN] = 1;
  moj_terminal.c_cc[VTIME] = 0;
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &moj_terminal);
}

Segment* init_segment_terminal(int pocetna_adresa, int krajnja_adresa,
                               Procesor* procesor) {
  static SegmentTVF terminal_tvf = {
      .dohvati_vrednost = &dohvati_vrednost_terminal,
      .postavi_vrednost = &postavi_vrednost_terminal,
      .obrisi_segment = &obrisi_terminal,
  };

  Terminal* terminal = (Terminal*)malloc(sizeof(Terminal));
  if (terminal == NULL) {
    perror("Greska u alokaciji\n");
    exit(1);
  }

  terminal->segment.desni = NULL;
  terminal->segment.levi = NULL;
  terminal->segment.pocetna_adresa = pocetna_adresa;
  terminal->segment.krajnja_adresa = krajnja_adresa;
  terminal->segment.visina = 1;
  terminal->segment.tvf = &terminal_tvf;
  terminal->procesor = procesor;

  signal(SIGINT, handle_signal);
  signal(SIGTERM, handle_signal);
  signal(SIGSEGV, handle_signal);
  signal(SIGABRT, handle_signal);

  podesi_terminal();

  return (Segment*)terminal;
}

void* terminal_radi(void* terminal_arg) {
  Terminal* terminal = (Terminal*)terminal_arg;

  while (terminal->procesor->working) {
    read(STDIN_FILENO, &terminal->karakter, 1);

    sem_wait(&terminal->procesor->semaphore[IRQ_TERMINAL]);

    terminal->procesor->irq[IRQ_TERMINAL] = 1;

    sem_post(&terminal->procesor->semaphore[IRQ_TERMINAL]);
  }
}