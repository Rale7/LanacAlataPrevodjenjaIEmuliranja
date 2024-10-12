#include "linker/linker.h"

#include <elf.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "linker/moja_string_sekcija.h"
#include "linker/parsiranje_cmd.h"
#include "linker/relokacioni_zapis.h"
#include "linker/sekcija.h"
#include "linker/simbol.h"
#include "linker/tabela_sekcija.h"
#include "linker/tabela_simbola.h"

#define PROVERI_ELF_HEADER(elf_header_ident)                   \
  elf_header_ident[0] != 0x7f || elf_header_ident[1] != 'E' || \
      elf_header_ident[2] != 'L' || elf_header_ident[3] != 'F'

static inline Elf32_Shdr* safe_realloc(Elf32_Shdr* shrd, int broj_zagljavlja) {
  if (broj_zagljavlja > 0 && broj_zagljavlja % 10 == 0) {
    shrd =
        (Elf32_Shdr*)realloc(shrd, (broj_zagljavlja + 10) * sizeof(Elf32_Shdr));
    if (shrd == NULL) {
      exit(1);
    }
  }

  return shrd;
}

int upisi_relokacione_zapise(int fd, RelokacioniZapis* rel) {
  int cnt = 0;

  for (; rel; rel = rel->sledeci) {
    int simbol_rel = rel->simbol->id;
    if (simbol_rel != -1) {
      cnt++;

      Elf32_Rela relokacioni_zapis = {.r_offset = rel->offset,
                                      .r_info = ELF32_R_INFO(simbol_rel, 0),
                                      .r_addend = rel->addend};

      write(fd, &relokacioni_zapis, sizeof(Elf32_Rela));
    }
  }

  return cnt;
}

Linker* init_linker() {
  Linker* novi = (Linker*)malloc(sizeof(Linker));
  if (novi == NULL) {
    exit(1);
  }

  novi->tabela_sekcija = init_tabela_sekcija();
  novi->tabela_simbola = init_tabela_simbola();
  novi->prvi = NULL;
  novi->indirect = &novi->prvi;

  ubaci_zadatu_sekciju(novi->tabela_sekcija, novi->tabela_simbola, "UND");

  return novi;
}

void procesiraj_ulazni_fajl(Linker* linker, const char* ime_ulaznog_fajla) {
  int fd = open(ime_ulaznog_fajla, O_RDONLY);
  if (fd < 0) {
    printf("Greska pri otvaranju ulaznog fajla %s\n", ime_ulaznog_fajla);
    exit(1);
  }

  Elf32_Ehdr elf_header;

  read(fd, &elf_header, sizeof(Elf32_Ehdr));

  if (PROVERI_ELF_HEADER(elf_header.e_ident)) {
    printf("Zadati fajl %s nije u ELF formatu\n", ime_ulaznog_fajla);
    close(fd);
    exit(1);
  }

  int shstrtab = elf_header.e_shstrndx;
  int num_of_sections = elf_header.e_shnum;
  lseek(fd, elf_header.e_shoff, SEEK_SET);
  Elf32_Shdr section_headers[num_of_sections];

  read(fd, &section_headers, elf_header.e_shentsize * num_of_sections);

  char* section_strings =
      (char*)malloc(section_headers[shstrtab].sh_size * sizeof(char));
  lseek(fd, section_headers[shstrtab].sh_offset, SEEK_SET);
  read(fd, section_strings, section_headers[shstrtab].sh_size * sizeof(char));

  int indeks_tabele_simbola;
  int indeks_string_simbola;

  for (int i = 0; i < num_of_sections; i++) {
    if (section_headers[i].sh_type == SHT_SYMTAB) {
      indeks_tabele_simbola = i;
      indeks_string_simbola = section_headers[i].sh_link;
    }

    if (section_headers[i].sh_type == SHT_PROGBITS) {
      ubaci_zadatu_sekciju(linker->tabela_sekcija, linker->tabela_simbola,
                           &section_strings[section_headers[i].sh_name]);
    }
  }

  int velicina_tabele_simbola = section_headers[indeks_tabele_simbola].sh_size;
  int broj_simbola = velicina_tabele_simbola /
                     section_headers[indeks_tabele_simbola].sh_entsize;
  Elf32_Sym* tabela_simbola = (Elf32_Sym*)malloc(velicina_tabele_simbola);
  int* indeksi = (int*)malloc(broj_simbola * sizeof(int));
  char* simbol_strings = (char*)malloc(
      section_headers[indeks_string_simbola].sh_size * sizeof(char));

  lseek(fd, section_headers[indeks_tabele_simbola].sh_offset, SEEK_SET);
  read(fd, tabela_simbola, velicina_tabele_simbola);

  lseek(fd, section_headers[indeks_string_simbola].sh_offset, SEEK_SET);
  read(fd, simbol_strings, section_headers[indeks_string_simbola].sh_size);

  for (int i = 0; i < broj_simbola; i++) {
    char* naziv_trenutnog_simbola = &simbol_strings[tabela_simbola[i].st_name];

    if (ELF32_R_TYPE(tabela_simbola[i].st_info) == STT_SECTION) {
      Sekcija* sekcija =
          dohvati_sekciju(linker->tabela_sekcija, naziv_trenutnog_simbola);
      indeksi[i] = sekcija->simbol.id;
      continue;
    }

    Simbol* novi;

    if (ELF32_ST_BIND(tabela_simbola[i].st_info) == STB_GLOBAL) {
      Simbol* postojeci;

      if ((postojeci = proveri_postojanje_globalnog(
               linker->tabela_simbola, naziv_trenutnog_simbola)) != NULL) {
        if (postojeci->tvf->dohvati_sekciju(postojeci) != SHN_UNDEF &&
            tabela_simbola[i].st_shndx != SHN_UNDEF) {
          printf("Greska: Visestruka definicija simbola %s\n",
                 naziv_trenutnog_simbola);
          close(fd);
          exit(1);
        } else if (tabela_simbola[i].st_shndx == SHN_ABS) {
          prebaci_u_simbolicku_konstantu(postojeci, tabela_simbola[i].st_value);
        } else if (tabela_simbola[i].st_shndx != 0) {
          Sekcija* sekcija = dohvati_sekciju(
              linker->tabela_sekcija,
              &section_strings[section_headers[tabela_simbola[i].st_shndx]
                                   .sh_name]);
          prebaci_u_definisan(postojeci, sekcija, tabela_simbola[i].st_value);
        }
        indeksi[i] = postojeci->id;
        continue;
      }

      if (tabela_simbola[i].st_shndx == SHN_ABS) {
        novi = init_simbolicka_konstanta(naziv_trenutnog_simbola,
                                         tabela_simbola[i].st_value, GLOBALNI);
      } else if (tabela_simbola[i].st_shndx == SHN_UNDEF) {
        novi = init_nedefinisan_simbol(naziv_trenutnog_simbola);
      } else {
        Sekcija* sekcija = dohvati_sekciju(
            linker->tabela_sekcija,
            &section_strings[section_headers[tabela_simbola[i].st_shndx]
                                 .sh_name]);
        novi = init_globalni_simbol(naziv_trenutnog_simbola,
                                    tabela_simbola[i].st_value, sekcija);
      }
    } else {
      if (tabela_simbola[i].st_shndx == SHN_ABS) {
        novi = init_simbolicka_konstanta(naziv_trenutnog_simbola,
                                         tabela_simbola[i].st_value, LOKALNI);
      } else {
        Sekcija* sekcija = dohvati_sekciju(
            linker->tabela_sekcija,
            &section_strings[section_headers[tabela_simbola[i].st_shndx]
                                 .sh_name]);
        novi = init_lokalni_simbol(naziv_trenutnog_simbola,
                                   tabela_simbola[i].st_value, sekcija);
      }
    }

    indeksi[i] = ubaci_simbol(linker->tabela_simbola, novi);
  }

  for (int i = 0; i < num_of_sections; i++) {
    if (section_headers[i].sh_type == SHT_PROGBITS) {
      const char* ime_trenutne_sekcije =
          &section_strings[section_headers[i].sh_name];
      Sekcija* sekcija =
          dohvati_sekciju(linker->tabela_sekcija, ime_trenutne_sekcije);

      char* mesto_za_sadrzaj =
          prosiri_sadrzaj(sekcija, section_headers[i].sh_size);

      lseek(fd, section_headers[i].sh_offset, SEEK_SET);
      read(fd, mesto_za_sadrzaj, section_headers[i].sh_size);
    } else if (section_headers[i].sh_type == SHT_RELA) {
      const char* ime_sekcije =
          &section_strings[section_headers[section_headers[i].sh_info].sh_name];
      Sekcija* sekcija = dohvati_sekciju(linker->tabela_sekcija, ime_sekcije);

      int broj_relokacionih_zapisa =
          section_headers[i].sh_size / section_headers[i].sh_entsize;
      Elf32_Rela* relokacioni_zapisi =
          (Elf32_Rela*)malloc(sizeof(Elf32_Rela) * broj_relokacionih_zapisa);

      lseek(fd, section_headers[i].sh_offset, SEEK_SET);
      read(fd, relokacioni_zapisi,
           broj_relokacionih_zapisa * sizeof(Elf32_Rela));

      for (int i = 0; i < broj_relokacionih_zapisa; i++) {
        int offset = relokacioni_zapisi[i].r_offset + sekcija->velicina;
        Simbol* simbol = dohvati_simbol_id(
            linker->tabela_simbola,
            indeksi[ELF32_M_SYM(relokacioni_zapisi[i].r_info)]);

        RelokacioniZapis* rz = simbol->tvf->napravi_relokacioni_zapis(
            simbol, offset, relokacioni_zapisi[i].r_addend);
        dodaj_relokacioni_zapis(sekcija, rz);
      }

      free(relokacioni_zapisi);
    }
  }

  for (int i = 0; i < num_of_sections; i++) {
    if (section_headers[i].sh_type == SHT_PROGBITS) {
      const char* ime_sekcije = &section_strings[section_headers[i].sh_name];
      Sekcija* sekcija = dohvati_sekciju(linker->tabela_sekcija, ime_sekcije);
      sekcija->velicina += section_headers[i].sh_size;
    }
  }

  free(indeksi);
  free(section_strings);
  free(tabela_simbola);
  free(simbol_strings);

  close(fd);
}

void ispisi_strukture(Linker* linker) {
  for (int i = 0; i < linker->tabela_simbola->trenutni_id; i++) {
    Simbol* simbol = linker->tabela_simbola->simboli[i];
    printf("%d\t%s\t%d\t%d\t%d\n", simbol->id, simbol->naziv,
           simbol->tvf->dohvati_sekciju(simbol), simbol->tip, simbol->vrednost);
  }
}

void razresi_virtuelne_adrese(Linker* linker) {
  for (CmdSekcija* tekuci = linker->prvi; tekuci; tekuci = tekuci->sledeci) {
    Sekcija* sekcija = dohvati_sekciju(linker->tabela_sekcija, tekuci->ime);
    if (sekcija == NULL) {
      printf("Greska pri zadavanju argumenata, sekcija %s ne postoji\n",
             tekuci->ime);
    }

    sekcija->virtuelna_adresa = tekuci->va;
  }
}
char* int_to_string(int broj) {
  char* string = (char*)malloc(sizeof(char) * 5);

  sprintf(string, "%d", broj);

  return string;
}

void readelf_program(const char* txt_fajl, const char* ulazni_fajl,
                     Elf32_Shdr* zaglavlja, int broj_zaglavlja) {
  int broj_sekcija = 0;

  for (int i = 0; i < broj_zaglavlja; i++) {
    if (zaglavlja[i].sh_type == SHT_PROGBITS) {
      broj_sekcija++;
    }
  }

  char** argv = (char**)malloc(sizeof(char*) * (4 + 2 * broj_sekcija));

  argv[0] = "readelf";
  argv[1] = (char*)ulazni_fajl;
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

char* promeni_ektenziju_txt(const char* ulazni_fajl) {
  char* pozicija_tacke = strchr(ulazni_fajl, '.');
  int n = (pozicija_tacke ? (pozicija_tacke - ulazni_fajl)
                          : strlen(ulazni_fajl) - 1);

  char* txt_fajl = (char*)malloc(sizeof(char) * (n + 5));

  strncpy(txt_fajl, ulazni_fajl, n);
  txt_fajl[n] = '.';
  txt_fajl[n + 1] = 't';
  txt_fajl[n + 2] = 'x';
  txt_fajl[n + 3] = 't';
  txt_fajl[n + 4] = '\0';

  return txt_fajl;
}
void napravi_txt_elf_file(const char* ulazni_fajl, Elf32_Shdr* zaglavlja,
                          int broj_zaglavlja) {
  char* txt_fajl = promeni_ektenziju_txt(ulazni_fajl);

  int pid = fork();

  if (pid < 0) {
    perror("Greska u fork-u\n");
    exit(1);
  } else if (pid == 0) {
    readelf_program(txt_fajl, ulazni_fajl, zaglavlja, broj_zaglavlja);
  }

  free(txt_fajl);
}

void napravi_relokativni_fajl(Linker* linker, const char* ime_izlaznog_fajla) {
  int fd = open(ime_izlaznog_fajla, O_WRONLY | O_CREAT | O_TRUNC, 0777);

  lseek(fd, sizeof(Elf32_Ehdr), SEEK_SET);
  MojaStringSekcija* shstr = init_moja_string_sekcija();

  int broj_zaglavlja = 0;
  Elf32_Shdr* zagljavlja = calloc(10, sizeof(Elf32_Shdr));
  if (zagljavlja == NULL) {
    exit(1);
  }

  zagljavlja[broj_zaglavlja++] =
      (Elf32_Shdr){.sh_name = dodaj_string(shstr, "UND"),
                   .sh_type = SHT_NULL,
                   .sh_flags = 0,
                   .sh_addr = 0,
                   .sh_offset = 0,
                   .sh_size = 0,
                   .sh_link = SHN_UNDEF,
                   .sh_info = 0,
                   .sh_addralign = 0,
                   .sh_entsize = 0};

  for (int i = 0; i < linker->tabela_sekcija->broj_sekcija; i++) {
    zagljavlja = safe_realloc(zagljavlja, broj_zaglavlja);

    Sekcija* sekcija = linker->tabela_sekcija->sekcije[i];

    off_t trenutna_pozicija_sadrzaja = lseek(fd, 0, SEEK_CUR);
    write(fd, sekcija->sadrzaj, sekcija->velicina);

    int indeks_stringa_sekcija;
    int indeks_stringa_rel;

    if (sekcija->prvi) {
      indeks_stringa_rel =
          dodaj_string_povezano(shstr, "rel", sekcija->simbol.naziv);
      indeks_stringa_sekcija = indeks_stringa_rel + 4;
    } else {
      indeks_stringa_sekcija = dodaj_string(shstr, sekcija->simbol.naziv);
    }

    sekcija->broj_elf_ulaza = broj_zaglavlja;
    zagljavlja[broj_zaglavlja++] =
        (Elf32_Shdr){.sh_name = indeks_stringa_sekcija,
                     .sh_type = SHT_PROGBITS,
                     .sh_flags = SHF_ALLOC,
                     .sh_addr = 0,
                     .sh_offset = trenutna_pozicija_sadrzaja,
                     .sh_size = sekcija->velicina,
                     .sh_link = 0,
                     .sh_info = 0,
                     .sh_addralign = 0x04,
                     .sh_entsize = 0};

    if (sekcija->prvi) {
      trenutna_pozicija_sadrzaja = lseek(fd, 0, SEEK_CUR);
      int broj_zapisa = upisi_relokacione_zapise(fd, sekcija->prvi);
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

  MojaStringSekcija* strtab = init_moja_string_sekcija();
  off_t pozicija_simbola = lseek(fd, 0, SEEK_CUR);
  int broj_simbola = 0;
  int poslednji_lokalni_simbol = -1;

  for (int i = 0; i < linker->tabela_simbola->trenutni_id; i++) {
    Simbol* simbol = linker->tabela_simbola->simboli[i];

    if (simbol->tip == STB_LOCAL) {
      poslednji_lokalni_simbol = broj_simbola;
    }
    broj_simbola++;

    Elf32_Sym novi_simbol = {
        .st_name = dodaj_string(strtab, simbol->naziv),
        .st_info = ELF32_ST_INFO(simbol->tip, simbol->tvf->dohvati_tip(simbol)),
        .st_other = STV_DEFAULT,
        .st_shndx = (simbol->sekcija ? simbol->sekcija->broj_elf_ulaza
                                     : simbol->tvf->dohvati_sekciju(simbol)),
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

  napravi_txt_elf_file(ime_izlaznog_fajla, zagljavlja, broj_zaglavlja);
  free(zagljavlja);
}

char* promeni_ektenziju(const char* ulazni_fajl) {
  char* pozicija_tacke = strchr(ulazni_fajl, '.');
  int n =
      (pozicija_tacke ? (pozicija_tacke - ulazni_fajl) : strlen(ulazni_fajl));

  char* hex_fajl = (char*)malloc(sizeof(char) * (n + 5));

  strncpy(hex_fajl, ulazni_fajl, n);
  hex_fajl[n] = '.';
  hex_fajl[n + 1] = 'h';
  hex_fajl[n + 2] = 'e';
  hex_fajl[n + 3] = 'x';
  hex_fajl[n + 4] = '\0';

  return hex_fajl;
}

void napravi_izvrsni_fajl(Linker* linker, const char* ime_izlaznog_fajla) {
  razresi_virtuelne_adrese(linker);

  sortiraj_tabelu_sekcija(linker->tabela_sekcija);

  Simbol* simbol;
  if ((simbol = provera_postoji_nedefinisan(linker->tabela_simbola)) != NULL) {
    printf("Simbol %s nije definisan\n", simbol->naziv);
    exit(1);
  }

  int fd = open(ime_izlaznog_fajla, O_WRONLY | O_CREAT | O_TRUNC, 0777);
  FILE* hex_izlaz = fopen(promeni_ektenziju(ime_izlaznog_fajla), "w");

  int broj_segmenata = linker->tabela_sekcija->broj_sekcija;

  Elf32_Ehdr elf_header;
  Elf32_Phdr* segment_headeri =
      (Elf32_Phdr*)malloc(broj_segmenata * sizeof(Elf32_Phdr));
  if (segment_headeri == NULL) {
    close(fd);
    exit(1);
  }

  write(fd, &elf_header, sizeof(elf_header));
  write(fd, segment_headeri, sizeof(Elf32_Phdr) * broj_segmenata);

  int poslednja_va = 0;

  for (int i = 0; i < linker->tabela_sekcija->broj_sekcija; i++) {
    Sekcija* trenutna_sekcija = linker->tabela_sekcija->sekcije[i];

    if (trenutna_sekcija->virtuelna_adresa < poslednja_va) {
      trenutna_sekcija->virtuelna_adresa = poslednja_va;
    }

    poslednja_va =
        trenutna_sekcija->virtuelna_adresa + trenutna_sekcija->velicina;
  }

  for (int i = 0; i < linker->tabela_sekcija->broj_sekcija; i++) {
    Sekcija* trenutna_sekcija = linker->tabela_sekcija->sekcije[i];

    razresi_relokacije(trenutna_sekcija);

    off_t offset = lseek(fd, 0, SEEK_CUR);

    segment_headeri[i] =
        (Elf32_Phdr){.p_type = PT_LOAD,
                     .p_flags = 0,
                     .p_offset = offset,
                     .p_vaddr = trenutna_sekcija->virtuelna_adresa,
                     .p_paddr = 0,
                     .p_filesz = trenutna_sekcija->velicina,
                     .p_memsz = trenutna_sekcija->velicina,
                     .p_align = 0x04};
    ispisi_sadrzaj(trenutna_sekcija, hex_izlaz);
    write(fd, trenutna_sekcija->sadrzaj, trenutna_sekcija->velicina);
  }

  lseek(fd, sizeof(Elf32_Ehdr), SEEK_SET);

  for (int i = 0; i < linker->tabela_sekcija->broj_sekcija; i++) {
    write(fd, &segment_headeri[i], sizeof(Elf32_Phdr));
  }

  off_t kraj_segmenata = lseek(fd, 0, SEEK_CUR);
  lseek(fd, sizeof(Elf32_Ehdr), SEEK_SET);

  for (int i = 0; i < broj_segmenata; i++) {
    write(fd, &segment_headeri[i], sizeof(Elf32_Phdr));
  }

  lseek(fd, kraj_segmenata, SEEK_SET);

  elf_header = (Elf32_Ehdr){
      .e_ident = {0x7F, 'E', 'L', 'F', ELFCLASS32, ELFDATA2LSB, EV_CURRENT,
                  ELFOSABI_NONE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                  0x00},
      .e_type = ET_EXEC,
      .e_machine = EM_NONE,
      .e_version = EV_CURRENT,
      .e_entry = MY_ENTRY,
      .e_phoff = sizeof(Elf32_Ehdr),
      .e_shoff = 0,
      .e_flags = 0,
      .e_ehsize = sizeof(Elf32_Ehdr),
      .e_phentsize = sizeof(Elf32_Phdr),
      .e_phnum = linker->tabela_sekcija->broj_sekcija,
      .e_shentsize = 0,
      .e_shnum = 0,
      .e_shstrndx = 0,
  };

  lseek(fd, 0, SEEK_SET);

  write(fd, &elf_header, sizeof(Elf32_Ehdr));

  free(segment_headeri);
  close(fd);
}

void obrisi_linker(Linker* linker) {
  while (linker->prvi) {
    CmdSekcija* stari = linker->prvi;
    linker->prvi = linker->prvi->sledeci;
    free(stari);
  }

  obrisi_tabelu_simbol(linker->tabela_simbola);
  obrisi_tabelu_sekcija(linker->tabela_sekcija);
}
