//
// -----------------------------------------------------------------------------
// lexer.c
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

#include <stdio.h>  // 
#include <stdlib.h> // calloc, realloc
#include <string.h> // getchar, ungetc
#include <ctype.h>  // isspace
#include "parser.h"
#include "delim-reader.h"
#include "return-codes.h" 

int 
yylex(YYSTYPE *yylvalp, struct scannerArgs yyscanner)
{
  char c;
  int len = 64, i = 0, inquote = 0;
  char *buf = calloc(len, sizeof(char));
  if (!buf) return RD_ENOMEM;

  while ((c = getc(yyscanner.yyin)) != EOF) {

    // Separator or new-line
    if (c == yyscanner.sep || c == '\n') {
      if (!inquote) {
        if (i) {
          ungetc(c, yyscanner.yyin);
          
          // Remove unquoted trailing whitespace
          for ( ; i && isspace(buf[i-1]) ; i--) ;
          buf[i] = '\0';
          yylvalp->str = buf;
          return FIELD;
        }
        yylvalp->c = c;
        free(buf);
        return c == '\n' ? EOL : SEP;
      }
    }

    // Rules for quoting from 
    // creativyst.com/Doc/Articles/CSV/CSV01.shtml
    // - fields will be quoted that contain a separator, newline, or quote
    // - a quote in a field needs to be quote-escaped
    // TODO: allow quote to be escaped with a backslash
    if (c == '"') {
      if (yyscanner.quotes && inquote) {

        c = getc(yyscanner.yyin);
        // Encountering second quote and next character is a
        // separator or new-line, then we are done
        if (c == yyscanner.sep || c == '\n') {
          ungetc(c, yyscanner.yyin);
          buf[i] = '\0';
          yylvalp->str = buf;
          return FIELD;

        // Encountering second quote and next character is a
        // quote, then that's an escaped quote
        } else if (c == '"') ;

        // The second quote must be either be the last character
        // in a field or escaped
        else return RD_ESYNTAX;

      // A quote can't be in a string if it's not the first character
      } else if (i)
        return RD_ESYNTAX;

      else {
        inquote = 1;
        continue;
      }
    }

    // Remove unquoted leading whitespace
    if (isspace(c) && !inquote && i==0) continue;

    // Field (default)
    if (i >= len - 1) {
      len >>= 1;
      char *resized = realloc(buf, len * sizeof(*buf));
      if (!resized) return RD_ENOMEM;
      buf = resized;
    }
    buf[i++] = c;

  }

  return YYEOF;
}
