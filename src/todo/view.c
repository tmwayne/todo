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
#include <curses.h>          // initscr, cbreak, noecho, getch, endwin
#include <string.h>          // strdup
#include <time.h>            // time
#include <stdbool.h>         // true, false
#include "error-functions.h" // errMsg
#include "task.h"            // task_T
#include "edit.h"            // editTask
#include "backend-sqlite3.h" // readTasks
#include "error-codes.h"     // TD_OK
#include "view.h"

#define clearStatusLine() do { \
  move(max_row-1, 0);          \
  clrtoeol();                  \
  move(cur_row, cur_col);      \
} while(0);

int
viewTaskScreen(task_T *task, list_T updates)
{
  int row;
  char c;
  char buf[16]; // holds a 15 digit char

  do {

    clear();
    row = 0;

    // TODO: check that there are enough lines on screen
    snprintf(buf, 16, "%d", (*task)->id);
    mvaddstr(row++, 0, "id: ");
    addstr(buf);

    snprintf(buf, 16, "%d", (*task)->parent_id);
    mvaddstr(row++, 0, "parent_id: ");
    addstr(buf);

    mvaddstr(row++, 0, "name: ");
    addstr((*task)->name);

    mvaddstr(row++, 0, "effort: ");
    addstr((*task)->effort);

    mvaddstr(row++, 0, "file_date: ");
    addstr((*task)->file_date);

    mvaddstr(row++, 0, "due_date: ");
    addstr((*task)->due_date);

    refresh();
    c = getch();

    if (c == 'e') {
      editTask(task);
      listAddTask(updates, *task);
    }
      
    else return TD_OK;

  } while (1);
}

int
viewListScreen(list_T list)
{
  clear();

  int row = 0;
  mvaddstr(row++, 0, "# Task Tracker");

  for (int i=0; i<list->ntasks; i++, row++)
    mvaddstr(row, 0, list->tasks[i]->name);

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

void 
eventLoop()
{

  list_T list = readTasks(); // TODO: should list be an arg?

  // TODO: make updates a hash table so that per session only
  // one update per task is made
  list_T updates = listNew("updates");

  viewListScreen(list);

  char c, answer;
  int cur_row, cur_col, max_row, max_col; 
  int save_row, save_col;
  int status_row;
  bool redraw = false;
  while ((c = getch())) {

    getyx(stdscr, cur_row, cur_col);
    getmaxyx(stdscr, max_row, max_col);
    status_row = max_row - 1;

    clearStatusLine();

    switch (c) {
    
    // Edit task
    case 'e':
      if (cur_row > 0 && cur_row <= list->ntasks) {
        editTask(&list->tasks[cur_row-1]);
        listAddTask(updates, list->tasks[cur_row-1]);
        redraw = true;
      }
      break;

    // View help screen
    case 'h':
      if (viewHelpScreen() != TD_OK) {
        endwin();
        fprintf(stderr, "Error opening help\n");
        exit(EXIT_FAILURE);
      }
      break;

    // Move cursor down
    case 'j':
      moveDown();
      break;

    // Move cursor up
    case 'k':
      moveUp();
      break;

    // Quick
    case 'q':
      if (updates->ntasks == 0) return;
      else {
        save_row = cur_row; save_col = cur_col;
        mvaddstr(status_row, 0, "Save changes before quitting? (y/n) ");
        refresh();
        answer = getch();
        if (answer != 'y') return;
        else if (writeUpdates(updates) == TD_OK) return;
        else {
          move(status_row, 0);
          clrtoeol();
          addstr("Unable to save changes. Quit anyway? (y/n) ");
          refresh();
          answer = getch();
          if (answer == 'y') return;
          else move(save_row, save_col);
        }
      }
      break;
            
    // Save changes
    case 's':
      save_row = cur_row; save_col = cur_col;
      mvaddstr(status_row, 0, "Save changes? (y/n) ");
      refresh();
      answer = getch();
      move(status_row, 0);
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

    // View task
    case 'v':
      save_row = cur_row; save_col = cur_col;
      if (cur_row > 0 && cur_row <= list->ntasks)
        viewTaskScreen(&list->tasks[cur_row-1], updates);
      move(save_row, save_col);
      redraw = true;
      break;

    default:
      break;
    }

    if (redraw) {
      viewListScreen(list);
      redraw = false;
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

