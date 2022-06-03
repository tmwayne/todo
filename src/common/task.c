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

#include <stdio.h>  // sscanf
#include <stdlib.h> // calloc, free
#include <string.h> // strdup
#include "task.h"

task_T 
Task_new() 
{
  task_T task;
  task = calloc(1, sizeof(*task));

  return task;
}

void
Task_free(task_T *task)
{
  if (task && *task) {
    free(*task);
    *task = NULL;
  }
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
