#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <elf.h>
#include "../../inc/emulator/racunar.h"

void ucitaj_segment(Racunar* racunar, int fd, Elf32_Off offset, Elf32_Addr vaddr, Elf32_Xword filesz) {
  if (filesz == 0) return;

  char* sadrzaj = (char*) malloc(sizeof(char) * filesz);
  lseek(fd, offset, SEEK_SET);

  read(fd, sadrzaj, filesz);

  ubaci_segment(racunar->memorija, init_segment_sadrzaj((unsigned int)vaddr, (unsigned int )(vaddr) + filesz - 1, sadrzaj));
}

Racunar* init_racunar(const char* ime_ulaznog_fajla) {

  Racunar* novi = (Racunar*) malloc(sizeof(Racunar));
  if (novi == NULL) {
    printf("Greska pri alokaciji memorije\n");
    exit(1);
  }

  novi->procesor = init_procesor();
  novi->memorija = init_memorija();

  int fd = open(ime_ulaznog_fajla, O_RDONLY);
  if (fd < 0) {
    perror("Pogresno navedena datoteka\n");
    exit(1);
  }

  Elf32_Ehdr elf_header;
  read(fd, &elf_header, sizeof(elf_header));

  Elf32_Off phoffs = elf_header.e_phoff;
  Elf32_Half phentsize = elf_header.e_phentsize;
  Elf32_Half phnum = elf_header.e_phnum;

  Elf32_Phdr* program_header_table = malloc(phnum * sizeof(Elf32_Phdr));
  if (program_header_table == NULL) {
    perror("Greska u alokaciji memorije\n");
    exit(1);
  }

  lseek(fd, phoffs, SEEK_SET);
  read(fd, program_header_table, phentsize * phnum);

  for (int i = 0; i < phnum; i++) {
    
    if (program_header_table[i].p_type != PT_LOAD) continue;

    ucitaj_segment(
      novi, fd, program_header_table[i].p_offset,
      program_header_table[i].p_vaddr,
      program_header_table[i].p_filesz
    );
  }

  close(fd);

  novi->procesor->gpr[PC] = elf_header.e_entry;

  return novi;
}

void obrada_prekida(Procesor* procesor, Memorija* memorija, int cause, int IRQ) {
  procesor->gpr[SP] -= 4;
  postavi_vrednost(memorija, procesor->gpr[SP], procesor->csr[STATUS]);
  procesor->gpr[SP] -= 4;
  postavi_vrednost(memorija, procesor->gpr[SP], procesor->gpr[PC]); 
  procesor->csr[CAUSE] = cause;
  procesor->csr[STATUS] = procesor->gpr[STATUS] | STATUSI;
  procesor->gpr[PC] = procesor->csr[HANDLER];
  procesor->irq[IRQ] = 0;
}

void provera_prekida(Procesor* procesor, Memorija* memorija) {

  if (procesor->csr[STATUS] & STATUSI) return;

  if (!(procesor->csr[STATUS] & STATUSTl)) {
    sem_wait(&procesor->semaphore[IRQ_TERMINAL]);

    if (procesor->irq[IRQ_TERMINAL]) {
      obrada_prekida(procesor, memorija, TERMINAL, IRQ_TERMINAL);
    }

    sem_post(&procesor->semaphore[IRQ_TERMINAL]);
  }

  if (procesor->csr[STATUS] & STATUSI) return;

  if (!(procesor->csr[STATUS] & STATUSTr)) {
    sem_wait(&procesor->semaphore[IRQ_TIMER]);

    if (procesor->irq[IRQ_TIMER]) {
      obrada_prekida(procesor, memorija, TIMER, IRQ_TIMER);
    }

    sem_post(&procesor->semaphore[IRQ_TIMER]);
  }
}

void* rad_racunara(void* racunar_arg) {

  Racunar* racunar = (Racunar*) racunar_arg;

  unsigned char instrukcija[4];
  int indeks;
  Instrukcija* trenutna; 
  int status;

  while (1) {
    int procitana_instrukcija = dohvati_vrednost(racunar->memorija, racunar->procesor->gpr[PC]); 

    instrukcija[0] = (unsigned char) (procitana_instrukcija & 0xFF);
    instrukcija[1] = (unsigned char) ((procitana_instrukcija & 0xFF00) >> 8);
    instrukcija[2] = (unsigned char) ((procitana_instrukcija & 0xFF0000) >> 16);
    instrukcija[3] = (unsigned char) ((procitana_instrukcija & 0xFF000000) >> 24);

    unsigned char indeks = instrukcija[0];

    if (racunar->procesor->instrukcije[indeks]) {
      racunar->procesor->gpr[PC] += 4;
      status = racunar->procesor->instrukcije[indeks](racunar->procesor, racunar->memorija, instrukcija);
    } else {
      status = -1;
    }

    if (status != 0) return (void*) (long)status;

    provera_prekida(racunar->procesor, racunar->memorija);
  }

}