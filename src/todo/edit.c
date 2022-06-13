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
#include <sys/stat.h>        // fstat
#include <sys/mman.h>        // mmap, munmap, MAP_FAILED
#include <inttypes.h>        // uint32_t, uint8_t
#include <fcntl.h>           // open
#include "error-functions.h" // errExit, fatal
#include "error-codes.h"     // TD_OK
#include "task.h"            // elem_T
#include "list.h"            // list_T
#include "screen.h"          // line_T, lineType, lineObj

static void
writeTaskToTmpFile(char *template, const task_T task)
{
  int fd = mkstemp(template);
  if (fd == -1) 
    errExit("Failed editing task: temp file not successfully created");

  for (int i=0; i < taskSize(task); i++) {
    elem_T elem = taskElemInd(task, i);
    if (strcmp(elemKey(elem), "id") == 0) continue;
    int size = dprintf(fd, "%s: %s\n", elemKey(elem), elemVal(elem));
    if (size < 0)
      errExit("Failed editing task: write unsuccessful");
  }

  if (close(fd) == -1) 
    errExit("Failed editing task: unable to close temp file");

}

// http://www.rosettacode.org/wiki/CRC-32#C
static uint32_t
crc32(uint32_t crc, const char *buf, size_t len)
{
  uint32_t table[256];
  int have_table = 0;
  uint32_t rem;
  uint8_t octet;
  int i, j;
  const char *p, *q;

  // This check is not thread safe, there is no mutex
  if (have_table == 0) {

    // Calculate CRC table
    for (i = 0; i < 256; i++) {
      rem = i; // remainder from polynomial division
      for (j = 0; j < 8; j++) {
        if (rem & 1) {
          rem >>= 1;
          rem ^= 0xedb88320;
        } else
          rem >>= 1;
      }
      table[i] = rem;
    }
    have_table = 1;
  }

  crc = ~crc;
  q = buf + len;
  for (p = buf; p < q; p++) {
    octet = *p; // cast to unsigned octet
    crc = (crc >> 8) ^ table[(crc & 0xff) ^ octet];
  }

  return ~crc;
}

/**
 * If any of the system calls fail, then we return 0 and parse the
 * file regardless. We do exit the program if the system calls
 * to release the resources fail.
 */
static uint32_t
genCRC32(const char *pathname)
{
  int fd;
  struct stat statbuf;
  char *buf;

  if ((fd = open(pathname, O_RDONLY)) == -1) return 0;
  
  if (fstat(fd, &statbuf) == -1) return 0;

  buf = mmap(0, statbuf.st_size, PROT_READ, MAP_SHARED, fd, 0);
  if (buf == MAP_FAILED) return 0;

  uint32_t crc = crc32(0, (const char *) buf, statbuf.st_size);

  if (munmap(buf, statbuf.st_size) == -1)
    sysErrExit("Failed to edit file: unable to unmap temp file for crc-32");

  if (close(fd) == -1)
    sysErrExit("Failed to edit file: unable to close temp file for crc-32");

  return crc;

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

  uint32_t hash_before = genCRC32(pathname);

  if (def_prog_mode() == ERR)
    errExit("Failed to open editor: terminal process not created"); 
  endwin();

  int status = system(command);
  if (status == -1) {
    sysErrExit("Failed to open editor: editor process could not be created");
  } else {
    if (WIFEXITED(status) && WEXITSTATUS(status) == 127)
      errExit("Editor returned 127, likely unable to invoke shell");
  }

  refresh(); // restore save modes, repaint screen

  uint32_t hash_after = genCRC32(pathname);

  return (!hash_before || hash_before != hash_after) ? ET_MOD : ET_UNMOD;

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

static void
parseEditedFile(const char *pathname, task_T task)
{
  FILE *file = fopen(pathname, "r");
  if (!file) 
    sysErrExit("Failed to parse edited file: unable to open temp file");

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
    errExit("Failed to parse edited task");
  }
}

// TODO: changing category of parent task should change
// category of child tasks as well
static void
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
  // Error if new id of parent doesn't exist
  else {
    if (!parent)
      errExit("Edited task invalid: task belonging to new parent doesn't exit");

    // Error if the new parent is one of the children
    task_T child = taskFindChildById(task, edit_parent);
    if (child) 
      errExit("Edited task invalid: a subtask can't become that task's parent");

    // Otherwise enforce that the category is that of the new parent
    taskSet(edit, "category", taskGet(parent, "category"));

  }
}

static void
validateEditedTask(const list_T list, task_T edit)
{
  if (taskCheckKeys(edit) != TD_OK)
    errExit("Edited task invalid: missing required fields");

  enforceParentCategory(list, edit);

  for (int i=0; i < taskSize(edit); i++) {
    elem_T elem = taskElemInd(edit, i);
    if (!listContainsKey(list, elemKey(elem)))
      errExit("Edited task invalid: unrecognized or corrupted fields");
  }
}

int
editTask(list_T list, task_T task)
{
  if (!(list && task))
    errExit("Failed to edit task: null pointer passed as argument");
  
  task_T edit = taskNew();
  taskSet(edit, "id", taskGet(task, "id"));
  taskSet(edit, "category", taskGet(task, "category"));

  taskSetFlag(edit, TF_UPDATE);

  char pathname[] = "/tmp/task-XXXXXX";
  writeTaskToTmpFile(pathname, task);

  int rc = editTmpFile(pathname);

  // If the first hash is zero or the hashes are different
  // then the file has been edited.
  if (rc == ET_UNMOD) {
    taskFree(&edit);
    unlink(pathname);
    return ET_UNMOD;
  }

  parseEditedFile(pathname, edit);

  unlink(pathname);

  validateEditedTask(list, edit);

  if (listSetTask(list, edit) != TD_OK)
    errExit("Failed to edit task: unable to update task in list");

  return ET_MOD;
}


// TODO: how do we validate that a meaningful task was created?
void
addTask(list_T list, line_T line)
{
  if (!(list && line))
    errExit("Failed to add task: null pointer passed as argument");

  task_T task = taskNew();
  char **keys = listGetKeys(list);

  for (int i=0; i < listNumKeys(list); i++)
    taskSet(task, keys[i], NULL);

  taskSetFlag(task, TF_NEW);
  
  // TODO: can we make this magic number more robust?

#define BUF_LEN 16
  char id[BUF_LEN];
  snprintf(id, BUF_LEN, "%d", listGetMaxId(list)+1);

  taskSet(task, "id", id); 

  switch (lineType(line)) {
  case LT_CAT:
    taskSet(task, "category", catName((cat_T) lineObj(line)));
    break;
  
  case LT_TASK:
    taskSet(task, "parent_id", taskGet((task_T) lineObj(line), "id"));
    taskSet(task, "category", taskGet((task_T) lineObj(line), "category"));
    break;

  default:
    break;
  }

  if (listSetTask(list, task) != TD_OK)
    errExit("Failed to add task: unable to add task before editing");

  editTask(list, task);
}

