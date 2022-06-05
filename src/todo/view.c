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
#include <unistd.h>          // unlink, close
#include <ncurses.h>         // initscr, cbreak, noecho, getch, endwin
#include <string.h>          // strdup
#include <time.h>            // time
#include "error-functions.h" // errMsg
#include "task.h"            // task_T
#include "edit.h"            // editTask
#include "backend-sqlite3.h" // readTasks
#include "error-codes.h"     // TD_OK
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
  mvaddstr(row++, 0, task->name);
  return row;
}

int
viewListScreen(list_T list)
{
  clear();

  int row = 0;
  mvaddstr(row++, 0, "# Task Tracker");

  for (int i=0; i<list->ntasks; i++)
    row = drawTask(row, 0, list->tasks[i]);

  move(row-1, 0);
  refresh();

  return TD_OK;
}

int
pageHelp(char *filename)
{
  char *pager = getenv("PAGER");
  if (pager == NULL) return -1;

#define MAX_CMD_LEN 256
  char command[MAX_CMD_LEN];
  int n = snprintf(command, MAX_CMD_LEN-1, "%s %s", pager, filename);

  // TODO: check return code
  def_prog_mode(); // save current tty modes
  endwin();

  int status = system(command);
  if (status == -1) {
    return -1; // TODO: return error code
  } else {
    if (WIFEXITED(status) && WEXITSTATUS(status) == 127)
      return -1; // TODO: return error code
      // fatal("Editor returned 127, likely unable to invoke shell");
    // else
      // TODO: check return status of shell
      // printWaitStatus(NULL, status);
  }

  refresh(); // restore save modes, repaint screen

  return TD_OK;
}

int
viewHelpScreen()
{

  char *help =
#include "help.inc"

  char filename[] = "/tmp/help-XXXXXX";

  int fd = mkstemp(filename);
  if (fd == -1) return -1; // TODO: return error code

  if (dprintf(fd, "%s", help) < 0) return -1;
  if (pageHelp(filename) != TD_OK) return -1;

  unlink(filename);
  if (close(fd) == -1) return -1; // TODO: return error code
  
  return TD_OK;
  
}

void
moveDown()
{
  int cur_row, cur_col, max_row, max_col;

  getyx(stdscr, cur_row, cur_col);
  getmaxyx(stdscr, max_row, max_col);

  if (cur_row < max_row) move(cur_row+1, cur_col);
}
    
void
moveUp()
{
  int cur_row, cur_col, max_row, max_col;

  getyx(stdscr, cur_row, cur_col);
  getmaxyx(stdscr, max_row, max_col);

  if (cur_row > 0) move(cur_row-1, cur_col);
}

#define clearStatusLine() do { \
  move(max_row-1, 0);          \
  clrtoeol();                  \
  move(cur_row, cur_col);      \
} while(0);

void 
eventLoop()
{

  list_T list = readTasks(); // TODO: should list be an arg?
  list_T updates = listNew("updates");

  viewListScreen(list);

  char c, answer;
  int cur_row, cur_col, max_row, max_col; 
  int save_row, save_col;
  while ((c = getch())) {

    getyx(stdscr, cur_row, cur_col);
    getmaxyx(stdscr, max_row, max_col);

    clearStatusLine();

    switch (c) {
    case 'e':
      if (cur_row > 0 && cur_row <= list->ntasks) {
        editTask(&list->tasks[cur_row-1]);
        listAddTask(updates, list->tasks[cur_row-1]);
        viewListScreen(list);
      }
      break;

    case 'h':
      if (viewHelpScreen() != TD_OK) {
        endwin();
        fprintf(stderr, "Error opening help\n");
        exit(EXIT_FAILURE);
      }
      break;

    case 'j':
      moveDown();
      break;

    case 'k':
      moveUp();
      break;

    case 'q':
      return;

    case 's':
      save_row = cur_row, save_col = cur_col;
      mvaddstr(max_row-1, 0, "Save changes? (y/n) ");
      refresh();
      answer = getch();
      move(max_row-1, 0);
      clrtoeol();
      if (answer == 'y') {
        if (writeUpdates(updates) == TD_OK)
          // mvaddstr(max_row-1, 0, "Changes successfully saved to backend.");
          addstr("Changes successfully saved to backend.");
        else
          addstr("Unable to save changes to backend.");
      } else
        addstr("Changes not saved to backend.");
      refresh();
      move(save_row, save_col);
      break;

    default:
      break;
    }

  }
}

void
view(int argc, char **argv)
{

  initscr(); // TODO: check return value
  cbreak();   // disable line buffering
  noecho();   // disable echo for getch

  eventLoop();

  endwin(); // TODO: check return value
}

