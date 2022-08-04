//
// -----------------------------------------------------------------------------
// screen.c
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

#include <stdio.h>
#include <stdlib.h> // free
#include <string.h> // strdup
#include "mem.h"
#include "return-codes.h"
#include "task.h"
#include "list.h"
#include "screen.h"

struct line_T {
  int lineno;
  int level;    // indentation level
  // int hidden;   // should the line be hidden?
  int type;     // type of line (string, category, task)
  void *obj;    // pointer to line object
  line_T llink;
  line_T rlink;
};

// TODO: add ability to increase/decrease offset

screen_T
screenNew()
{
  screen_T screen;
  screen = memCalloc(1, sizeof(*screen));
  return screen;
}

static int
screenAddLine(screen_T screen, const int type, void *obj, const int level, const int lineno)
{

  line_T line;
  line = memCalloc(1, sizeof(*line));
  if (!line) return -1;

  line->lineno = lineno;
  line->level = level;
  line->type = type;
  line->obj = obj;

  screen->nlines++;

  if (!screen->lines) {
    screen->lines = line;
    return TD_OK;
  }

  line_T tail = screen->lines;
  for ( ; tail->rlink; tail=tail->rlink) ;

  tail->rlink = line;
  line->llink = tail;

  return TD_OK;
}

/**
 * This function recursively traverses the task tree for a category
 * and adds tasks and subtasks. It also includes a lineno. We wait
 * to see if a task is non-null, that is the parent task or previous
 * task was not a leaf, before incrementing the line count
 */
static int
screenAddTasks(screen_T screen, const task_T task, const int level, int lineno)
{
  // Skip NULL, completed, or deleted tasks
  if (!task) return lineno; 

  if (strcasecmp(taskGet(task, "status"), "Complete") != 0 &&
      !taskGetFlag(task, TF_DELETE)) {
    lineno++;
    screenAddLine(screen, LT_TASK, task, level, lineno);
  }

  lineno = screenAddTasks(screen, taskGetSubtask(task), level+1, lineno); 
  return screenAddTasks(screen, taskGetNext(task), level, lineno); 
}


int
screenInitialize(screen_T screen, const list_T list)
{
  cat_T cat = NULL;
  int lineno = 0;
  while ((cat = listGetCat(list, cat))) {
    if (catNumOpen(cat) <= 0) continue;

    screenAddLine(screen, LT_CAT, cat, 0, lineno);

    task_T task = catGetTask(cat, NULL);
    if (!task) return -1; // TODO: return error code

    // Increment once to bring it to the current line
    // and a second time to add a blank line
    lineno = screenAddTasks(screen, task, 1, lineno) + 2;
    screen->nlines++; // Blank line
  }

  // Remove the trailing blank line
  screen->nlines--;

  return TD_OK;
}

// TODO: this is not an efficient way to update the screen.
// Is there a better way?
int
screenReset(screen_T *screen, const list_T list)
{
  int offset = 0; // save the offset

  if (screen && *screen) {
    offset = (*screen)->offset;
    screenFree(screen);
  }

  *screen = screenNew();
  (*screen)->offset = offset;
  
  return screenInitialize(*screen, list);
}

line_T
screenGetLine(const screen_T screen, const int lineno)
{
  if (!screen || lineno < 0 || lineno >= screen->nlines) return NULL;

  struct line_T *line = screen->lines;
  for ( ; line->lineno < lineno ; line = line->rlink) ;

  if (line->lineno == lineno) return line;
  else return NULL;
}

line_T
screenGetFirstLine(const screen_T screen)
{
  if (!screen) return NULL;
  else return screen->lines;
}

void
screenFree(screen_T *screen)
{
  if (!(screen && *screen)) return;

  line_T line, next;
  for (line=(*screen)->lines; line; ) {
    next = line->rlink;
    free(line);
    line = next;
  }

  free(*screen);
  *screen = NULL;
}

// -----------------------------------------------------------------------------
// Line
// -----------------------------------------------------------------------------

int
lineNum(const line_T line)
{
  if (!line) return -1;
  else return line->lineno;
}

int
lineType(const line_T line)
{
  if (!line) return TD_INVALIDARG;
  else return line->type;
}

void *
lineObj(const line_T line)
{
  if (!line) return (void *) NULL;
  else return line->obj;
}

int
lineLevel(const line_T line)
{
  if (!line) return TD_INVALIDARG;
  else return line->level;
}
