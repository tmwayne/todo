//
// -----------------------------------------------------------------------------
// screen.c
// -----------------------------------------------------------------------------
//
// Description
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
#include "mem.h"
#include "task.h"
#include "error-codes.h"
#include "screen.h"

struct line_T {
  int level;    // indentation level
  int hidden;   // should the line be hidden?
  int type;     // type of line (string, category, task)
  void *obj;    // pointer to line object
  line_T llink;
  line_T rlink;
};

struct screen_T {
  int nlines;
  line_T lines;
};

screen_T
screenNew()
{
  screen_T screen;
  screen = memCalloc(1, sizeof(*screen));
  return screen;
}

int
screenAddLine(screen_T screen, int type, void *obj, int level)
{
  line_T line;
  line = memCalloc(1, sizeof(*line));
  if (!line) return -1;

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

int
screenAddTasks(screen_T screen, task_T task, int level)
{
  if (!task) return TD_OK;

  screenAddLine(screen, LT_TASK, task, level);

  screenAddTasks(screen, taskGetSubtask(task), level+1);
  screenAddTasks(screen, taskGetNext(task), level);

  return TD_OK;
}


int
screenInitialize(screen_T screen, const list_T list)
{
  cat_T cat = NULL;
  while ((cat = listGetCat(list, cat))) {
    screenAddLine(screen, LT_CAT, cat, 0); // TODO: check return code
    screenAddTasks(screen, catGetTask(cat, NULL), 1); // TODO: check return code
  }

  return TD_OK;
}

int
screenReset(screen_T *screen, const list_T list)
{
  screenFree(screen);
  screen_T new = screenNew();

  screenInitialize(new, list);
  *screen = new;

  return TD_OK;
}

line_T
screenGetFirstLine(const screen_T screen)
{
  if (!screen) return NULL;
  else return screen->lines;
}

int
screenFree(screen_T *screen)
{
  line_T line, next;
  for (line=(*screen)->lines; line; ) {
    next = line->rlink;
    free(line);
    line = next;
  }

  free(*screen);
  *screen = NULL;

  return TD_OK;
}

// -----------------------------------------------------------------------------
// Line
// -----------------------------------------------------------------------------

int
lineType(const line_T line)
{
  if (!line) return 0;
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
  if (!line) return -1;
  else return line->level;
}

line_T
lineGetNext(const line_T line)
{
  if (!line) return NULL;
  else return line->rlink;
}

line_T
lineGetPrev(const line_T line)
{
  if (!line) return NULL;
  else return line->llink;
}

int
lineIsHidden(const line_T line)
{
  if (!line) return -1;
  else return line->hidden;
}
