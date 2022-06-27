//
// -----------------------------------------------------------------------------
// config-reader.c
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
//

#include <stdio.h>  // fprintf
#include "parser.h" // yyparse
#include "lexer.h"  // yylex_init, yyset_in, yylex_destroy
#include "dict.h"   // dict_T, dictNew
#include "config-reader.h"

void 
cr_yyerror(const dict_T dict, const yyscan_t scanner, const char *msg)
{
  fprintf(stderr, "Error: %s\n", msg);
}

dict_T
readConfig(dict_T configs, FILE *file)
{
  if (!file) return NULL;
  if (!configs) configs = dictNew();

  yyscan_t scanner;
  cr_yylex_init(&scanner);
  cr_yyset_in(file, scanner);

  if (cr_yyparse(configs, scanner) == -1)
    return NULL;

  cr_yylex_destroy(scanner);

  return configs;
}
