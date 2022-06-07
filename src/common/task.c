//
// -----------------------------------------------------------------------------
// task.c
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

#include <stdio.h>       // sscanf
#include <stdlib.h>      // calloc, free
#include <string.h>      // strdup
#include "mem.h"         // memCalloc, memFree
#include "task.h"
#include "error-codes.h" // TD_OK

typedef struct elem_T {
  char *key;
  char *val;
  struct elem_T *link;
} *elem_T;

struct task_T {
  elem_T head;
};

task_T 
taskNew() 
{
  task_T task;
  task = memCalloc(1, sizeof(*task));
  task->head = NULL;
  return task;
}

int 
taskSize(task_T task) 
{
  if (!task) return -1;

  elem_T elem;
  int size = 0;
  for (elem=task->head; elem; elem=elem->link) size++;

  return size;
}

void 
taskSet(task_T task, char *key, char *val) 
{
  if (!(task && key && val)) return;
  if (*key == '\0') return;

  elem_T elem;

  for (elem=task->head; elem; elem=elem->link) {
    if (strcmp(elem->key, key) == 0) {
      elem->val = val;
      return;
    }
  }

  elem = memCalloc(1, sizeof(*elem));
  elem->key = strdup(key);
  elem->val = strdup(val);
  elem->link = task->head;
  task->head = elem;
}

char *
taskGet(task_T task, char *key) 
{
  if (!(task && key)) return NULL;

  for (elem_T elem=task->head; elem; elem=elem->link)
    if (strcmp(elem->key, key) == 0)
      return elem->val;

  return NULL;
}

void 
taskFree(task_T *task)
{
  if (!(task && *task)) return;

  elem_T elem, link;

  for (elem=(*task)->head; elem; elem=link) {
    link = elem->link;
    memFree(elem->key);
    memFree(elem->val);
    memFree(elem);
  }
  memFree(*task);
}

list_T
listNew(char *name)
{
  list_T list;
  list = calloc(1, sizeof(*list));
  if (list == NULL) return NULL;

  list->name = name; // TODO: do we need to strdup?
  list->len = 8;
  list->tasks = calloc(8, sizeof(task_T));
  if (list->tasks == NULL) {
    free(list);
    return NULL;
  }

  return list;
}

void
listFree(list_T *list)
{
  if (!(list && *list)) return;

  for (int i=0; i<(*list)->len; i++)
    taskFree(&(*list)->tasks[i]);

  free((*list)->name);
  free((*list)->tasks);
}

int
listAddTask(list_T list, task_T task)
{
  if (list->ntasks >= list->len) {
    list->len <<= 1;
    task_T *ptr = realloc(list->tasks, list->len);
    if (ptr == NULL) return -1; // TODO: return error code
    else list->tasks = ptr;
  }

  list->tasks[list->ntasks++] = task;
  
  return TD_OK;
}
