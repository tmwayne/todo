//
// -----------------------------------------------------------------------------
// edit.c
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

#include <stdio.h>           // dprintf, snprintf, fopen
#include <stdlib.h>          // mkstemp, getenv, calloc, system
#include <curses.h>         // def_prog_mode, endwin, refresh
#include <string.h>          // getline
#include "error-functions.h" // errExit, fatal
#include "error-codes.h"     // TD_OK
#include "task.h"

void
writeTaskToTmpFile(char *template, task_T task)
{
  int fd;

  fd = mkstemp(template);
  if (fd == -1)
    sysErrExit("mkstemp");

  int size = dprintf(fd, 
    "%s\n"  // name
    "%s\n"  // effort
    "%s\n"  // file_date
    "%s\n", // due_date
    task->name, task->effort, task->file_date, task->due_date);

  if (size < 0)
    errExit("Error writing task to temp file");
}

// TODO: need version when not in view (curses) mode
void
editTmpFile(char *pathname)
{
  char *editor = getenv("EDITOR");
  if (editor == NULL)
    errExit("EDITOR isn't set");

#define MAX_CMD_LEN 256
  char command[MAX_CMD_LEN];
  int n = snprintf(command, MAX_CMD_LEN-1, "%s %s", editor, pathname);

  // TODO: check return code
  def_prog_mode(); // save current tty modes
  endwin();

  int status = system(command);
  if (status == -1) {
    sysErrExit("system");
  } else {
    if (WIFEXITED(status) && WEXITSTATUS(status) == 127)
      errExit("Editor returned 127, likely unable to invoke shell");
    // else
      // TODO: check return status of shell
      // printWaitStatus(NULL, status);
      // "nothing";
  }

  refresh(); // restore save modes, repaint screen

  return;
}

void
checkEditedFile(char *pathname)
{
  return;
}

task_T
parseEditedFile(char *pathname)
{
  task_T task = taskNew(NULL);
  task = calloc(1, sizeof(*task));
  
  FILE *file = fopen(pathname, "r");
  if (!file) errExit("fopen");

#define STRUCT_ELEM 4
  char *buffer[STRUCT_ELEM] = {0};
  size_t len = 0;
  int nread;

  for (int i=0; i < STRUCT_ELEM; i++) {
    nread = getline(&buffer[i], &len, file);
    if (nread == -1)
      sysErrExit("getline");

    if (buffer[i][nread-1] == '\n')
      buffer[i][nread-1] = '\0';
  }

  task->name = buffer[0];
  task->effort = buffer[1];
  task->file_date = buffer[2];
  task->due_date = buffer[3];

  return task;
}

int
validateEditedTask()
{
  return TD_OK;
}

int
replaceTask(task_T *old, task_T new)
{
  new->id = (*old)->id;

  // TODO: allow parent id to be edited
  new->parent_id = (*old)->parent_id;

  taskFree(old);
  *old = new;

  return TD_OK;
}

// TODO: check if any edit was actually made
int
editTask(task_T *task)
{
  // Write task to temp file
  char pathname[] = "/tmp/task-XXXXXX";
  writeTaskToTmpFile(pathname, *task);

  // Open temp file in editor 
  editTmpFile(pathname);

  // Check validity of temporary file
  checkEditedFile(pathname);

  // Update results from temporary file
  task_T edited_task = parseEditedFile(pathname);

  // TODO: 
  validateEditedTask();

  // TODO: do we write the task to file before overwriting existing task?
  
  replaceTask(task, edited_task);
  
  return TD_OK;
  
}

