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

#include <stdio.h>        // sscanf
#include <stdlib.h>       // calloc, free
#include <string.h>       // strdup
#include <stdbool.h>      // bool, true, false
#include "mem.h"          // memCalloc, memFree
#include "return-codes.h" // TD_OK
#include "task.h"

static char *required_keys[] = {
  "id",
  "parent_id",
  "category",
  "name",
  "status",
  "timing", // TODO: remove this
  NULL
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
  if (!task) return TD_INVALIDARG;

  elem_T elem;
  int size = 0;
  for (elem=task->head; elem; elem=elem->link) size++;

  return size;
}

// TODO: throw error if alloc fails
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
  if (!elem) return;

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

int
taskCheckKeys(const task_T task)
{
  for (int i=0; required_keys[i]; i++)
    if (taskGet(task, required_keys[i]) == NULL)
      return -1;

  return TD_OK;
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
  free(*task);
  *task = NULL;
}

task_T
taskGetSubtask(const task_T task)
{
  if (!task) return NULL;
  else return task->child;
}

task_T
taskGetNext(const task_T task)
{
  if (!task) return NULL;
  else return task->rlink;
}
  
int
taskSwap(task_T old, task_T new)
{
  if (!(old && new)) return TD_INVALIDARG;

  struct task_T tmp = *old;

  new->llink = old->llink;
  new->rlink = old->rlink;
  new->child = old->child;
  new->parent = old->parent;
  new->level = old->level;
  new->flags |= old->flags; // TODO: double check that we want to do this
  *old = *new;

  *new = tmp;
  taskFree(&new);

  return TD_OK;
}

static int
hasInvalidFlag(const int flags)
{
  return flags & ~(TF_NEW | TF_UPDATE | TF_COMPLETE | TF_DELETE);
}


int
taskSetFlag(task_T task, const int flags)
{
  if (hasInvalidFlag(flags)) return TD_INVALIDARG;
  task->flags |= flags;
  return TD_OK;
}

int
taskUnsetFlag(task_T task, const int flags)
{
  if (hasInvalidFlag(flags)) return TD_INVALIDARG;
  task->flags &= ~flags;
  return TD_OK;
}

int
taskGetFlag(task_T task, const int flags)
{
 // `& flags` is a bit-mask for non-relevant bits
 if (~(task->flags ^ flags) & flags) return 1;
 else return 0;
}

int
taskSetLevel(task_T task, const int level)
{
  if (!task && level > 0) return TD_INVALIDARG;
  task->level = level;

  return TD_OK;
}

int
taskGetLevel(const task_T task)
{
  if (!task) return TD_INVALIDARG;
  return task->level;
}
