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
#include "error-codes.h"     // TD_OK
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

static void
viewListScreen(const screen_T screen, const list_T list)
{
  clear();
  
  // TODO: need to check line count
  // TODO: add ability to hide lines

  int row = 0;

  line_T first_line = screenGetFirstLine(screen);
  line_T line = first_line;

  for (line_T line=first_line; line; line=lineGetNext(line)) {
    char *val;
    int level = lineLevel(line);
    if (level < 0) 
      errExit("Failed to render list screen: indent level less than 0");
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

    default: // ignore unrecognized types
      break;
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

#define clearStatusLine() do { \
  move(max_row-1, 0);          \
  clrtoeol();                  \
  move(cur_row, cur_col);      \
} while (0);

#define statusMessage(str) do { \
  move(status_row, 0);          \
  clrtoeol();                   \
  addstr((str));                \
  refresh();                    \
} while (0);


static void 
eventLoop(char *listname, char *filename)
{
  screen_T screen = screenNew();
  list_T list = listNew(listname);
  task_T task;

  readTasks(list, filename);
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

    case 'a': // Add task
      if (lineType(line) == LT_CAT || lineType(line) == LT_TASK) {
        addTask(list, line);
        redraw = true;
      }
      break;
    
    case 'e': // Edit task
      if (lineType(line) == LT_TASK) {
        // getyx(stdscr, save_row, save_col);
        if (editTask(list, (task_T) lineObj(line)) == ET_MOD)
          redraw = true;
      }
      break;

    case 'h': // View help screen
      viewHelpScreen();
      break;

    case 'j': // Move cursor down
      moveDown(screen, &line);
      break;

    case 'k': // Move cursor up
      moveUp(screen, &line);
      break;

    case 'q': // Quit
      if (listNumUpdates(list) == 0) return;
      else {
        getyx(stdscr, save_row, save_col);
        statusMessage("Save changes before quitting? (y/n) ");
        answer = getch();
        if (answer != 'y') return;
        else if (writeUpdates(list, filename) == TD_OK) return;
        else {
          statusMessage("Unable to save changes. Quit anyway? (y/n) ");
          answer = getch();
          if (answer == 'y') return;
          else move(save_row, save_col);
        }
      }
      break;
            
    case 's': // Save changes
      getyx(stdscr, save_row, save_col);
      if (listNumUpdates(list) == 0) {
        statusMessage("No updates to save.");
        move(save_row, save_col);
        break;
      }
      statusMessage("Save changes? (y/n) ");
      answer = getch();
      if (answer == 'y') {
        writeUpdates(list, filename);
        statusMessage("Changes successfully saved to backend.");
      } else
        statusMessage("Changes not saved to backend.");

      move(save_row, save_col);
      break;

    // TODO: create an undo option (this will require substantial work)

    case 'v': // View task
      if (lineType(line) == LT_TASK) {
        getyx(stdscr, save_row, save_col);
        
        viewTaskScreen(list, (task_T) lineObj(line));
        redraw = true;
      }
      break;

    case 'x': // Mark task as complete
      if (lineType(line) == LT_TASK) {
        getyx(stdscr, save_row, save_col);
        task = (task_T) lineObj(line);
        statusMessage("Mark as complete? (y/n) ");
        answer = getch();
        if (answer != 'y') {
          statusMessage("Task left unchanged.");
        } else do {
            if (taskGetSubtask(task)) {
              statusMessage("Mark all subtasks as complete? (y/n) ");
              answer = getch();
              if (answer != 'y') {
                statusMessage("Task left unchanged.");
                break;
              }
            }

            if (markComplete(list, task) == TD_OK) redraw = true;
            else statusMessage("Unable to mark as complete.");
        } while (0);
        move(save_row, save_col);
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

static void
endwinAtExit()
{
  endwin();
}

void
view(char *listname, char *filename)
{
  // Register this exit handler so that we can exit
  // the program within functions when errors occur
  atexit(endwinAtExit);

  initscr();
  cbreak();
  noecho();

  eventLoop(listname, filename);
}

