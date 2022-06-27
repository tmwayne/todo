// 
// -----------------------------------------------------------------------------
// config-reader.h
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

#ifndef CONFIG_READER_INCLUDED
#define CONFIG_READER_INCLUDED

#include <stdio.h>  // FILE
#include "dict.h"   // dict_T

// To avoid depending on the generated headers of flex and bison
// we include these typedefs here
typedef void *yyscan_t;
typedef union YYSTYPE YYSTYPE;

extern int    cr_yylex(YYSTYPE *, yyscan_t);
extern void   cr_yyerror(const dict_T, const yyscan_t, const char *msg);
extern dict_T readConfig(dict_T, FILE *file);

#endif // CONFIG_READER_INCLUDED
