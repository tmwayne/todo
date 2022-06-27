// 
// -----------------------------------------------------------------------------
// dict.h
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

#ifndef DICT_INCLUDED
#define DICT_INCLUDED

enum dictReturnCodes {
  DT_OK = 0,
  DT_EINVALARG = -1, // null pointer passed as argument
  DT_EALLOC = -2     // unable to allocate memory
};

typedef struct dict_T *dict_T;

extern dict_T dictNew();
extern int    dictSet(dict_T, const char *key, const char *val);
extern char  *dictGet(const dict_T, const char *key);
extern void   dictDump(const dict_T);
extern void   dictFree(dict_T *);

#endif // DICT_INCLUDED
