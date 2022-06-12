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
#include "task.h"    // task_T

// These need to be multiples of 2
enum taskFlags {
  TF_NEW      = 1,
  TF_UPDATE   = 2,
  TF_COMPLETE = 4
};

struct elem_T {
  char *key;
  char *val;
  struct elem_T *link;
};

struct task_T {
  struct elem_T *head;   // head of linked list holding data
  struct elem_T *tail;   // tail of data
  struct task_T *llink;  // next task in tasks linked list
  struct task_T *rlink;  // prev task in tasks linked list
  struct task_T *child;  // head of subtask linked list
  struct task_T *parent; // parent task, if there is one
  int    level;         // depth of the task in the list tree
  int    flags;         // flags for indicating changes to the task
};

typedef struct elem_T *elem_T;
typedef struct task_T *task_T;

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

extern elem_T  taskElemInd(const task_T task, const int ind);
extern char   *elemKey(const elem_T);
extern char   *elemVal(const elem_T);
extern char   *taskValInd(const task_T task, const int ind);
extern char   *taskKeyInd(const task_T task, const int ind);
extern int     taskCheckKeys(const task_T);

extern task_T  taskGetSubtask(const task_T);
extern task_T  taskGetNext(const task_T);

extern int     taskSetFlag(task_T, const int flags);
extern int     taskGetFlag(const task_T, const int flag);

extern int     taskSetLevel(task_T, const int level);
extern int     taskGetLevel(const task_T);

extern int     taskSwap(task_T old, task_T new);
extern void    taskFree(task_T *);

#endif // TASK_INCLUDED
