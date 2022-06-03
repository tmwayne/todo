//
// -----------------------------------------------------------------------------
// backend-sqlite3.c
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

#include <stdio.h>  // fprintf
#include <stdlib.h> // NULL
#include <string.h> // strlen
#include <sqlite3.h>
#include "task.h"   // task_T
#include "error-codes.h"

task_T *
readTasks()
{
  sqlite3 *db;
  sqlite3_stmt *stmt;
  int rc;

  char *filename = "test/data/test-db.sqlite3";
  char *sql = "select * from todo";

  rc = sqlite3_open_v2(
    filename,              // filename
    &db,                   // db handle
    SQLITE_OPEN_READWRITE, // don't create if database doesn't exist
    NULL                   // OS interface for db connection
  );

  if (rc != SQLITE_OK) {
    fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    sqlite3_close(db);
    exit(EXIT_FAILURE);
    return NULL;
  }

  rc = sqlite3_prepare_v2(
    db,                // db handle
    sql,               // sql statement
    strlen(sql)+1,     // maximum length of sql, in bytes (including '\0')
    &stmt,             // out: statement handle
    NULL               // out: point to unused portion of sql
  );

  if (rc != SQLITE_OK) {
    fprintf(stderr, "Failed to fetch data: %s\n", sqlite3_errmsg(db));
    sqlite3_close(db);
    exit(EXIT_FAILURE);
  }

  // We don't know the size of the results.
  // We allocate a small array of tasks to hold them.
  // If we run out of space, we resize the array doubling it's size
  int tasks_len = 16;
  task_T *tasks;
  tasks = calloc(tasks_len, sizeof(*tasks));
  if (tasks == NULL) {
    fprintf(stderr, "Failed to allocate array of tasks\n");
    sqlite3_close(db);
    exit(EXIT_FAILURE);
  }

  int row_ind = 0;

  while ((rc = sqlite3_step(stmt)) != SQLITE_DONE) {

    if (rc == SQLITE_ERROR) {
      fprintf(stderr, "Error executing query: %s\n", sqlite3_errmsg(db));
      sqlite3_close(db);
      exit(EXIT_FAILURE);
    }

    // Skip corrupted entries
    // TODO: do we want to throw an error instead?
    int ncols = sqlite3_column_count(stmt);
    if (ncols != TASK_NCOLS) continue;

    if (row_ind >= tasks_len) {
      tasks_len <<= 1; // double tasks_len
      tasks = realloc(tasks, tasks_len);
      if (tasks == NULL) {
        fprintf(stderr, "Error resizing tasks array\n");
        sqlite3_close(db);
        exit(EXIT_FAILURE);
      }
    }

    char *buf[TASK_NCOLS];
    for (int i=0; i<TASK_NCOLS; i++)
      buf[i] = (char *) sqlite3_column_text(stmt, i);

    task_T task = Task_new();
    if (task == NULL) {
      fprintf(stderr, "Error allocating new task\n");
      sqlite3_close(db);
      exit(EXIT_FAILURE);
    }

    taskFromArray(task, buf);
    tasks[row_ind++] = task;

  }

  sqlite3_finalize(stmt);
  sqlite3_close(db);

  return tasks;
}
