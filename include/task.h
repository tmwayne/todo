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

typedef struct task_T *task_T;

typedef struct {
  char *name;    // name of list
  int len;       // length of tasks array
  int ntasks;    // number of tasks
  task_T *tasks; // tasks array
} *list_T;

extern task_T taskNew();
extern int    taskSize(task_T);
extern void   taskSet(task_T, char *key, char *val);
extern char  *taskGet(task_T, char *key);
// extern char  *taskKeys(task_T);
extern void   taskFree(task_T *);

extern list_T listNew(char *);
extern void listFree(list_T *);
extern int listAddTask(list_T, task_T);

#endif // TASK_INCLUDED
