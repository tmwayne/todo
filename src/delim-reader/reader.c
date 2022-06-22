//
// -----------------------------------------------------------------------------
// reader.c
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

#include <stdio.h>           // FILE
#include "parser.h"          // yyparse
#include "delim-reader.h"    // scannerArgs
#include "dataframe.h"       // dataframe_T, dataframeNew

void
yyerror(const dataframe_T data, const struct scannerArgs scanner, const char *msg)
{
  fprintf(stderr, "Error: %s\n", msg);
}

dataframe_T
parseDelim(FILE *file, char sep, int headers, int quotes)
{
  struct scannerArgs scanner = {
    .yyin     = file,
    .sep      = sep,
    .headers  = headers,
    .quotes   = quotes
  };
  
  dataframe_T data = dataframeNew();

  if (yyparse(data, scanner) == -1)
    return NULL;

  return data;
}
