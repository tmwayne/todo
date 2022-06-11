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
#include "screen.h"

#define clearStatusLine() do { \
  move(max_row-1, 0);          \
  clrtoeol();                  \
  move(cur_row, cur_col);      \
} while(0);


static int
viewTaskScreen(list_T list, task_T task)
{
  int row;
  char c;
  char buf[16]; // holds a 15 digit char

  do {

    clear();
    row = 0;

#define BLANKIFNULL(val) (val) ? (val) : ""

#define ADDVAL(key, val) do {   \
    mvaddstr(row++, 0, (key) ); \
    addstr(": ");               \
    addstr((val));              \
  } while (0);

    for (int i=0; i < taskSize(task); i++)
      ADDVAL(taskKeyInd(task, i), BLANKIFNULL(taskValInd(task, i)));

    refresh();
    c = getch();

    if (c == 'e') {
      if (editTask(list, task) != TD_OK) {
        endwin();
        errExit("Failed editing task");
      }
    }
      
    else return TD_OK;

  } while (1);
}

static int
viewListScreen(screen_T screen, list_T list)
{
  clear();
  
  // TODO: need to check line count
  // TODO: add ability to hide lines

  int row = 0;
  // mvaddstr(row++, 0, "# Task Tracker");

  line_T first_line = screenGetFirstLine(screen);
  line_T line = first_line;

  for (line_T line=first_line; line; line=lineGetNext(line)) {
    char *val;
    int level = lineLevel(line);
    if (level < 0) return -1; // TODO: return error code
    int type = lineType(line);
    switch (type) {
    case LT_STR:
      val = (char *) lineObj(line);
      break;

    case LT_CAT:
      val = catName((cat_T) lineObj(line));
      break;

    case LT_TASK:
      val = taskGet((task_T) lineObj(line), "name");
      break;

    default:
      return -1;
    }

    move(row, 0);
    if (type == LT_CAT) addstr("[");
    else {
      for (int i=level; i>0; i--) 
        if (i == 1) addstr(". ");
        else addstr("  ");
    }

    addstr(val);
    if (type == LT_CAT) addstr("]");

    row++;
  }

  return TD_OK;
}

static int
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

static int
viewHelpScreen()
{

  char *help =
#include "help.inc"

  char filename[] = "/tmp/help-XXXXXX";

  int fd = mkstemp(filename);
  if (fd == -1) return -1; // TODO: return error code

  if (dprintf(fd, "%s", help) < 0) return -1;
  if (close(fd) == -1) return -1; // TODO: return error code

  if (pageHelp(filename) != TD_OK) return -1;
  unlink(filename);
  
  return TD_OK;
}

// TODO: rerender if at end of window but screen isn't exhausted
static int
moveDown(const screen_T screen, line_T *line)
{
  int cur_row, cur_col, max_row, max_col;

  getyx(stdscr, cur_row, cur_col);
  getmaxyx(stdscr, max_row, max_col);

  line_T next = lineGetNext(*line);

  if (cur_row < max_row && next) {
    move(cur_row+1, cur_col);
    *line = next;
  }

  return TD_OK;
}
    
static int
moveUp(const screen_T screen, line_T *line)
{
  int cur_row, cur_col, max_row, max_col;

  getyx(stdscr, cur_row, cur_col);
  getmaxyx(stdscr, max_row, max_col);

  line_T prev = lineGetPrev(*line);

  if (cur_row > 0 && prev) {
    move(cur_row-1, cur_col);
    *line = prev;
  }

  return TD_OK;
}

static void 
eventLoop()
{

  screen_T screen = screenNew();
  list_T list = NULL;
  task_T task;

  readTasks(&list); 
  screenInitialize(screen, list);

  viewListScreen(screen, list);
  line_T line = screenGetFirstLine(screen);
  move(0, 0);
  refresh();

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
      if (lineType(line) == LT_TASK) {
        getyx(stdscr, save_row, save_col);
        if (editTask(list, (task_T) lineObj(line)) != TD_OK) {
          endwin();
          errExit("Failed editing task");
        }
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
      moveDown(screen, &line);
      break;

    // Move cursor up
    case 'k':
      moveUp(screen, &line);
      break;

    // Quit
    case 'q':
      if (listNumUpdates(list) == 0) return;
      else {
        save_row = cur_row; save_col = cur_col;
        mvaddstr(status_row, 0, "Save changes before quitting? (y/n) ");
        refresh();
        answer = getch();
        if (answer != 'y') return;
        else if (writeUpdates(list) == TD_OK) return;
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
      if (listNumUpdates(list) == 0) {
        move(status_row, 0);
        mvaddstr(status_row, 0, "No updates to save.");
        move(save_row, save_col);
        break;
      }
      mvaddstr(status_row, 0, "Save changes? (y/n) ");
      refresh();
      answer = getch();
      move(status_row, 0);
      clrtoeol();
      if (answer == 'y') {
        if (writeUpdates(list) == TD_OK)
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
      if (lineType(line) == LT_TASK) {
        getyx(stdscr, save_row, save_col);
        
        // TODO: check if task was edited
        // TODO: the category of a task can be edited
        // which could cause misalignment between the line and the cursor.
        // check for this
        viewTaskScreen(list, (task_T) lineObj(line));
        redraw = true;
      }
      break;

    default:
      break;

    }

    if (redraw) {
      screenReset(&screen, list);
      viewListScreen(screen, list);
      line = screenGetFirstLine(screen);
      move(0, 0);
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

