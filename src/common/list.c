//
// -----------------------------------------------------------------------------
// list.c
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

#include <stdlib.h>      // free
#include <string.h>      // strcmp, strdup
#include "error-codes.h" // TD_OK
#include "mem.h"         // memCalloc, memResize
#include "task.h"
#include "list.h"

// TODO: decouple this from catGetTask and move back to task.c
task_T
taskFindChildById(const task_T task, const char *id)
{
  if (!(task && task->child && id)) return NULL;
  task_T child = task->child;
  do {
    if (strcmp(taskGet(child, "id"), id) == 0)
      return child;
  } while ((child = catGetTask(NULL, child)));

  return NULL;
}

char *
catName(const cat_T cat)
{
  if (!cat) return NULL;
  else return cat->name;
}

// TODO: this works but isn't clear for some cases. Create an interface
// that is more intuitive
/**
 * This traverses the category depth first and returns each task 
 * in turn until all have been returned
 */
task_T
catGetTask(const cat_T cat, task_T task)
{
  if (!(cat || task)) return NULL;

  else if (!task) return cat->tasks;

  else if (task->child) return task->child;

  else if (task->rlink) return task->rlink;

  else {
    for (task=task->parent; task; task=task->parent)
      if (task->rlink) return task->rlink;
  }

  return NULL;
}

static void
catFreeTasks(task_T *task)
{
  if (!(task && *task)) return;

  catFreeTasks(&(*task)->rlink);
  catFreeTasks(&(*task)->child);

  taskFree(task);
  *task = NULL;
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
  if (!cat) return NULL; 

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

int
catNumOpen(const cat_T cat)
{
  if (!cat) return 0;
  return cat->nopen;
}

// -----------------------------------------------------------------------------
// List
// -----------------------------------------------------------------------------

list_T
listNew(const char *name)
{
  list_T list;
  list = memCalloc(1, sizeof(*list));
  if (!list) return NULL;

  list->name = strdup(name);

  list->keys_len = 8;
  list->keys = memCalloc(8, sizeof(char *));
  if (!list->keys) {
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

/**
 * This deletes the category and relinks the list.
 * It doesn't free the memory held by any tasks in
 * the category
 */
static int
listDeleteCat(list_T list, cat_T *cat)
{
  if (!(list && cat && *cat)) return TD_INVALIDARG;

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
taskNumChildren(task_T task)
{
  int n = 0;
  if (!task) return 0;

  int stop = task->level;
  while ((task = catGetTask(NULL, task)) && task->level > stop) 
    n++;

  return n;
}

static int
taskNumChildrenOpen(task_T task)
{
  int n = 0;
  if (!task) return 0;

  int stop = task->level;
  while ((task = catGetTask(NULL, task)) && task->level > stop)
    if (!taskGetFlag(task, TF_COMPLETE)) n++;

  return n;
}

static int
listPopTask(list_T list, task_T task)
{
  if (!(list && task)) return TD_INVALIDARG;

  cat_T cat = getCategory(list, taskGet(task, "category"));

  // If we're the only task in the category,
  // then there isn't a parent.
  if (cat->ntasks == 0) listDeleteCat(list, &cat);

  // If we're the first task in category,
  // then there isn't a parent
  else if (cat->tasks == task) cat->tasks = task->rlink;

  // Otherwise, check if we're the first child of a parent
  else if (task->parent) {
    if (task->parent->child == task) 
      task->parent->child = task->rlink;
  }
      
  // If none of these, then there is a left link to adjust
  else if (task->llink) task->llink->rlink = task->rlink;

  if (task->rlink) task->rlink->llink = task->llink;

  // Sever the task from the tree
  task->parent = task->llink = task->rlink = NULL;

  // Update accounting for ntasks and nopen
  list->ntasks--;
  cat->ntasks -= taskNumChildren(task) + 1;
  cat->nopen -= taskNumChildrenOpen(task) + !taskGetFlag(task, TF_COMPLETE);

  return TD_OK;
}

static void
taskAdjustSubtreeLevels(task_T task, const int level)
{
  if (!task) return;
  
  task->level = level;
  if (task->child) taskAdjustSubtreeLevels(task->child, level + 1);
  if (task->rlink) taskAdjustSubtreeLevels(task->rlink, level);
}

int
listSetTask(list_T list, task_T task)
{
  // First check if the task current exists
  task_T old = listFindTaskById(list, taskGet(task, "id"));
  if (old) {
    int new_placement = strcmp(taskGet(old, "parent_id"), 
      taskGet(task, "parent_id")) || strcmp(taskGet(old, "category"),
      taskGet(task, "category"));

    if (new_placement) listPopTask(list, old); // TODO: check for error

    list->nupdates += !(old->flags & TF_UPDATE);
    taskSwap(old, task);
    task = old;

    // If we have to adjust the placement of the task,
    // then we let it fall through to the next section
    if (!(new_placement)) return TD_OK;
  }

  // If it doesn't check for an existing parent
  cat_T cat = getCategory(list, taskGet(task, "category"));
  task_T parent = listFindTaskById(list, taskGet(task, "parent_id"));

  if (parent) {
    if (parent->child) parent->child->llink = task;
    task->rlink = parent->child;
    task->llink = NULL;
    parent->child = task;
    taskAdjustSubtreeLevels(task, parent->level+1);
    task->parent = parent;

  // Otherwise, set as a new task
  } else {
    if (cat->tasks) cat->tasks->llink = task;
    task->rlink = cat->tasks;
    task->llink = NULL;
    cat->tasks = task;
    taskAdjustSubtreeLevels(task, 0);
  }

  int id = strtol(taskGet(task, "id"), NULL, 10);
  if (id > list->maxid) list->maxid = id;

  cat->nopen += taskNumChildrenOpen(task) + !taskGetFlag(task, TF_COMPLETE);
  cat->ntasks += taskNumChildren(task) + 1;
  list->ntasks++;
  
  return TD_OK;
}

int
listAddKey(list_T list, const char *key)
{
  if (list->nkeys >= list->keys_len) {
    list->keys_len <<= 1;
    char **ptr = memResize(list->keys, list->keys_len * sizeof(char *));
    if (!ptr) return -1; // TODO: return error code
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
listNumKeys(const list_T list)
{
  if (!list) return TD_INVALIDARG;
  else return list->nkeys;
}

// TODO: make this interface consistent with the other ones
char **
listGetKeys(const list_T list)
{
  if (!list) return NULL;
  else return list->keys;
}

int
listContainsKey(const list_T list, const char *key)
{
  for (int i=0; i < listNumKeys(list); i++)
    if (strcmp(list->keys[i], key) == 0) 
      return 1;

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
      if (task->flags & TF_UPDATE) {
        if (i >= len - 2) {
          len *= 2;
          updates = memResize(updates, len * sizeof(task_T));
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
      task->flags &= ~(TF_UPDATE);

  }
  list->nupdates = 0;

  return TD_OK;
}

int 
markComplete(list_T list, task_T task)
{
  if (!task) return TD_INVALIDARG;
  cat_T cat = getCategory(list, taskGet(task, "category"));
  if (!cat) return -1; // TODO: return error code

  int stop = task->level;
  do {
    taskSet(task, "status", "Complete");
    list->nupdates += !(task->flags & TF_UPDATE);
    if (!taskGetFlag(task, TF_COMPLETE)) 
      cat->nopen--;
    taskSetFlag(task, TF_UPDATE | TF_COMPLETE);
    task = catGetTask(NULL, task);
  } while (task && task->level > stop);

  return TD_OK;
}
  
int
listGetMaxId(const list_T list)
{
  if (!list) return TD_INVALIDARG;
  else return list->maxid;
}
