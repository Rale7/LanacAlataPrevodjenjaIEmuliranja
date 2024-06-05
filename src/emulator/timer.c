#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include "../../inc/emulator/procesor.h"
#include "../../inc/emulator/timer.h"

static int dovhati_vrednost_timer(Segment* segment, unsigned int adresa) {
  return 0;
}

static void postavi_vrednost_timer(Segment* segment, unsigned int adresa, int vrednost) {
  Timer* timer = (Timer*) segment;

  timer->tim_cfg = vrednost;
  if (timer->tim_cfg < 0 || timer->tim_cfg > 7) {
    timer->tim_cfg = 0;
  }
}

static void obrisi_timer(Segment* segment) {
  Timer* timer = (Timer*) segment;

  free(timer);
}

Segment* init_timer(unsigned int pocetna_adresa, unsigned int krajnja_adresa, Procesor* procesor) {
  static SegmentTVF timer_tvf = {
    .dohvati_vrednost = &dovhati_vrednost_timer,
    .postavi_vrednost = &postavi_vrednost_timer,
    .obrisi_segment = &obrisi_timer,
  };

  Timer* timer = (Timer*) malloc(sizeof(Timer));
  if (timer == NULL) {
    perror("Greska u alokaciji\n");
    exit(1);
  }

  timer->segment.desni = NULL;
  timer->segment.levi = NULL;
  timer->segment.pocetna_adresa = pocetna_adresa;
  timer->segment.pocetna_adresa = krajnja_adresa;
  timer->segment.visina = 1;
  timer->segment.tvf = &timer_tvf;

  timer->tim_cfg = 0;
  timer->procesor = procesor;
    
}

void* rad_tajmera(void* timer_arg) {
  static unsigned int times_in_miliseconds[] = {500, 1000, 1500, 2000, 5000, 10000, 30000, 60000};

  Timer* timer = (Timer*) timer_arg;
  struct timespec ts;

  while (timer->procesor->working){
    ts.tv_sec = times_in_miliseconds[timer->tim_cfg] / 1000;
    ts.tv_nsec = (times_in_miliseconds[timer->tim_cfg] % 1000) * 1000000;
  
    nanosleep(&ts, &ts);

    sem_wait(&timer->procesor->semaphore[IRQ_TIMER]);

    timer->procesor->irq[IRQ_TIMER] = 1;

    sem_post(&timer->procesor->semaphore[IRQ_TIMER]);
  }

}