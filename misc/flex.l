%{
#include "bison.h"
%}

%%

".global"                   {return global;}
".extern"                   {return externa;}
".section"                  {return section;}
".word"                     {return word;}
".ascii"                    {return ascii;}
".end"                      {return end;}
".skip"                     {return skip;}
"halt"                      {return halt;}
"int"                       {return software_int;}
"iret"                      {return iret;}
"ret"                       {return ret;}
"call"                      {return call;}
"jmp"                       {return jmp;}
"beq"                       {return beq;}
"bne"                       {return bne;}
"bgt"                       {return bgt;}
"push"                      {return push;}
"pop"                       {return pop;}
"not"                       {return not;}
"xchg"                      {return xchg;}
"add"                       {return add;}
"sub"                       {return sub;}
"mul"                       {return mul;}
"div"                       {return division;}
"and"                       {return and;}
"or"                        {return or;}
"xor"                       {return xor;}
"shl"                       {return shl;}
"shr"                       {return shr;}
"csrrd"                     {return csrrd;}
"csrwr"                     {return csrwr;}
"ld"                        {return ld;}
"st"                        {return st;}
%r[0-9]+                    {yylval.num = atoi(yytext + 2); return registar;}
%sp                         {yylval.num = 14; return registar;}
%pc                         {yylval.num = 15; return registar;}
"%handler"                  {yylval.num = 0; return csr_registar;}
"%status"                   {yylval.num = 1; return csr_registar;}
"%cause"                    {yylval.num = 2; return csr_registar;}
[a-zA-Z_][a-zA-Z0-9_]*      { 
                              yylval.myword = strdup(yytext);
                              return rec;
                            }
0x[0-9ABCDEF]+              {yylval.num = strtol(yytext + 2, NULL, 16); return broj;}
0b[0-9]+                    {yylval.num = strtol(yytext + 2, NULL, 2); return broj;}
0[0-9]+                     {yylval.num = strtol(yytext + 1, NULL, 8); return broj;}
[0-8]+                      {yylval.num = atoi(yytext); return broj;}
[ \t]                       ;
#.*                          ;
[+\n,":$\[\]]  	          {return yytext[0];}
                          

%%

int yywrap(void) {return 1;}