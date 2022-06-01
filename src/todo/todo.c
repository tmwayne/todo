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

#include <stdio.h>           // printf
#include <stdlib.h>          // calloc
#include <ncurses.h>         // initscr, cbreak, noecho, getch, endwin
#include "error-functions.h" // errMsg
#include "todo.h"

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
createDummyTask()
{
  task_T task;
  task = calloc(1, sizeof(*task));
  if (task == NULL) errMsg("calloc");

  task->id        = 1;
  task->name      = "Build todo";
  task->effort    = "L";
  task->file_date = "2022-05-31";
  task->due_date  = "2022-07-01";

  return task;
}
  

void
todo()
{
  task_T task = createDummyTask();

  initscr(); // TODO: check return value

  cbreak();   // disable line buffering
  noecho();   // disable echo for getch

  addstr("Time to do it!");

  getch();

  endwin(); // TODO: check return value

}

