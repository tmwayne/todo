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
#include <stdbool.h>     // bool, true, false
#include "mem.h"         // memCalloc, memFree
#include "task.h"
#include "error-codes.h" // TD_OK

struct elem_T {
  char *key;
  char *val;
  struct elem_T *link;
};

struct task_T {
  elem_T head;
  elem_T tail;
};

struct list_T {
  char *name;    // name of list
  char **keys;   // array of keys each of the tasks should have
  int nkeys;     // number of keys
  int keys_len;  // length of keys array
  task_T *tasks; // tasks array
  int ntasks;    // number of tasks
  int tasks_len; // length of tasks array
};

task_T 
taskNew() 
{
  task_T task;
  task = memCalloc(1, sizeof(*task));
  return task;
}

int 
taskSize(const task_T task) 
{
  if (!task) return -1;

  elem_T elem;
  int size = 0;
  for (elem=task->head; elem; elem=elem->link) size++;

  return size;
}

void 
taskSet(task_T task, const char *key, const char *val) 
{
  if (!(task && key)) return;
  if (*key == '\0') return;

  if (!val) val = "";

  elem_T elem;

  for (elem=task->head; elem; elem=elem->link) {
    if (strcmp(elem->key, key) == 0) {
      free(elem->val);
      elem->val = strdup(val);
      return;
    }
  }

  elem = memCalloc(1, sizeof(*elem));
  elem->key = strdup(key);
  elem->val = strdup(val);

  if (task->tail) 
    task->tail->link = elem;
  else
    task->head = elem; // head is empty, initialize it

  task->tail = elem;
  
}

char *
taskGet(const task_T task, const char *key) 
{
  if (!(task && key)) return NULL;

  for (elem_T elem=task->head; elem; elem=elem->link)
    if (strcmp(elem->key, key) == 0)
      return elem->val;

  return NULL;
}

elem_T
taskElemInd(const task_T task, const int ind)
{
  if (taskSize(task) == 0 || ind >= taskSize(task))
    return NULL;

  elem_T elem = task->head;
  for (int i=0; i<ind; i++, elem=elem->link);

  return elem;
}

char *
elemKey(const elem_T elem)
{
  if (!elem) return NULL;
  else return elem->key;
}

char *
elemVal(const elem_T elem)
{
  if (!elem) return NULL;
  else return elem->val;
}

char *
taskValInd(const task_T task, const int ind)
{
  return elemVal(taskElemInd(task, ind));
}

char *
taskKeyInd(const task_T task, const int ind)
{
  return elemKey(taskElemInd(task, ind));
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
listNew(const char *name)
{
  list_T list;
  list = calloc(1, sizeof(*list));
  if (list == NULL) return NULL;

  list->name = strdup(name);
  list->tasks_len = 8;
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

  int i;
  for (i=0; i < (*list)->nkeys; i++)
    free((*list)->keys[i]);

  for (i=0; i < (*list)->ntasks; i++)
    taskFree(&(*list)->tasks[i]);

  free((*list)->keys);
  free((*list)->tasks);
  free((*list)->name);
}

int
listAddTask(list_T list, const task_T task)
{
  if (list->ntasks >= list->tasks_len) {
    list->tasks_len <<= 1;
    task_T *ptr = realloc(list->tasks, list->tasks_len);
    if (ptr == NULL) return -1; // TODO: return error code
    else list->tasks = ptr;
  }

  list->tasks[list->ntasks++] = task;
  
  return TD_OK;
}

int
listAddKey(list_T list, const char *key)
{
  if (list->nkeys >= list->keys_len) {
    list->keys_len <<= 1;
    char **ptr = realloc(list->keys, list->keys_len);
    if (ptr == NULL) return -1; // TODO: return error code
    else list->keys = ptr;
  }

  list->keys[list->nkeys++] = strdup(key);

  return TD_OK;
}

char *
listName(const list_T list)
{
  if (!list) return NULL;
  else return list->name;
}

int
listSize(const list_T list)
{
  if (!list) return -1; // TODO: return error code
  else return list->ntasks;
}

int
listKeySize(const list_T list)
{
  if (!list) return -1; // TODO: return error code
  else return list->nkeys;
}

task_T
listGetTask(const list_T list, const int ind)
{
  if (!list || ind >= list->ntasks) return NULL;
  else return list->tasks[ind];
}

int
listUpdateTask(list_T list, const task_T task)
{

  if (!(list && task)) return -1; // TODO: return error code
  char *id = taskGet(task, "id");
  int i;
  for ( i=0; i < listSize(list); i++)
    if (strcmp(taskGet(list->tasks[i], "id"), id) == 0) break;

  taskFree(&list->tasks[i]);
  list->tasks[i] = task;
}

int
listClearTasks(list_T list, bool free_tasks)
{
  if (!list) return -1; // TODO: return error code

  if (free_tasks)
    for (int i=0; i < listSize(list); i++) {
      task_T task = listGetTask(list, i);
      taskFree(&task);
    }

  list->ntasks = 0;

  return TD_OK;
}
