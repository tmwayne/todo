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
typedef struct list_T *list_T;

extern task_T taskNew();
extern int    taskSize(const task_T);

/**
 * Sets the value of given key, which can't be NULL. If the key already exists
 * then the memory help by val is first free, then it is set to the new value.
 * If val is NULL, the the value is set to NULL. For both the key and val,
 * the arguments are copied.
 */
extern void   taskSet(task_T, const char *key, const char *val);
extern char  *taskGet(task_T, const char *key);
extern elem_T taskElemInd(const task_T task, const int ind);
extern char  *elemKey(const elem_T);
extern char  *elemVal(const elem_T);
extern char  *taskValInd(const task_T task, const int ind);
extern char  *taskKeyInd(const task_T task, const int ind);
extern void   taskFree(task_T *);
extern int    taskValidate(const task_T); // TODO: write this

extern list_T listNew(const char *);
extern int    listAddTask(list_T, const task_T);
extern int    listAddKey(list_T, const char *);
extern task_T listGetTask(const list_T, const int ind);
extern int    listUpdateTask(list_T, const task_T);
extern char  *listName(const list_T);
extern int    listClearTasks(list_T, bool free_tasks);
extern void   listFree(list_T *);
extern int    listSize(const list_T);

#endif // TASK_INCLUDED
