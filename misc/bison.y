%{
void yyerror (char *s);
int yylex();
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../inc/asembler/direktive.h"
#include "../../inc/asembler/instrukcije.h"
#include "../../inc/asembler/neizracunjivi_simbol.h"

int broj_linije = 0;
Izraz* izraz = NULL;

%}

%union {int num; char* myword;}
%start program
%token <num> broj
%token <myword> rec
%token <myword> recenica
%token <num> registar
%token <num> csr_registar
%type <myword> lista_global_dir lista_extern_dir lista_word_simbola
%type <myword> labela
%token global
%token externa
%token section
%token word
%token ascii
%token equ
%token end
%token skip
%token halt
%token software_int
%token iret
%token ret
%token call
%token jmp
%token beq
%token bne
%token bgt
%token push
%token pop
%token not
%token xchg
%token add
%token sub
%token mul
%token division
%token and
%token or
%token xor
%token shl
%token shr
%token csrrd
%token csrwr
%token ld
%token st

%%

program:  /* empty */
        | program linija {printf("linija %d\n", broj_linije++);}
        ;

linija:   '\n'           {;}
        | direktiva '\n' {;}
        | instrukcija '\n' {provera_prekoracenja_bazena_inst();}
        | labela ':' direktiva '\n' {;}
        | labela ':' instrukcija '\n' {provera_prekoracenja_bazena_inst();}
        | labela  ':' '\n' {;}
        ;

instrukcija:    bezadresna      {;}
        |       uslovni_skok    {;}
        |       bezuslovni_skok {;}
        |       jednoadr_ins    {;}
        |       dvoadr_ins      {;}
        |       ldinst          {;}
        |       stinst          {;}
        ;
bezadresna:     halt            {bezadresna_inst(0);}
        |       software_int    {bezadresna_inst(16);}
        |       iret            {iret_inst();}
        |       ret             {pop_inst(15);}
        ;
bezuslovni_skok:call broj       {skok_literal(32, 0, 0, $2);}
        |       call rec        {skok_simbol(32, 0, 0, $2);}
        |       jmp broj        {skok_literal(48, 0, 0, $2);}
        |       jmp rec         {skok_simbol(48, 0, 0, $2);}
        ;
uslovni_skok:   beq registar ',' registar ',' broj      {skok_literal(0x31, $2, $4, $6);}
        |       beq registar ',' registar ',' rec       {skok_simbol(0x31, $2, $4, $6);}
        |       bne registar ',' registar ',' broj      {skok_literal(0x32, $2, $4, $6);}
        |       bne registar ',' registar ',' rec       {skok_simbol(0x32, $2, $4, $6);}
        |       bgt registar ',' registar ',' broj      {skok_literal(0x33, $2, $4, $6);}
        |       bgt registar ',' registar ',' rec       {skok_simbol(0x33, $2, $4, $6);}
        ;
jednoadr_ins:   push registar   {push_inst($2);}
        |       pop registar    {pop_inst($2);}
        |       not registar    {jednoadr_inst(96, $2);}
        ;
dvoadr_ins:     xchg registar ',' registar      {dvoadr_inst(64, $2, $4);}
        |       add registar ',' registar       {dvoadr_inst(80, $2, $4);}
        |       sub registar ',' registar       {dvoadr_inst(81, $2, $4);}
        |       mul registar ',' registar       {dvoadr_inst(82, $2, $4);}
        |       division registar ',' registar  {dvoadr_inst(83, $2, $4);}
        |       and registar ',' registar       {dvoadr_inst(97, $2, $4);}
        |       or registar ',' registar        {dvoadr_inst(98, $2, $4);}
        |       xor registar ',' registar       {dvoadr_inst(99, $2, $4);}
        |       shl registar ',' registar       {dvoadr_inst(112, $2, $4);}
        |       shr registar ',' registar       {dvoadr_inst(113, $2, $4);}
        |       csrrd csr_registar ',' registar     {csr_inst(0x90, $2, $4);}
        |       csrwr registar ',' csr_registar     {csr_inst(0x94, $2, $4);}
        ;
ldinst:         ld '$' broj ',' registar        {ld_imm_literal($5, $3);}
        |       ld '$' rec ',' registar         {ld_imm_simbol($5, $3);}
        |       ld broj ',' registar            {ld_mem_literal($4, $2);}
        |       ld rec ',' registar             {ld_mem_simbol($4, $2);}
        |       ld registar ',' registar        {dvoadr_inst(0x91, $4, $2);}
        |       ld '[' registar ']' ',' registar        {ld_reg_literal($6, $3, 0);}
        |       ld '[' registar '+' rec ']' ',' registar        {ld_reg_simbol($8, $3, $5);}
        |       ld '[' registar '+' broj ']' ',' registar       {ld_reg_literal($8, $3, $5);}
        ;
stinst:         st registar ',' '$' broj        {printf("st ne moze sa imm\n");}
        |       st registar ',' '$' rec         {printf("st ne moze sa imm\n");}
        |       st registar ',' broj            {st_mem_literal($2, $4);}
        |       st registar ',' rec             {st_mem_simbol($2, $4);}
        |       st registar ',' registar        {dvoadr_inst(0x91, $4, $2);}
        |       st registar ',' '[' registar ']'       {st_reg_literal($2, $5, 0);}
        |       st registar ',' '[' registar '+' rec ']'        {st_reg_simbol($2, $5, $7);}
        |       st registar ',' '[' registar '+' broj ']'       {st_reg_literal($2, $5, $7);}
        ;                    
labela: rec {dodaj_labelu($1);}
        ;
direktiva: global lista_global_dir      {;}
         | externa lista_extern_dir     {;}
         | section rec                  { section_dir($2); }
         | word lista_word_simbola      {;}
         | skip broj                    {skip_dir($2);}
         | ascii recenica               {printf("%s\n", $2);ascii_dir($2);}
         | end                          {end_dir();}
         | equ rec ',' equ_izraz        {equ_dir($2, izraz); izraz = NULL;}
         ;
lista_global_dir:  rec                    {global_dir($1);}
                | lista_global_dir ',' rec  {$$ = $1; global_dir($3);}
                ;
lista_extern_dir: rec                     {extern_dir($1);}
                | lista_extern_dir ',' rec  {$$ = $1; extern_dir($3);}
                ;
lista_word_simbola: rec                   {word_dir_sim($1);}
                | broj                      {word_dir_literal($1);}
                | lista_word_simbola ',' rec  {$$ = $1; word_dir_sim($3);}
                | lista_word_simbola ',' broj {$$ = $1; word_dir_literal($3);}
                ;
equ_izraz:              rec                     {if (izraz == NULL) izraz = init_Izraz();
                                                dodaj_clan(izraz, init_clan_izraza_simbol($1));}
                |       broj                    {if (izraz == NULL) izraz = init_Izraz();
                                                dodaj_clan(izraz, init_clan_izraza_literal($1));}    
                |       equ_izraz '+' rec       {if (izraz == NULL) izraz = init_Izraz();
                                                dodaj_clan(izraz, init_clan_izraza_operator(0));
                                                dodaj_clan(izraz, init_clan_izraza_simbol($3));}
                |       equ_izraz '-' rec       {if (izraz == NULL) izraz = init_Izraz();
                                                dodaj_clan(izraz, init_clan_izraza_operator(1));
                                                dodaj_clan(izraz, init_clan_izraza_simbol($3));}
                |       equ_izraz '*' rec       {if (izraz == NULL) izraz = init_Izraz();
                                                dodaj_clan(izraz, init_clan_izraza_operator(2));
                                                dodaj_clan(izraz, init_clan_izraza_simbol($3));}
                |       equ_izraz '/' rec       {if (izraz == NULL) izraz = init_Izraz();
                                                dodaj_clan(izraz, init_clan_izraza_operator(3));
                                                dodaj_clan(izraz, init_clan_izraza_simbol($3));}
                |       equ_izraz '+' broj      {if (izraz == NULL) izraz = init_Izraz();
                                                dodaj_clan(izraz, init_clan_izraza_operator(0));
                                                dodaj_clan(izraz, init_clan_izraza_literal($3));}
                |       equ_izraz '-' broj      {if (izraz == NULL) izraz = init_Izraz();
                                                dodaj_clan(izraz, init_clan_izraza_operator(1));
                                                dodaj_clan(izraz, init_clan_izraza_literal($3));}
                |       equ_izraz '*' broj      {if (izraz == NULL) izraz = init_Izraz();
                                                dodaj_clan(izraz, init_clan_izraza_operator(2));
                                                dodaj_clan(izraz, init_clan_izraza_literal($3));}
                |       equ_izraz '/' broj      {if (izraz == NULL) izraz = init_Izraz();
                                                dodaj_clan(izraz, init_clan_izraza_operator(3));
                                                dodaj_clan(izraz, init_clan_izraza_literal($3));}
                ;    
%%

void yyerror (char *s) { fprintf (stderr, "%s\n", s); }
