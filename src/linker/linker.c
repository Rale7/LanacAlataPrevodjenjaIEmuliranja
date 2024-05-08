#include <unistd.h>
#include <fcntl.h>
#include <elf.h>
#include <stdlib.h>
#include <stdio.h>
#include "../../inc/linker/linker.h"
#include "../../inc/linker/tabela_sekcija.h"
#include "../../inc/linker/simbol.h"
#include "../../inc/linker/sekcija.h"
#include "../../inc/linker/tabela_simbola.h"
#include "../../inc/linker/relokacioni_zapis.h"
#include "../../inc/linker/moja_string_sekcija.h"

#define PROVERI_ELF_HEADER(elf_header_ident) \
  elf_header_ident[0] != 0x7f || \
  elf_header_ident[1] != 'E' || \
  elf_header_ident[2] != 'L' || \
  elf_header_ident[3] != 'F'

Linker* init_linker() {

  Linker* novi = (Linker*) malloc(sizeof(Linker));
  if (novi == NULL) {
    exit(1);
  }

  novi->tabela_sekcija = init_tabela_sekcija();
  novi->tabela_simbola = init_tabela_simbola();

  ubaci_zadatu_sekciju(novi->tabela_sekcija, novi->tabela_simbola, "UND");

  return novi;

}

void procesiraj_ulazni_fajl(Linker* linker, const char* ime_ulaznog_fajla) {

  int fd = open(ime_ulaznog_fajla, O_RDONLY);
  if (fd < 0) {
    printf("Greska pri otvaranju ulaznog fajla %s\n", ime_ulaznog_fajla);
    return;
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

  char* section_strings = (char*) malloc(section_headers[shstrtab].sh_size * sizeof(char));
  lseek(fd, section_headers[shstrtab].sh_offset, SEEK_SET);
  read(fd, section_strings,  section_headers[shstrtab].sh_size * sizeof(char));

  int indeks_tabele_simbola;
  int indeks_string_simbola;

  for (int i = 0; i < num_of_sections; i++) {
    if (section_headers[i].sh_type == SHT_SYMTAB) {
      indeks_tabele_simbola = i;
      indeks_string_simbola = section_headers[i].sh_link;
    }
    
    if (section_headers[i].sh_type == SHT_PROGBITS) {
      ubaci_zadatu_sekciju(linker->tabela_sekcija, linker->tabela_simbola, &section_strings[section_headers[i].sh_name]);
    }
  }

  int velicina_tabele_simbola = section_headers[indeks_tabele_simbola].sh_size;
  int broj_simbola = velicina_tabele_simbola / section_headers[indeks_tabele_simbola].sh_entsize;
  Elf32_Sym *tabela_simbola = (Elf32_Sym*) malloc(velicina_tabele_simbola);
  int* indeksi = (int*) malloc(broj_simbola * sizeof(int));
  char* simbol_strings = (char*) malloc(section_headers[indeks_string_simbola].sh_size * sizeof(char));

  lseek(fd, section_headers[indeks_tabele_simbola].sh_offset, SEEK_SET);
  read(fd, tabela_simbola, velicina_tabele_simbola);

  lseek(fd, section_headers[indeks_string_simbola].sh_offset, SEEK_SET);
  read(fd, simbol_strings, section_headers[indeks_string_simbola].sh_size);

  for (int i = 0; i < broj_simbola; i++) {
    char* naziv_trenutnog_simbola = &simbol_strings[tabela_simbola[i].st_name];

    if (ELF32_R_TYPE(tabela_simbola[i].st_info) == STT_SECTION) {
      Sekcija* sekcija = dohvati_sekciju(linker->tabela_sekcija, naziv_trenutnog_simbola);
      indeksi[i] = sekcija->simbol.id;
      continue;
    }

    Simbol* novi;

    if (ELF32_ST_BIND(tabela_simbola[i].st_info) == STB_GLOBAL) {

      Simbol* postojeci;
      
      if ((postojeci = proveri_postojanje_globalnog(linker->tabela_simbola, naziv_trenutnog_simbola)) != NULL) {
        
        if (postojeci->tvf->dohvati_sekciju(postojeci) != SHN_UNDEF && tabela_simbola[i].st_shndx != SHN_UNDEF) {
          printf("Greska: Visestruka definicija simbola %s\n", naziv_trenutnog_simbola);
          close(fd);
          exit(1);
        } else if (tabela_simbola[i].st_shndx == SHN_ABS) {
          prebaci_u_simbolicku_konstantu(postojeci, tabela_simbola[i].st_value);
        } else if (tabela_simbola[i].st_shndx != 0) {
          Sekcija* sekcija = dohvati_sekciju(linker->tabela_sekcija, 
          &section_strings[section_headers[tabela_simbola[i].st_shndx].sh_name]);
          prebaci_u_definisan(postojeci, sekcija, tabela_simbola[i].st_value);
        }
        indeksi[i] = postojeci->id;
        continue;
      }

      if (tabela_simbola[i].st_shndx == SHN_ABS) {
        novi = init_simbolicka_konstanta(naziv_trenutnog_simbola, tabela_simbola[i].st_value, GLOBALNI);
      } else if (tabela_simbola[i].st_shndx == SHN_UNDEF) {
        novi = init_nedefinisan_simbol(naziv_trenutnog_simbola); 
      } else {
        Sekcija* sekcija = dohvati_sekciju(linker->tabela_sekcija, 
        &section_strings[section_headers[tabela_simbola[i].st_shndx].sh_name]);
        novi = init_globalni_simbol(naziv_trenutnog_simbola, tabela_simbola[i].st_value, sekcija);
      }
    } else {
        if (tabela_simbola[i].st_shndx == SHN_UNDEF) {
        printf("Greska: Nedefinisan lokalni simbol %s\n", naziv_trenutnog_simbola);
      } else if (tabela_simbola[i].st_shndx == SHN_ABS) {
        novi = init_simbolicka_konstanta(naziv_trenutnog_simbola, tabela_simbola[i].st_value, LOKALNI);
      } else {
        Sekcija* sekcija = dohvati_sekciju(linker->tabela_sekcija, 
        &section_strings[section_headers[tabela_simbola[i].st_shndx].sh_name]);
        novi = init_lokalni_simbol(naziv_trenutnog_simbola, tabela_simbola[i].st_value, sekcija);
      }
    }

    indeksi[i] = ubaci_simbol(linker->tabela_simbola, novi);
  }

  for (int i = 0; i < num_of_sections; i++) {

    if (section_headers[i].sh_type == SHT_PROGBITS) {
      const char* ime_trenutne_sekcije = &section_strings[section_headers[i].sh_name];
      Sekcija* sekcija = dohvati_sekciju(linker->tabela_sekcija, ime_trenutne_sekcije);

      char* mesto_za_sadrzaj = prosiri_sadrzaj(sekcija, section_headers[i].sh_size);

      lseek(fd, section_headers[i].sh_offset, SEEK_SET);
      read(fd, mesto_za_sadrzaj, section_headers[i].sh_size);
    } else if (section_headers[i].sh_type == SHT_RELA) {
      const char* ime_sekcije = &section_strings[section_headers[section_headers[i].sh_info].sh_name];
      Sekcija* sekcija = dohvati_sekciju(linker->tabela_sekcija, ime_sekcije);

      int broj_relokacionih_zapisa = section_headers[i].sh_size / section_headers[i].sh_entsize;
      Elf32_Rela* relokacioni_zapisi = (Elf32_Rela*) malloc(sizeof(Elf32_Rela) * broj_relokacionih_zapisa);

      lseek(fd, section_headers[i].sh_offset, SEEK_SET);
      read(fd, relokacioni_zapisi, broj_relokacionih_zapisa * sizeof(Elf32_Rela));

      for (int i = 0; i < broj_relokacionih_zapisa; i++) {

        int offset = relokacioni_zapisi[i].r_offset + sekcija->velicina;
        Simbol* simbol = dohvati_simbol_id(linker->tabela_simbola, indeksi[ELF32_M_SYM(relokacioni_zapisi[i].r_info)]);

        RelokacioniZapis* rz = simbol->tvf->napravi_relokacioni_zapis(simbol, offset, relokacioni_zapisi[i].r_addend);
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
    printf("%d\t%s\t%d\t%d\t%d\n", 
    simbol->id, simbol->naziv, simbol->tvf->dohvati_sekciju(simbol), simbol->tip, simbol->vrednost);
  }

}

void napravi_izvrsni_fajl(Linker* linker, const char* ime_izlaznog_fajla) {

  sortiraj_tabelu_sekcija(linker->tabela_sekcija);

  Simbol* simbol;
  if ((simbol = provera_postoji_nedefinisan(linker->tabela_simbola)) != NULL) {
    printf("Simbol %s nije definisan\n", simbol->naziv);
    exit(1);
  }

  int fd = open(ime_izlaznog_fajla, O_WRONLY | O_CREAT | O_TRUNC, 0777);

  int broj_segmenata = linker->tabela_sekcija->broj_sekcija;

  Elf32_Ehdr elf_header;
  Elf32_Phdr* segment_headeri = (Elf32_Phdr*) malloc(broj_segmenata * sizeof(Elf32_Phdr));
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

    poslednja_va = trenutna_sekcija->virtuelna_adresa + trenutna_sekcija->velicina;
  }

  for (int i = 0; i < linker->tabela_sekcija->broj_sekcija; i++) {
    Sekcija* trenutna_sekcija = linker->tabela_sekcija->sekcije[i];

    razresi_relokacije(trenutna_sekcija);

    off_t offset = lseek(fd, 0, SEEK_CUR);

    segment_headeri[i] = (Elf32_Phdr) {
      .p_type = PT_LOAD,
      .p_flags = 0,
      .p_offset = offset,
      .p_vaddr = trenutna_sekcija->virtuelna_adresa,
      .p_paddr = 0,
      .p_filesz = trenutna_sekcija->velicina,
      .p_memsz = trenutna_sekcija->velicina,
      .p_align = 0x04
    };
    ispisi_sadrzaj(trenutna_sekcija);
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

  elf_header = (Elf32_Ehdr) {
    .e_ident = {
      0x7F, 'E', 'L', 'F',
      ELFCLASS32,
      ELFDATA2LSB,
      EV_CURRENT,
      ELFOSABI_NONE,
      0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00
    },
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

