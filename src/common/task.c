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
#include "error-codes.h" // TD_OK
#include "task.h"

static char *required_keys[] = {
  "id",
  "parent_id",
  "category",
  "name",
  NULL
};

struct elem_T {
  char *key;
  char *val;
  struct elem_T *link;
};

struct task_T {
  elem_T head;
  elem_T tail;
  task_T llink;
  task_T rlink;
  task_T child;
  task_T parent;
  int update;
};

struct cat_T {
  char *name;   // name of the category
  int ntasks;   // number of tasks in the task linked list
  task_T tasks;  // task linked list
  cat_T link;   // link to next category
};

struct list_T {
  char *name;   // name of list
  char **keys;  // array of keys each of the tasks should have
  int nkeys;    // number of keys
  int keys_len; // length of keys array
  int ntasks;   // number of tasks
  int nupdates; // number of updates
  int ncats;    // number of categories
  cat_T cat;    // categories linked list
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
  memFree(*task);
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
  
static int
taskSwap(task_T old, task_T new)
{
  if (!(old && new)) return -1; // TODO: return error code

  struct task_T tmp = *old;

  new->llink = old->llink;
  new->rlink = old->rlink;
  new->child = old->child;
  new->parent = old->parent;
  *old = *new;

  *new = tmp;
  taskFree(&new);

  return TD_OK;
}

// -----------------------------------------------------------------------------
// Category
// -----------------------------------------------------------------------------

char *
catName(const cat_T cat)
{
  if (!cat) return NULL;
  else return cat->name;
}

task_T
catGetTask(const cat_T cat, task_T task)
{
  if (!cat) return NULL;

  else if (!task) return cat->tasks;

  else if (task->child) return task->child;

  else if (task->rlink) return task->rlink;

  else {
    for (task=task->parent; task; task=task->parent)
      if (task->rlink) return task->rlink;
  }

  return NULL;
}

static int
catFreeTasks(task_T *task)
{
  if (!(*task)) return TD_OK;

  catFreeTasks(&(*task)->rlink);
  catFreeTasks(&(*task)->child);

  taskFree(task);
  *task = NULL;

  return TD_OK;
}

// -----------------------------------------------------------------------------
// List
// -----------------------------------------------------------------------------

list_T
listNew(const char *name)
{
  list_T list;
  list = calloc(1, sizeof(*list));
  if (list == NULL) return NULL;

  list->name = strdup(name);

  list->keys_len = 8;
  list->keys = calloc(8, sizeof(char *));
  if (list->keys == NULL) {
    free(list);
    return NULL;
  }

  return list;
}

void
listFree(list_T *list)
{
  if (!(list && *list)) return;

  cat_T cat, next;

  for (cat = (*list)->cat; cat; ) {
    next = cat->link;
    catFreeTasks(&cat->tasks);
    free(cat->name);
    cat = next;
  }

  for (int i=0; i<(*list)->nkeys; i++)
    free((*list)->keys[i]);

  free((*list)->keys);
  free((*list)->name);

  *list = NULL;
}

/**
 * listGetCategory either fetches the category
 * matching the argument or creates the category,
 * if it doesn't exist, then returns it.
 */
static cat_T
getCategory(list_T list, char *name)
{
  cat_T cat = NULL;

  for (cat=list->cat; cat; cat=cat->link)
    if (strcmp(cat->name, name) == 0) return cat;

  cat = memCalloc(1, sizeof(*cat));
  if (!cat) return NULL; // TODO: return error code
  cat->name = strdup(name);
  cat->link = list->cat;
  list->cat = cat;
  list->ncats++;

  return cat;
}

static task_T
catFindTaskById(const cat_T cat, const char *id)
{
  if (!(cat && id)) return NULL;
  task_T task = NULL;
  while ((task = catGetTask(cat, task)))
    if (strcmp(taskGet(task, "id"), id) == 0)
      return task;

  return NULL;
}

task_T
listFindTaskById(const list_T list, const char *id)
{
  if (!(list && id)) return NULL;

  cat_T cat = NULL;
  while ((cat = listGetCat(list, cat))) {
    task_T task = catFindTaskById(cat, id);
    if (task) return task;
  }

  return NULL;
}

static int
listDeleteCat(list_T list, cat_T *cat)
{
  if (!(list && cat && *cat)) return -1; // TODO: return error code

  if (list->cat == *cat) list->cat = (*cat)->link;
  else {
    cat_T prev = list->cat;
    for ( ; prev->link == (*cat); prev=prev->link ) ;
    prev->link = (*cat)->link;
  }

  free((*cat)->name);
  free(*cat);
  *cat = NULL;

  return TD_OK;
}

static int
listRemoveTask(list_T list, task_T task)
{
  if (!(list && task)) return -1; // TODO: return error code

  cat_T cat = getCategory(list, taskGet(task, "category"));
  cat->ntasks--;

  // If we're the only task in the category,
  // then there isn't a parent.
  if (cat->ntasks == 0) listDeleteCat(list, &cat);

  // If we're the first task in category,
  // then there isn't a parent
  else if (cat->tasks == task) cat->tasks = task->rlink;

  // Otherwise, check if where the first child
  // of a parent
  else if (task->parent) {
    if (task->parent->child == task) 
      task->parent->child = task->rlink;
  }
      
  // If none of these, then there is a left link to adjust
  else if (task->llink) task->llink->rlink = task->rlink;

  if (task->rlink) task->rlink->llink = task->llink;

  task->llink = task->rlink = NULL;

  return TD_OK;
}

int
listSetTask(list_T list, task_T task)
{
  // TODO: there's currently a bug where if the edited task's 
  // parent is currently a child
  // First check if the task current exists
  task_T old = listFindTaskById(list, taskGet(task, "id"));
  if (old) {
    int new_placement = strcmp(taskGet(old, "parent_id"), 
      taskGet(task, "parent_id")) || strcmp(taskGet(old, "category"),
      taskGet(task, "category"));

    if (new_placement) listRemoveTask(list, old); // TODO: check for error

    list->nupdates += old->update ^ 1;
    taskSwap(old, task);
    task = old;
    task->update = 1;

    if (!(new_placement)) return TD_OK;

    // If we have to adjust the placement of the task,
    // then we let it fall through to the next section
    else list->ntasks--;

  }

  // If it doesn't check for an existing parent
  cat_T cat = getCategory(list, taskGet(task, "category"));
  task_T parent = listFindTaskById(list, taskGet(task, "parent_id"));

  if (parent) {
    if (parent->child) parent->child->llink = task;
    task->rlink = parent->child;
    task->llink = NULL;
    parent->child = task;
    task->parent = parent;

  // Otherwise, set as a new task
  } else {
    if (cat->tasks) cat->tasks->llink = task;
    task->rlink = cat->tasks;
    task->llink = NULL;
    cat->tasks = task;
  }

  cat->ntasks++;
  list->ntasks++;
  
  return TD_OK;
}

int
listAddKey(list_T list, const char *key)
{
  if (list->nkeys >= list->keys_len) {
    list->keys_len <<= 1;
    char **ptr = realloc(list->keys, list->keys_len * sizeof(char *));
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
listKeySize(const list_T list)
{
  if (!list) return -1; // TODO: return error code
  else return list->nkeys;
}

int
listContainsKey(const list_T list, const char *key)
{
  for (int i=0; i < listKeySize(list); i++)
    if (strcmp(list->keys[i], key) == 0) return 1;

  return 0;
}
  
cat_T
listGetCat(const list_T list, const cat_T cat)
{
  if (!list) return NULL;
  else if (!cat) return list->cat;
  else return cat->link;
}

int
listNumUpdates(const list_T list)
{
  if (!list) return 0;
  else return list->nupdates;
}

task_T *
listGetUpdates(const list_T list)
{

  if (!list->nupdates) return NULL;

  int len = 8, i = 0;
  task_T *updates = memCalloc(len, sizeof(task_T));
  if (!updates) return NULL;
  task_T task;

  cat_T cat = NULL;
  while ((cat = listGetCat(list, cat))) {
    task = NULL;
    while ((task = catGetTask(cat, task))) {
      if (task->update) {
        if (i >= len - 2) {
          len *= 2;
          updates = realloc(updates, len * sizeof(task_T));
          if (!updates) return NULL;
        }
        updates[i] = task;
        i++;
      }
    }
  }

  updates[i] = NULL;

  return updates;
}

int
listClearUpdates(list_T list)
{
  cat_T cat = NULL;
  while ((cat = listGetCat(list, cat))) {

    task_T task = NULL;
    while ((task = catGetTask(cat, task)))
      task->update = 0;

  }
  list->nupdates = 0;

  return TD_OK;
}
