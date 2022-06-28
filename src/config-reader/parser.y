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

%{
#include <stdio.h>
#include "parser.h"
#include "config-reader.h"
#include "dict.h"

%}

%define parse.trace
%define api.pure full

%code requires {
  typedef struct dict_T *dict_T;
  typedef void *yyscan_T;
}

%parse-param{ dict_T configs }
%param{ yyscan_T scanner }

%union {
  char *str;
}

%token <str> NAME STRING
%token EOL

%type <s> line

%%

input:
  /* nothing */
  | input EOL
  | input line EOL
  | input error             { yyerrok; }
  ;

line: 
    NAME '=' NAME           { dictSet(configs, $1, $3); }
  | NAME '=' STRING         { dictSet(configs, $1, $3); }
  ; 

%%
