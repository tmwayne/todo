//
// -----------------------------------------------------------------------------
// dict.c
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

#include <stdio.h>  // printf
#include <stdlib.h> // calloc, realloc, free
#include <string.h> // strdup, strcmp
#include "dict.h"

// TODO: change dict to accept other data types
struct elem_T {
  char *key;
  char *val;
  struct elem_T *link;
};

typedef struct dict_T {
  struct elem_T *head;
} *dict_T;

static void
elemFree(struct elem_T **elem)
{
  if (!(elem && *elem)) return;

  // key and val should never be NULL but we guard against it
  if ((*elem)->key) free((*elem)->key);
  if ((*elem)->val) free((*elem)->val);

  free(*elem);
  *elem = NULL;
}

dict_T
dictNew()
{
  dict_T dict;
  dict = calloc(1, sizeof(*dict));
  return dict;
}

static int
dictAddElem(dict_T dict, const char *key, const char *val)
{
  struct elem_T *elem;
  elem = calloc(1, sizeof(*elem));
  if (!elem) return DT_EALLOC;

  elem->key = strdup(key);
  elem->val = strdup(val);
  elem->link = dict->head;
  dict->head = elem;

  return DT_OK;
}

// TODO: return error if memory can't be allocated
int
dictSet(dict_T dict, const char *key, const char *val)
{
  if (!(dict && key)) return DT_EINVALARG;
  if (!val) val = "";

  struct elem_T *elem = dict->head;

  for ( ; elem ; elem = elem->link) {
    if (strcmp(elem->key, key) == 0) {
      free(elem->val);
      elem->val = strdup(val);
      return DT_OK;
    }
  }

  return dictAddElem(dict, key, val);
}

char *
dictGet(const dict_T dict, const char *key)
{
  if (!(dict && key)) return NULL;

  struct elem_T *elem = dict->head;

  for ( ; elem ; elem = elem->link )
    if (strcmp(elem->key, key) == 0)
      return elem->val;

  return NULL;
}

void
dictFree(dict_T *dict)
{
  if (!(dict && dict)) return;

  struct elem_T *elem = (*dict)->head, *next;
  for ( ; elem ; elem = next ) {
    next = elem->link;
    elemFree(&elem);
    elem = next;
  }

  free(*dict);
  *dict = NULL;
}

void
dictDump(const dict_T dict)
{
  if (!dict) return;

  struct elem_T *elem = dict->head;

  if (!elem) printf("Dict is empty\n");

  for ( ; elem ; elem = elem->link )
    printf("%s -> %s\n", elem->key, elem->val);
}
