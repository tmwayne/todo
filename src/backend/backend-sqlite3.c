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

#include <stdio.h>           // fprintf
#include <stdlib.h>          // NULL
#include <string.h>          // strlen
#include <sqlite3.h>
#include "task.h"            // task_T
#include "error-codes.h"
#include "error-functions.h" // errExit

// TODO: parameterize readTasks (filename, db, table, etc)

list_T
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

  list_T list = listNew("default");
  if (list == NULL) {
    fprintf(stderr, "Failed to create list\n");
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

    char *buf[TASK_NCOLS];
    for (int i=0; i<TASK_NCOLS; i++)
      buf[i] = (char *) sqlite3_column_text(stmt, i);

    task_T task = taskNew();
    if (task == NULL) {
      fprintf(stderr, "Error allocating new task\n");
      sqlite3_close(db);
      exit(EXIT_FAILURE);
    }

    taskFromArray(task, buf);
    listAddTask(list, task);

  }

  sqlite3_finalize(stmt);
  sqlite3_close(db);

  return list;
}

static int
updateTask(task_T task)
{
  sqlite3 *db;
  sqlite3_stmt *stmt;
  int rc;

  char *filename = "test/data/test-db.sqlite3";

  char *sql = "\
  update todo         \
  set parent_id = ?2, \
    name = ?3,        \
    effort = ?4,      \
    file_date = ?5,   \
    due_date = ?6     \
  where id = ?1       \
  ";

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
    return -1;
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

  // TODO: check if values are NULL

  sqlite3_bind_int(stmt, 1, task->id);

  if (task->parent_id == 0)
    sqlite3_bind_null(stmt, 2);
  else
    sqlite3_bind_int(stmt, 2, task->parent_id);

  if (task->name == NULL)
    sqlite3_bind_null(stmt, 3);
  else
    sqlite3_bind_text(stmt, 3, task->name, -1, SQLITE_STATIC);

  if (task->effort == NULL)
    sqlite3_bind_null(stmt, 4);
  else 
    sqlite3_bind_text(stmt, 4, task->effort, -1, SQLITE_STATIC);

  if (task->file_date == NULL)
    sqlite3_bind_null(stmt, 5);
  else
    sqlite3_bind_text(stmt, 5, task->file_date, -1, SQLITE_STATIC);

  if (task->due_date == NULL)
    sqlite3_bind_null(stmt, 6);
  else
    sqlite3_bind_text(stmt, 6, task->due_date, -1, SQLITE_STATIC);

  if ((rc = sqlite3_step(stmt)) != SQLITE_DONE) {
    fprintf(stderr, "Failed to update database: %s\n", sqlite3_errmsg(db));
    sqlite3_close(db);
    exit(EXIT_FAILURE);
  }

  sqlite3_finalize(stmt);
  sqlite3_close(db);

  return TD_OK;

}

int
writeUpdates(list_T updates)
{

  if (updates == NULL) return -1; // TODO: return error code

  for (int i=0; i < updates->ntasks; i++)
    if (updateTask(updates->tasks[i]) != TD_OK) return -1;

  return TD_OK;

}

int
dumpTasks()
{

  char command[] = "sqlite3 -header test/data/test-db.sqlite3 'select * from todo'";

  int status = system(command);
  if (status == -1) {
    errExit("system");
  }
  // TODO: finish error checking system here

  return TD_OK;
}
