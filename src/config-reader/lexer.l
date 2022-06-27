/*
 * -----------------------------------------------------------------------------
 * lexer.l
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2022 Tyler Wayne
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

%option noinput nounput noyywrap yylineno nodefault
%option reentrant bison-bridge

%option prefix="cr_yy"
%option outfile="lexer.c"
%option header-file="lexer.h"

%{

#include <stdlib.h> // malloc
#include <string.h> // strdup, memcpy
#include "config-reader.h"
#include "parser.h"

char *duplicate_segment(const char *, int);

%}

%%

[a-zA-Z][_0-9a-zA-Z]*   { yylval_param->str = strdup(yytext);
                          return NAME; }

\"(\\.|[^\\\"])*\"      { yylval_param->str = duplicate_segment(yytext+1, yyleng-2);
                          return STRING; }

=                       { return yytext[0]; }

\n                      { return EOL; }

"#".*                   /* ignore comments */
[ \t]                   /* ignore whitespace */

%%

char *duplicate_segment(const char *token, const int len) {
  // TODO: handle memory allocation error
  char *buf = malloc(len + 1);
  memcpy(buf, token, len);
  buf[len] = 0;
  return buf;
}
