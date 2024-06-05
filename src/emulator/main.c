#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include "../../inc/emulator/terminal.h"
#include "../../inc/emulator/racunar.h"
#include "../../inc/emulator/timer.h" 

int main(int argc, const char* argv[]) {

  if (argc < 2) {
    perror("Nedovoljan broj argumenata\n");
    exit(1);
  }

  const char* ime_ulaza = argv[1];

  Racunar* racunar = init_racunar(ime_ulaza);
  Segment* terminal = init_segment_terminal(0xFFFFFF04, 0xFFFFFF07, racunar->procesor);
  Segment* terminal_out = init_segment_terminal_out(0xFFFFFF00, 0xFFFFFF03);
  Segment* timer = init_timer(0xFFFFFF10, 0xFFFFFF13, racunar->procesor);

  ubaci_segment(racunar->memorija, terminal);
  ubaci_segment(racunar->memorija, terminal_out);
  ubaci_segment(racunar->memorija, timer);

  int status;
  pthread_t cpu_t, terminal_t, timer_t;

  printf("Start\n");
    
  pthread_create(&cpu_t, NULL, &rad_racunara, racunar);
  pthread_create(&terminal_t, NULL, &terminal_radi, terminal);
  pthread_create(&timer_t, NULL, &rad_tajmera, timer);

  pthread_join(cpu_t, (void**) &status);

  racunar->procesor->working = 0;

  pthread_cancel(terminal_t);
  pthread_cancel(timer_t);
  
  printf("\nEmulated processor ");
  if (status == 1) {
    printf("executed halt instruction\n");
  }
  printf("Emulated processor state:\n");
  printf("r0=0x%08x\t", racunar->procesor->gpr[R0]);
  printf("r1=0x%08x\t", racunar->procesor->gpr[R1]);
  printf("r2=0x%08x\t", racunar->procesor->gpr[R2]);
  printf("r3=0x%08x\n", racunar->procesor->gpr[R3]);
  printf("r4=0x%08x\t", racunar->procesor->gpr[R4]);
  printf("r5=0x%08x\t", racunar->procesor->gpr[R5]);
  printf("r6=0x%08x\t", racunar->procesor->gpr[R6]);
  printf("r7=0x%08x\n", racunar->procesor->gpr[R7]);
  printf("r8=0x%08x\t", racunar->procesor->gpr[R8]);
  printf("r9=0x%08x\t", racunar->procesor->gpr[R9]);
  printf("r10=0x%08x\t", racunar->procesor->gpr[RA]);
  printf("r11=0x%08x\n", racunar->procesor->gpr[RB]);
  printf("r12=0x%08x\t", racunar->procesor->gpr[RC]);
  printf("r13=0x%08x\t", racunar->procesor->gpr[RD]);
  printf("r14=0x%08x\t", racunar->procesor->gpr[RE]);
  printf("r15=0x%08x\n", racunar->procesor->gpr[RF]);

  obrisi_racunar(racunar);

}