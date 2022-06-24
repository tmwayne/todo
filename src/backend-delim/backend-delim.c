//
// -----------------------------------------------------------------------------
// backend-delim.c
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

#include <stdio.h>           // fprintf
#include <stdlib.h>          // NULL
#include <string.h>          // strcasecmp
#include "delim-reader.h"    // parseDelim
#include "task.h"
#include "list.h"
#include "return-codes.h"
#include "error-functions.h" // errExit

void
readTasks_delim(list_T list, const char *filename)
{
  FILE *file = filename ? fopen(filename, "r") : stdin;
  if (!file)
    sysErrExit("Failed to open delimited data");

  dataframe_T data = parseDelim(
    file,       //
    '|',        // separator
    1,          // headers
    1           // quotes
  );

  if (!data)
    errExit("Failed to parse delimited data");

  for (int i=0; i < data->nfields; i++)
    listAddKey(list, data->headers->fields[i]);

  for (int i=0; i < data->nrecords; i++) {
    task_T task = taskNew();
    if (!task)
      errExit("Failed to allocate new task");

    for (int j=0; j < data->nfields; j++)
      taskSet(task, data->headers->fields[j], data->records[i]->fields[j]);

    if (taskCheckKeys(task) != TD_OK) 
      errExit("Task doesn't have all required keys");

    taskSetFlag(task, TF_NEW|TF_UPDATE);

    // TODO: remove this dependency on knowing the internal structure of list
    list->nupdates++;

    if (strcasecmp(taskGet(task, "status"), "Complete") != 0)
      listSetTask(list, task);
  }
}

