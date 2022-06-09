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
  // task_T parent;
};

struct cat_T {
  char *name;   // name of the category
  int ntasks;   // number of tasks in the task linked list
  task_T task;  // task linked list
  cat_T link;   // link to next category
};

struct list_T {
  char *name;   // name of list
  char **keys;  // array of keys each of the tasks should have
  int nkeys;    // number of keys
  int keys_len; // length of keys array
  int ntasks;   // number of tasks
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

// -----------------------------------------------------------------------------
// Category
// -----------------------------------------------------------------------------

char *
catName(const cat_T cat)
{
  if (!cat) return NULL;
  else return cat->name;
}

cat_T
catGetNext(const cat_T cat)
{
  if (!cat) return NULL;
  else return cat->link;
}

task_T
catGetTask(const cat_T cat)
{
  if (!cat) return NULL;
  else return cat->task;
}

static int
catFree(task_T *task)
{
  if (!(*task)) return TD_OK;

  catFree(&(*task)->rlink);
  catFree(&(*task)->child);

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
    catFree(&cat->task);
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
findParent(task_T task, char *parent_id)
{
  task_T parent;
  if (!task) return NULL;

  else if (strcmp(taskGet(task, "id"), parent_id) == 0)
    return task;

  else if (task->child) {
    parent = findParent(task->child, parent_id);
    if (parent) return parent;
  }

  else if (task->rlink) {
    parent = findParent(task->rlink, parent_id);
    if (parent) return parent;
  }

  return NULL;
}

// TODO: edit
int
listAddTask(list_T list, const task_T task)
{
  char *name = taskGet(task, "category");
  cat_T cat = getCategory(list, name);

  task_T parent = findParent(cat->task, taskGet(task, "parent_id"));
  if (parent) {
    if (parent->child) parent->child->llink = task;
    task->rlink = parent->child;
    parent->child = task;
  } else {
    if (cat->task) cat->task->llink = task;
    task->rlink = cat->task;
    cat->task = task;
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
    char **ptr = reallocarray(list->keys, list->keys_len, sizeof(char *));
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
listNumTasks(const list_T list)
{
  if (!list) return -1; // TODO: return error code
  else return list->ntasks;
}

int
listNumCats(const list_T list)
{
  if (!list) return -1; // TODO: return error code
  else return list->ncats;
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
listGetCat(const list_T list)
{
  if (!list) return NULL;
  else return list->cat;
}

static task_T
findPrevTaskById(task_T prev, char *id)
{
  if (!prev) return NULL;

  if (prev->child && strcmp(taskGet(prev->child, "id"), id) == 0)
    return prev;

  if (prev->rlink && strcmp(taskGet(prev->rlink, "id"), id) == 0)
    return prev;

  task_T ret = findPrevTaskById(prev->child, id);
  if (ret) return ret;

  ret = findPrevTaskById(prev->rlink, id);
  if (ret) return ret;

  return NULL;
}

int
listUpdateTask(list_T list, task_T task)
{
  char *id = taskGet(task, "id");
  // Note we can't start with the category, because
  // the category could have changed
  char *category = taskGet(task, "category");

  task_T old;
  task_T prev;

  cat_T cat = listGetCat(list);
  for ( ; cat; cat = catGetNext(cat)) {
    if (cat->ntasks) {
      if (strcmp(taskGet(cat->task, "id"), id) == 0) {
        old = cat->task;
        cat->task = task;
        break;
      }
      // When we find the previous task, we have to check if it's
      // a parent or a link
      else if ((prev = findPrevTaskById(cat->task, id))) {
        if (prev->child && strcmp(taskGet(prev->child, "id"), id) == 0) {
          old = prev->child;
          prev->child = task;
          break;
        } else {
          old = prev->rlink;
          break;
        }
      }
    }
  }

  task->child = old->child;
  if (old->rlink) old->rlink->llink = task;
  if (old->llink) old->llink->rlink = task;
  task->rlink = old->rlink;
  task->llink = old->llink;

  return TD_OK;

  // TODO: Check if category has changed, move task if needed
}
  

