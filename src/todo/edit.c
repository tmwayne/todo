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
#include <unistd.h>          // unlink
#include <curses.h>          // def_prog_mode, endwin, refresh
#include <string.h>          // getline
#include <ctype.h>           // isspace
#include <errno.h>           // errno
#include "error-functions.h" // errExit, fatal
#include "error-codes.h"     // TD_OK
#include "task.h"            // elem_T

static int
writeTaskToTmpFile(char *template, const task_T task)
{
  int fd = mkstemp(template);
  if (fd == -1) return -1; // TODO: return error code

  for (int i=0; i < taskSize(task); i++) {
    elem_T elem = taskElemInd(task, i);
    if (strcmp(elemKey(elem), "id") == 0) continue;
    int size = dprintf(fd, "%s: %s\n", elemKey(elem), elemVal(elem));
    if (size < 0)
      errExit("Error writing task to temp file");
  }

  if (close(fd) == -1) return -1; // TODO: return error code

  return TD_OK;

}

// TODO: need version when not in view (curses) mode
static int
editTmpFile(const char *pathname)
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

  return TD_OK;
}

static int
trim(char **buf, ssize_t *len)
{
  // TODO: add error checking
  while (isspace((unsigned char) (*buf)[(*len)-1])) {
    (*buf)[(*len)-1] = '\0';
    (*len)--;
  }

  while (isspace((unsigned char) *(*buf))) {
    (*buf)++;
    (*len)--;
  }

  return 0;
}

static int
parseEditedFile(const char *pathname, task_T task)
{
  FILE *file = fopen(pathname, "r");
  if (!file) errExit("fopen");

  char *buf = NULL;
  size_t len = 0;
  ssize_t nread;

  int row = 0;
  errno = 0;
  while ((nread = getline(&buf, &len, file)) > 0) {

    char *key = buf;
    char *val;
    trim(&key, &nread);

    // Skip if line is only whitespace
    if (nread == 0) continue; 

    val = strchr(key, ':');
    if (val == NULL) {
      free(buf);
      errno = -1;
      break;
    }

    *val++ = '\0';
    if (*val == ' ') *val++ = '\0';

    taskSet(task, key, val);

    // Free memory allocated by getline and reset for next line parse
    free(buf);
    buf = NULL;
    len = 0;

    row++;
    errno = 0;
  }

  fclose(file);

  if (errno != 0) {
    taskFree(&task);
    return -1; // TODO: return error code
  }

  return TD_OK;
}

static int
enforceParentCategory(const list_T list, task_T edit)
{
  task_T task = listFindTaskById(list, taskGet(edit, "id"));

  char *task_parent = taskGet(task, "parent_id");
  char *edit_parent = taskGet(edit, "parent_id");

  task_T parent = listFindTaskById(list, edit_parent);

  // Case 1: No parent id or parent id is removed
  // Allow any change to category
  if (strcmp(edit_parent, "") == 0) ;

  // Case 2: Parent id is unchanged
  // Enforce that the category must go unchanged
  else if (strcmp(task_parent, edit_parent) == 0)
    taskSet(edit, "category", taskGet(parent, "category"));

  // Case 3: Parent id is added or changed
  // Error if new id of parent doesn't exist, otherwise
  // enforce that the category is that of the new parent
  else {
    // TODO: return error code indicating change to non-existing parent
    if (!parent) return -1; 

    else taskSet(edit, "category", taskGet(parent, "category"));
  }

  return TD_OK;
}

static int
validateEditedTask(const list_T list, task_T edit)
{
  if (taskCheckKeys(edit) != TD_OK) return -1;

  enforceParentCategory(list, edit);

  for (int i=0; i < taskSize(edit); i++) {
    elem_T elem = taskElemInd(edit, i);
    if (!listContainsKey(list, elemKey(elem))) return -1;
  }

  return TD_OK;
}

// TODO: check if any edit was actually made
// TODO: throw the appropriate error codes here
int
editTask(list_T list, task_T task)
{
  task_T edit = taskNew();
  taskSet(edit, "id", taskGet(task, "id"));
  taskSet(edit, "category", taskGet(task, "category"));

  char pathname[] = "/tmp/task-XXXXXX";
  if (writeTaskToTmpFile(pathname, task) != TD_OK)
    return -1;

  if (editTmpFile(pathname) != TD_OK)
    return -1;

  if (parseEditedFile(pathname, edit) != TD_OK)
    return -1;

  unlink(pathname);

  if (validateEditedTask(list, edit) != TD_OK)
    return -1;

  if (listSetTask(list, edit) != TD_OK)
    return -1;
  
  return TD_OK;
  
}

