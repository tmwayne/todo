//
// -----------------------------------------------------------------------------
// parser.y
// -----------------------------------------------------------------------------
//
// Copyright (c) 2022 Tyler Wayne
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// stackoverflow.com/questions/48850242/thread-safe-reentrant-bison-flex
//

%{
#include <stdio.h>  // printf
#include <stdlib.h> // calloc
#include <string.h> // strncpy
#include "parser.h"
#include "delim-reader.h"
#include "dataframe.h"

record_T record;
char *val;

%}

%define parse.trace
%define api.pure full 
// %locations

%code requires {
  typedef struct dataframe *dataframe_T;
  typedef struct delimArgs scanner_T;
}

%parse-param{ dataframe_T data }
%param{ scanner_T scanner }

%union {
  char *str;
  char c;
}

%token <str> FIELD
%token <c> SEP EOL

%type <str> field

%%

input:
  record                { if (scanner.headers) dataframeSetHeaders(data, record);
                          else                 dataframePush(data, record);
                          record = NULL; }

  | input EOL record    { if(dataframePush(data, record) != DF_OK) YYERROR;
                          record = NULL; }
  ;

record: 
  /* blank line */
  | field               { if (!record) record = recordNew();
                          recordPush(record, $1); }

  | record SEP field    { recordPush(record, $3); }
  ;

field:
  /* empty field */     { $$ = ""; }
  | FIELD               { $$ = $1; }
  ;

%%
