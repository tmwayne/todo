// 
// -----------------------------------------------------------------------------
// reader.h
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

#ifndef READER_INCLUDED
#define READER_INCLUDED

#include "parser.h"    // YYSTYPE
#include "dataframe.h" // dataframe_T

struct scannerArgs {
  FILE *yyin;
  char sep;
  int headers;
  int quotes;
};

extern int  yylex(YYSTYPE *, struct scannerArgs);
extern void yyerror(const dataframe_T, const struct scannerArgs, const char *);

extern dataframe_T parseDelim(FILE *file, char sep, int headers, int quotes);


#endif // READER_INCLUDED
