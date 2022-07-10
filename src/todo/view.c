//
// -----------------------------------------------------------------------------
// view.c
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
#include "backend-delim.h"   // readTasks_delim
#include "return-codes.h"    // TD_OK
#include "view.h"
#include "screen.h"

// TODO: fix cursor so it doesn't reset everytime
// TODO: enable vertical movement of the cursor
static void
viewTaskScreen(list_T list, task_T task)
{
  int row, out;
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

    if (c == 'e') editTask(list, task);
      
    else return;

  } while (1);
}

// TODO: some tasks seem to be hidden. screen might be not initializing correctly
static void
viewListScreen(const screen_T screen, const list_T list)
{
  clear();
  
  // TODO: need to check line count

  int row = 0;

  line_T line;
  char *val;
  int level;
  int type;

  for (int i=0; i < screen->nlines; i++) {

    line = screenGetLine(screen, i);

    // Line is a blank line
    if (line == NULL) {
      type = LT_BLANK;
      val = "";
      level = 0;
    }

    else {
      level = lineLevel(line);
      if (level < 0) 
        errExit("Failed to render list screen: indent level less than 0");
      type = lineType(line);
      switch (type) {
      case LT_CAT:
        val = catName((cat_T) lineObj(line));
        break;

      case LT_TASK:
        val = taskGet((task_T) lineObj(line), "name");
        break;

      default: // ignore unrecognized types
        break;
      }
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
}

static void
pageHelp(char *filename)
{
  char *pager = getenv("PAGER");
  if (pager == NULL) 
    errExit("Failed to open help: PAGER environment variable unset"); 

#define MAX_CMD_LEN 256
  char command[MAX_CMD_LEN];
  int n = snprintf(command, MAX_CMD_LEN-1, "%s %s", pager, filename);

  if (def_prog_mode() == ERR)
    errExit("Failed to open pager: terminal process not created"); 

  endwin();

  int status = system(command);
  if (status == -1)
    sysErrExit("Failed to open help: pager process could not be created");

  else if (WIFEXITED(status) && WEXITSTATUS(status) == 127)
    errExit("Editor returned 127, likely unable to invoke shell");
  // We don't know which editor is used so we dont' check the exit
  // code of the editor command, which would be 128+n where n
  // is the exit code of the editor

  refresh(); // restore save modes, repaint screen
}

static void
viewHelpScreen()
{

  char *help =
#include "help.inc"

  char filename[] = "/tmp/help-XXXXXX";

  int fd = mkstemp(filename);
  if (fd == -1) 
    errExit("Failed to open help: unable to make temp file");

  if (dprintf(fd, "%s", help) < 0) 
    errExit("Failed to open help: unable to copy help text to temp file");

  if (close(fd) == -1) 
    errExit("Failed to open help: unable to close temp file");

  pageHelp(filename);
  unlink(filename);
}

// TODO: rerender if at end of window but screen isn't exhausted
static line_T
moveDown(const screen_T screen)
{
  int cur_row, cur_col, max_row, max_col;

  getyx(stdscr, cur_row, cur_col);
  getmaxyx(stdscr, max_row, max_col);

  // TODO: check and account for offset
  if (cur_row < max_row && cur_row < screen->nlines-1) {
    cur_row++;
    move(cur_row, cur_col);
  }

  return screenGetLine(screen, cur_row);
}
    
static line_T
moveUp(const screen_T screen)
{
  int cur_row, cur_col;

  getyx(stdscr, cur_row, cur_col);

  // TODO: check and account for offset
  if (cur_row > 0) {
    cur_row--;
    move(cur_row, cur_col);
  }

  return screenGetLine(screen, cur_row);
}

#define clearStatusLine() do { \
  move(max_row-1, 0);          \
  clrtoeol();                  \
  move(cur_row, cur_col);      \
} while (0)

#define statusMessage(str) do { \
  move(status_row, 0);          \
  clrtoeol();                   \
  addstr((str));                \
  refresh();                    \
} while (0)


static void 
eventLoop(list_T list, const char *filename)
{
  if (!list) return;

  screen_T screen = screenNew();
  task_T task;

  screenInitialize(screen, list);
  viewListScreen(screen, list);
  line_T line = screenGetLine(screen, 0);

  move(0, 0);
  refresh();

  char c;
  int rc;
  int cur_row, cur_col, max_row, max_col; 
  int status_row;
  bool redraw = false;
  while ((c = getch())) {

    getyx(stdscr, cur_row, cur_col);
    getmaxyx(stdscr, max_row, max_col);
    status_row = max_row - 1;

    clearStatusLine();

    switch (c) {

    // TODO: whenever we get input, we could receive a KEY_RESIZE. handle it
    // TODO: add command to fold categories
    // TODO: create an undo option (this will require substantial work)

    case 'a': // Add task
      if (lineType(line) == LT_CAT || lineType(line) == LT_TASK) {
        addTask(list, line);
        redraw = true;
      }
      break;
    
    case 'e': // Edit task
      if (lineType(line) == LT_TASK) {
        if (editTask(list, (task_T) lineObj(line)) == ET_MOD)
          redraw = true;
      }
      break;

    case 'h': // View help screen
      viewHelpScreen();
      break;

    case 'j': // Move cursor down
      line = moveDown(screen);
      break;

    case 'k': // Move cursor up
      line = moveUp(screen);
      break;

    case 'q': // Quit
      if (listNumUpdates(list) == 0) return;
      else if (filename) {
        statusMessage("Save changes before quitting? (y/n) ");
        if (getch() != 'y') return;

        int has_backend = 1;
        rc = backendCheck(list, filename);
        if (rc == BE_DBNOTEXIST || rc == BE_TBLNOTEXIST) {
          has_backend = 0;
          statusMessage("Initialize backend? (y/n) ");
          if (getch() == 'y' && backendCreate(list, filename) == TD_OK)
            has_backend = 1;
        }

        if (has_backend && writeUpdates(list, filename) == TD_OK) return;

        statusMessage("Unable to save changes. Quit anyway? (y/n) ");
        if (getch() == 'y') return;
        else move(cur_row, cur_col);
      }
      break;
            
    case 's': // Save changes
      if (listNumUpdates(list) == 0) {
        statusMessage("No updates to save.");
        move(cur_row, cur_col);
        break;
      }

      if (filename) {
        rc = backendCheck(list, filename);
        if (rc == BE_DBNOTEXIST || rc == BE_TBLNOTEXIST) {
          statusMessage("Initialize backend? (y/n) ");
          if (getch() == 'y') 
            if (backendCreate(list, filename) != TD_OK) {
              statusMessage("Unable to create backend.");
              break;
            }
          else {
            statusMessage("Changes not saved to backend.");
            break;
          }
        }

        statusMessage("Save changes? (y/n) ");
        if (getch() == 'y' && writeUpdates(list, filename) == TD_OK)
          statusMessage("Changes successfully saved to backend.");
        else
          statusMessage("Changes not saved to backend.");

        move(cur_row, cur_col);
      }
      break;

    case 'v': // View task
      if (lineType(line) == LT_TASK) {
        viewTaskScreen(list, (task_T) lineObj(line));
        redraw = true;
      }
      break;

    case 'x': // Mark task as complete
      if (lineType(line) == LT_TASK) {
        task = (task_T) lineObj(line);
        statusMessage("Mark as complete? (y/n) ");
        if (getch() != 'y') statusMessage("Task left unchanged.");
        else do {
            if (taskGetSubtask(task)) {
              statusMessage("Mark all subtasks as complete? (y/n) ");
              if (getch() != 'y') {
                statusMessage("Task left unchanged.");
                break;
              }
            }

            if (markComplete(list, task) == TD_OK) redraw = true;
            else statusMessage("Unable to mark as complete.");
        } while (0);
        move(cur_row, cur_col);
      }
      break;

    default:
      break;

    }

    if (redraw) {
      screenReset(&screen, list);
      viewListScreen(screen, list);
      line = screenGetLine(screen, cur_row);
      move(cur_row, cur_col);
      redraw = false;
    }

  }
}

static void
endwinAtExit()
{
  endwin();
}

void
view(list_T list, const char *filename)
{
  if (!(list && filename)) return;
  
  // Register this exit handler so that we can exit
  // the program within functions when errors occur
  atexit(endwinAtExit);

  initscr();
  cbreak();
  noecho();

  eventLoop(list, filename);
}

