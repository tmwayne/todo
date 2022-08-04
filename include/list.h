// 
// -----------------------------------------------------------------------------
// list.h
// -----------------------------------------------------------------------------
//
// Description
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

#ifndef LIST_INCLUDED
#define LIST_INCLUDED

#include "task.h" // task_T

// TODO: make naming of linked list heads consistent
// some use the singular, some use the plural
struct cat_T {
  char         *name;     // name of the category
  int           ntasks;   // number of tasks in the task linked list
  int           nopen;    // number of tasks that haven't been completed
  task_T        tasks;    // task linked list
  struct cat_T *link;     // link to next category
};

struct list_T {
  char         *name;     // name of list
  char        **keys;     // array of keys each of the tasks should have
  int           nkeys;    // number of keys
  int           keys_len; // length of keys array
  int           ntasks;   // number of tasks
  int           nupdates; // number of updates
  int           maxid;    // highest id of all tasks 
  int           ncats;    // number of categories
  struct cat_T *cat;      // categories linked list
};

typedef struct cat_T *cat_T;
typedef struct list_T *list_T;

extern task_T  taskFindChildById(const task_T, const char *id);

extern char   *catName(const cat_T);
extern task_T  catGetTask(const cat_T, const task_T);
extern int     catNumOpen(const cat_T);

extern list_T  listNew(const char *);

/**
 * Tasks are set by their ids. Because this data is stored in the task itself,
 * it doesn't not need to be passed as an argument. We set tasks at the list
 * level and not the category level because a task's category can change, 
 * in which case it needs to be relocated in the list.
 */
extern int     listSetTask(list_T, task_T);
extern int     listAddKey(list_T, const char *key);
extern int     listNumKeys(const list_T);
extern char  **listGetKeys(const list_T);
extern int     listContainsKey(const list_T, const char *key);
extern char   *listName(const list_T);

/**
 * If cat is NULL, returns the first category. If cat is not null, then
 * returns the next category.
 */
extern cat_T   listGetCat(const list_T, const cat_T);
extern task_T  listFindTaskById(const list_T, const char *id);

/**
 * Returns an array of tasks that have been updated
 */
extern task_T *listGetUpdates(const list_T list);
extern int     listNumUpdates(const list_T list);
extern int     listClearUpdates(list_T list);
extern void    listFree(list_T *);
extern int     listGetMaxId(const list_T);

// TODO: can we combine these two functions?
extern int     markComplete(list_T, task_T);
extern int     markDelete(list_T, task_T);

#endif // LIST_INCLUDED
