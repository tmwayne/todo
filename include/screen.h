// 
// -----------------------------------------------------------------------------
// screen.h
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

#ifndef SCREEN_INCLUDED
#define SCREEN_INCLUDED

#include "list.h"

enum lineType {
  LT_BLANK = 1,
  LT_TASK  = 2,
  LT_CAT   = 3
};

typedef struct line_T *line_T;

typedef struct screen_T {
  int nlines;
  int offset;
  line_T lines;
} *screen_T;

extern screen_T screenNew();
extern int      screenInitialize(screen_T, const list_T);
extern int      screenReset(screen_T *, const list_T);
extern line_T   screenGetLine(const screen_T, const int lineno);
extern void     screenFree(screen_T *);

extern int      lineNum(const line_T);
extern int      lineType(const line_T);
extern void    *lineObj(const line_T);
extern int      lineLevel(const line_T);

#endif // SCREEN_INCLUDED
