#include <elf.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "asembler/asembler.h"
#include "asembler/moja_string_sekcija.h"
#include "asembler/relokacioni_zapis.h"

int upisi_relokacione_zapise(int fd, RelokacioniZapis *rel) {
  int cnt = 0;

  for (; rel; rel = rel->sledeci) {
    int simbol_rel = rel->simbol->tip_tvf->dohvati_simbol_rel(rel->simbol);
    if (simbol_rel != -1) {
      cnt++;

      Elf32_Rela relokacioni_zapis = {
          .r_offset = rel->offset,
          .r_info = ELF32_R_INFO(simbol_rel, 0),
          .r_addend = rel->simbol->tip_tvf->dohvati_dodavanje(rel->simbol)};

      write(fd, &relokacioni_zapis, sizeof(Elf32_Rela));
    }
  }

  return cnt;
}

static inline Elf32_Shdr *safe_realloc(Elf32_Shdr *shrd, int broj_zagljavlja) {
  if (broj_zagljavlja > 0 && broj_zagljavlja % 10 == 0) {
    shrd = (Elf32_Shdr *)realloc(shrd,
                                 (broj_zagljavlja + 10) * sizeof(Elf32_Shdr));
    if (shrd == NULL) {
      exit(1);
    }
  }

  return shrd;
}

char *promeni_ektenziju(const char *ulazni_fajl) {
  char *pozicija_tacke = strchr(ulazni_fajl, '.');
  int n = (pozicija_tacke ? (pozicija_tacke - ulazni_fajl)
                          : strlen(ulazni_fajl) - 1);

  char *txt_fajl = (char *)malloc(sizeof(char) * (n + 5));

  strncpy(txt_fajl, ulazni_fajl, n);
  txt_fajl[n] = '.';
  txt_fajl[n + 1] = 't';
  txt_fajl[n + 2] = 'x';
  txt_fajl[n + 3] = 't';
  txt_fajl[n + 4] = '\0';

  return txt_fajl;
}

char *int_to_string(int broj) {
  char *string = (char *)malloc(sizeof(char) * 5);

  sprintf(string, "%d", broj);

  return string;
}

void readelf_program(const char *txt_fajl, const char *ulazni_fajl,
                     Elf32_Shdr *zaglavlja, int broj_zaglavlja) {
  int broj_sekcija = 0;

  for (int i = 0; i < broj_zaglavlja; i++) {
    if (zaglavlja[i].sh_type == SHT_PROGBITS) {
      broj_sekcija++;
    }
  }

  char **argv = (char **)malloc(sizeof(char *) * (4 + 2 * broj_sekcija));

  argv[0] = "readelf";
  argv[1] = (char *)ulazni_fajl;
  argv[2] = "-hSsr";

  int index = 3;
  for (int i = 0; i < broj_zaglavlja; i++) {
    if (zaglavlja[i].sh_type == SHT_PROGBITS) {
      argv[index++] = "-x";
      argv[index++] = int_to_string(i);
    }
  }

  argv[index] = NULL;

  int fd = open(txt_fajl, O_WRONLY | O_TRUNC | O_CREAT, 0664);
  if (fd < 0) {
    perror("Greska u otvaranju txt fajla\n");
    exit(1);
  }

  close(STDOUT_FILENO);
  dup(fd);
  close(STDIN_FILENO);

  execvp(argv[0], argv);
  perror("Greska u exec-u");
  exit(1);
}

void napravi_txt_elf_file(const char *ulazni_fajl, Elf32_Shdr *zaglavlja,
                          int broj_zaglavlja) {
  char *txt_fajl = promeni_ektenziju(ulazni_fajl);

  int pid = fork();

  if (pid < 0) {
    perror("Greska u fork-u\n");
    exit(1);
  } else if (pid == 0) {
    readelf_program(txt_fajl, ulazni_fajl, zaglavlja, broj_zaglavlja);
  }

  free(txt_fajl);
}

void napravi_elf_file(Asembler *asembler, const char *izlazni_fajl) {
  int fd = open(izlazni_fajl, O_WRONLY | O_TRUNC | O_CREAT, 0664);
  if (fd < 0) {
    printf("Greska pri otvaranju izlaznog fajla\n");
    exit(1);
  }

  lseek(fd, sizeof(Elf32_Ehdr), SEEK_SET);

  MojaStringSekcija *shstr = init_moja_string_sekcija();

  int broj_zaglavlja = 0;
  Elf32_Shdr *zagljavlja = calloc(10, sizeof(Elf32_Shdr));
  if (zagljavlja == NULL) {
    exit(1);
  }

  zagljavlja[broj_zaglavlja++] = (Elf32_Shdr){
      .sh_name = dodaj_string(shstr, asembler->undefined->simbol->naziv),
      .sh_type = SHT_NULL,
      .sh_flags = 0,
      .sh_addr = 0,
      .sh_offset = 0,
      .sh_size = 0,
      .sh_link = SHN_UNDEF,
      .sh_info = 0,
      .sh_addralign = 0,
      .sh_entsize = 0};
  asembler->undefined->broj_elf_ulaza = 0;

  for (SekcijaElem *sekcija_elem = asembler->sekcije; sekcija_elem;
       sekcija_elem = sekcija_elem->sledeci) {
    Sekcija *sekcija = sekcija_elem->sekcija;

    zagljavlja = safe_realloc(zagljavlja, broj_zaglavlja);

    off_t trenutna_pozicija_sadrzaja = lseek(fd, 0, SEEK_CUR);
    write(fd, sekcija->sadrzaj->byte, sekcija->location_counter);

    int indeks_stringa_sekcija;
    int indeks_stringa_rel;

    if (sekcija->trz->prvi) {
      indeks_stringa_rel =
          dodaj_string_povezano(shstr, "rel", sekcija->simbol->naziv);
      indeks_stringa_sekcija = indeks_stringa_rel + 4;
    } else {
      indeks_stringa_sekcija = dodaj_string(shstr, sekcija->simbol->naziv);
    }
    sekcija->broj_elf_ulaza = broj_zaglavlja;
    zagljavlja[broj_zaglavlja++] =
        (Elf32_Shdr){.sh_name = indeks_stringa_sekcija,
                     .sh_type = SHT_PROGBITS,
                     .sh_flags = SHF_ALLOC,
                     .sh_addr = 0,
                     .sh_offset = trenutna_pozicija_sadrzaja,
                     .sh_size = sekcija->location_counter,
                     .sh_link = 0,
                     .sh_info = 0,
                     .sh_addralign = 0x04,
                     .sh_entsize = 0};

    if (sekcija->trz->prvi) {
      trenutna_pozicija_sadrzaja = lseek(fd, 0, SEEK_CUR);
      int broj_zapisa = upisi_relokacione_zapise(fd, sekcija->trz->prvi);
      zagljavlja = safe_realloc(zagljavlja, broj_zaglavlja);

      if (broj_zapisa == 0) continue;

      zagljavlja[broj_zaglavlja] =
          (Elf32_Shdr){.sh_name = indeks_stringa_rel,
                       .sh_type = SHT_RELA,
                       .sh_flags = SHF_INFO_LINK,
                       .sh_addr = 0,
                       .sh_offset = trenutna_pozicija_sadrzaja,
                       .sh_size = broj_zapisa * sizeof(Elf32_Rela),
                       .sh_link = 0,
                       .sh_info = broj_zaglavlja - 1,
                       .sh_addralign = 0x08,
                       .sh_entsize = sizeof(Elf32_Rela)};
      broj_zaglavlja++;
    }
  }

  MojaStringSekcija *strtab = init_moja_string_sekcija();

  off_t pozicija_simbola = lseek(fd, 0, SEEK_CUR);
  int broj_simbola = 0;
  int poslednji_lokalni_simbol = -1;

  for (Simbol *simbol = dohvati_prvi_simbol(); simbol;
       simbol = simbol->sledeci) {
    if (simbol->tip == STB_LOCAL) {
      poslednji_lokalni_simbol = broj_simbola;
    }
    broj_simbola++;

    Elf32_Sym novi_simbol = {
        .st_name = dodaj_string(strtab, simbol->naziv),
        .st_info =
            ELF32_ST_INFO(simbol->tip, simbol->tip_tvf->dohvati_tip(simbol)),
        .st_other = STV_DEFAULT,
        .st_shndx = simbol->tip_tvf->dohvati_referisanu_sekciju(simbol),
        .st_value = simbol->vrednost,
        .st_size = 0,
    };

    write(fd, &novi_simbol, sizeof(novi_simbol));
  }

  zagljavlja = safe_realloc(zagljavlja, broj_zaglavlja);
  int zagljavlje_simbola = broj_zaglavlja;
  zagljavlja[broj_zaglavlja++] =
      (Elf32_Shdr){.sh_name = dodaj_string(shstr, "symtab"),
                   .sh_type = SHT_SYMTAB,
                   .sh_flags = 0,
                   .sh_addr = 0,
                   .sh_offset = pozicija_simbola,
                   .sh_size = broj_simbola * sizeof(Elf32_Sym),
                   .sh_link = 0,
                   .sh_info = 0,
                   .sh_addralign = 0x08,
                   .sh_entsize = sizeof(Elf32_Sym)};

  zagljavlja = safe_realloc(zagljavlja, broj_zaglavlja);
  int ime_strtab = dodaj_string(shstr, "strtab");
  int indeks_zagljavlja_strtab = broj_zaglavlja;
  off_t pozicija_strtab = lseek(fd, 0, SEEK_CUR);
  write(fd, strtab->slova, strtab->velicina);

  zagljavlja[broj_zaglavlja++] = (Elf32_Shdr){
      .sh_name = ime_strtab,
      .sh_type = SHT_STRTAB,
      .sh_flags = SHF_STRINGS,
      .sh_addr = 0,
      .sh_offset = pozicija_strtab,
      .sh_size = strtab->velicina,
      .sh_link = 0,
      .sh_info = 0,
      .sh_addralign = 0,
      .sh_entsize = 0,
  };

  zagljavlja = safe_realloc(zagljavlja, broj_zaglavlja);
  int ime_shstrtab = dodaj_string(shstr, "shstrtab");
  off_t pozicija_shstrtab = lseek(fd, 0, SEEK_CUR);
  write(fd, shstr->slova, shstr->velicina);

  zagljavlja[broj_zaglavlja++] = (Elf32_Shdr){.sh_name = ime_shstrtab,
                                              .sh_type = SHT_STRTAB,
                                              .sh_flags = SHF_STRINGS,
                                              .sh_addr = 0,
                                              .sh_offset = pozicija_shstrtab,
                                              .sh_size = shstr->velicina,
                                              .sh_link = 0,
                                              .sh_info = 0,
                                              .sh_addralign = 0,
                                              .sh_entsize = 0};

  off_t pozicija_shtab = lseek(fd, 0, SEEK_CUR);

  for (int i = 0; i < broj_zaglavlja; i++) {
    if (zagljavlja[i].sh_type == SHT_RELA) {
      zagljavlja[i].sh_link = zagljavlje_simbola;
    }

    if (zagljavlja[i].sh_type == SHT_SYMTAB) {
      zagljavlja[i].sh_link = indeks_zagljavlja_strtab;
      zagljavlja[i].sh_info = poslednji_lokalni_simbol + 1;
    }

    write(fd, &zagljavlja[i], sizeof(Elf32_Shdr));
  }

  Elf32_Ehdr elf_header = {
      .e_ident = {0x7F, 'E', 'L', 'F', ELFCLASS32, ELFDATA2LSB, EV_CURRENT,
                  ELFOSABI_NONE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                  0x00},
      .e_type = ET_REL,
      .e_machine = EM_NONE,
      .e_version = EV_CURRENT,
      .e_entry = 0,
      .e_phoff = 0,
      .e_shoff = pozicija_shtab,
      .e_flags = 0,
      .e_ehsize = 0x40,
      .e_phentsize = 0,
      .e_phnum = 0,
      .e_shentsize = sizeof(Elf32_Shdr),
      .e_shnum = broj_zaglavlja,
      .e_shstrndx = broj_zaglavlja - 1};

  lseek(fd, 0, SEEK_SET);

  write(fd, &elf_header, sizeof(Elf32_Ehdr));

  close(fd);
  obrisi_moju_string_sekciju(shstr);
  obrisi_moju_string_sekciju(strtab);
  napravi_txt_elf_file(izlazni_fajl, zagljavlja, broj_zaglavlja);
  free(zagljavlja);
}