//
// -----------------------------------------------------------------------------
// todo.c
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

#include <stdio.h>           // printf, dprintf
#include <stdlib.h>          // calloc, getenv, srand, rand
#include <ncurses.h>         // initscr, cbreak, noecho, getch, endwin
#include <string.h>          // strdup
#include <time.h>            // time
#include "error-functions.h" // errMsg
#include "task.h"            // task_T
#include "view.h"

static task_T dummy_task;

void
printTask(task_T task)
{
  printf(
    "id:        %d\n"
    "name:      %s\n"
    "parent_id: %d\n"
    "effort:    %s\n"
    "file_date: %s\n"
    "due_date:  %s\n",
    task->id, task->name, task->parent_id, 
    task->effort, task->file_date, task->due_date
  );
}

task_T 
createTask(char *name)
{
  task_T task;
  task = calloc(1, sizeof(*task));
  if (task == NULL) errExit("calloc");

  srand(time(NULL));

  task->id        = rand() % 100;
  task->name      = name;
  task->parent_id = 0;
  task->effort    = "L";
  task->file_date = "2022-05-31";
  task->due_date  = "2022-07-01";

  return task;
}

int
drawTask(int row, int col, task_T task)
{
  char num[3];
  snprintf(num, 3, "%d", task->id);

  mvaddstr(row++, 0, num);
  mvaddstr(row++, 0, task->name);
  mvaddstr(row++, 0, task->effort);
  mvaddstr(row++, 0, task->file_date);
  mvaddstr(row++, 0, task->due_date);

  return row;
}

void
drawScreen(task_T *tasks, int len)
{
  clear();

  mvaddstr(0, 0, "Time to do it!");
  mvaddstr(1, 0, "--------------");

  int row = 2;
  for (int i=0; i<len; i++)
    row = drawTask(row, 0, tasks[i]);

  move(row-1, 0);
  refresh();

  return;
}

void
writeTaskToTmpFile(char *template, task_T task)
{
  int fd;

  fd = mkstemp(template);
  if (fd == -1)
    errExit("mkstemp");

  int size = dprintf(fd, 
    "%s\n"  // name
    "%s\n"  // effort
    "%s\n"  // file_date
    "%s\n", // due_date
    task->name, task->effort, task->file_date, task->due_date);

  if (size < 0) fatal("fprintf");
}

void
editTmpFile(char *pathname)
{
  char *editor = getenv("EDITOR");
  if (editor == NULL)
    fatal("No editor");

#define MAX_CMD_LEN 256
  char command[MAX_CMD_LEN];
  int n = snprintf(command, MAX_CMD_LEN-1, "%s %s", editor, pathname);

  // TODO: check return code
  def_prog_mode(); // save current tty modes
  endwin();

  int status = system(command);
  if (status == -1) {
    errExit("system");
  } else {
    if (WIFEXITED(status) && WEXITSTATUS(status) == 127)
      fatal("Editor returned 127, likely unable to invoke shell");
    else
      // TODO: check return status of shell
      // printWaitStatus(NULL, status);
      "nothing";
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
  task_T task;
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
      errExit("getline");

    if (buffer[i][nread-1] == '\n')
      buffer[i][nread-1] = '\0';
  }

  task->name = buffer[0];
  task->effort = buffer[1];
  task->file_date = buffer[2];
  task->due_date = buffer[3];

  return task;
}

void
editTask()
{
  // Write task to temp file
  char pathname[] = "/tmp/task-XXXXXX";
  writeTaskToTmpFile(pathname, dummy_task);

  // Open temp file in editor 
  editTmpFile(pathname);

  // Check validity of temporary file
  checkEditedFile(pathname);

  // Update results from temporary file
  task_T edited_task = parseEditedFile(pathname);

  // Update screen
  drawScreen(&edited_task, 1);
  
  return;
  
}

void
moveDown()
{
  int cur_x, cur_y, max_x, max_y;

  getyx(stdscr, cur_y, cur_x);
  getmaxyx(stdscr, max_y, max_y);

  if (cur_y < max_y) move(cur_y+1, cur_x);
}
    
void
moveUp()
{
  int cur_x, cur_y, max_x, max_y;

  getyx(stdscr, cur_y, cur_x);
  getmaxyx(stdscr, max_y, max_y);

  if (cur_y > 0) move(cur_y-1, cur_x);
}

void 
parser() 
{
  char c;
  while (c = getch()) {

    switch (c) {
    case 'e':
      editTask();
      break;

    case 'j':
      moveDown();
      break;

    case 'k':
      moveUp();
      break;

    case 'q':
      return;

    default:
      break;
    }

  }
}

void
view(int argc, char **argv)
{
  // dummy_task = createTask();
  task_T tasks[2] = {
    createTask("write todo"),
    createTask("celebrate")
  };


  initscr(); // TODO: check return value

  cbreak();   // disable line buffering
  noecho();   // disable echo for getch

  drawScreen(tasks, 2);

  parser();

  endwin(); // TODO: check return value
}

