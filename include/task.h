// 
// -----------------------------------------------------------------------------
// task.h
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

#ifndef TASK_INCLUDED
#define TASK_INCLUDED

#include <stdbool.h> // bool

typedef struct elem_T *elem_T;
typedef struct task_T *task_T;
typedef struct cat_T  *cat_T;
typedef struct list_T *list_T;

extern task_T taskNew();
extern int    taskSize(const task_T); // TODO: rename this

/**
 * Sets the value of given key, which can't be NULL. If the key already exists
 * then the memory help by val is first free, then it is set to the new value.
 * If val is NULL, the the value is set to "". For both the key and val,
 * the arguments are copied.
 */
extern void    taskSet(task_T, const char *key, const char *val);
extern char   *taskGet(task_T, const char *key);
extern task_T  taskGetSubtask(const task_T);
extern task_T  taskGetNext(const task_T);

extern elem_T  taskElemInd(const task_T task, const int ind);
extern char   *elemKey(const elem_T);
extern char   *elemVal(const elem_T);
extern char   *taskValInd(const task_T task, const int ind);
extern char   *taskKeyInd(const task_T task, const int ind);
extern int     taskCheckKeys(const task_T);
extern void    taskFree(task_T *);
extern task_T  taskFindChildById(const task_T, const char *id);

extern char   *catName(const cat_T);
extern task_T  catGetTask(const cat_T, const task_T);

extern list_T  listNew(const char *);

/**
 * Tasks are set by their ids. Because this data is stored in the task itself,
 * it doesn't not need to be passed as an argument. We set tasks at the list
 * level and not the category level because a task's category can change, 
 * in which case it needs to be relocated in the list.
 */
extern int     listSetTask(list_T, task_T);
extern int     listAddKey(list_T, const char *key);
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

#endif // TASK_INCLUDED
