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
#include "task.h"
#include "error-codes.h" // TD_OK

task_T 
taskNew() 
{
  task_T task;
  task = calloc(1, sizeof(*task));
  return task;
}

void
taskFree(task_T *task)
{
  if (!(task && *task)) return;

  free((*task)->name);
  free((*task)->effort);
  free((*task)->file_date);
  free((*task)->due_date);
  free(*task);
  *task = NULL;
}

int
taskFromArray(task_T task, char **arr)
{
  if (arr[0]) sscanf(arr[0], "%d", &task->id);
  else task->id = 0;

  if (arr[1]) sscanf(arr[1], "%d", &task->parent_id);
  else task->parent_id = 0;
  task->name = strdup(arr[2]);
  task->effort = strdup(arr[3]);
  task->file_date = strdup(arr[4]);
  task->due_date = strdup(arr[5]);

  return 0;
}

list_T
listNew(char *name)
{
  list_T list;
  list = calloc(1, sizeof(*list));
  if (list == NULL) return NULL;

  list->name = name; // TODO: do we need to strdup?
  list->len = 8;
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

  for (int i=0; i<(*list)->len; i++)
    taskFree(&(*list)->tasks[i]);

  free((*list)->name);
  free((*list)->tasks);
}

int
listAddTask(list_T list, task_T task)
{
  if (list->ntasks >= list->len) {
    list->len <<= 1;
    task_T *ptr = realloc(list->tasks, list->len);
    if (ptr == NULL) return -1; // TODO: return error code
    else list->tasks = ptr;
  }

  list->tasks[list->ntasks++] = task;
  
  return TD_OK;
}

