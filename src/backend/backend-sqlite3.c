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

#ifdef TESTING
#define sqlErr(fmt, ...) return -1;
#else
#define sqlErr(fmt, ...) do {               \
    sqlite3_close(db);                      \
    errExit(fmt, ##__VA_ARGS__); \
  } while (0);
#endif

#define BUF_LEN 512
#define MAX_SQL_LEN 2048
#define FILENAME "test/data/test-db.sqlite3"

int
readTasks(list_T *list)
{
  // TODO: make the list name a parameter
  if (*list == NULL) *list = listNew("default_list");

  sqlite3 *db;
  sqlite3_stmt *stmt;
  int rc;

  // TODO: need to guard against SQL injection here
  char sql[BUF_LEN];
  snprintf(sql, BUF_LEN, "select * from %s", (*list)->name);

  rc = sqlite3_open_v2(
    FILENAME,              // filename
    &db,                   // db handle
    SQLITE_OPEN_READWRITE, // don't create if database doesn't exist
    NULL                   // OS interface for db connection
  );

  if (rc != SQLITE_OK) 
    sqlErr("Can't open database: %s", sqlite3_errmsg(db));

  rc = sqlite3_prepare_v2(
    db,                // db handle
    sql,               // sql statement
    strlen(sql)+1,     // maximum length of sql, in bytes (including '\0')
    &stmt,             // out: statement handle
    NULL               // out: point to unused portion of sql
  );

  if (rc != SQLITE_OK) 
    sqlErr("Unable to prepare SQL for fetching tasks: %s", sqlite3_errmsg(db));

  int row_ind = 0;

#define TASK_NCOLS 13
  while ((rc = sqlite3_step(stmt)) != SQLITE_DONE) {

    if (rc == SQLITE_ERROR)
      sqlErr("Unable to fetch tasks: %s", sqlite3_errmsg(db));

    // Skip corrupted entries
    // TODO: do we want to throw an error instead?
    int ncols = sqlite3_column_count(stmt);
    if (ncols != TASK_NCOLS) continue;

    char *buf[TASK_NCOLS];
    for (int i=0; i<TASK_NCOLS; i++)
      buf[i] = (char *) sqlite3_column_text(stmt, i);

    task_T task = taskNew();
    if (task == NULL)
      sqlErr("Failed to allocate new task");

    taskSet(task, "id", buf[0]);
    taskSet(task, "parent_id", buf[1]);
    taskSet(task, "category", buf[2]);
    taskSet(task, "name", buf[3]);
    taskSet(task, "effort", buf[4]);
    taskSet(task, "priority", buf[5]);
    listAddTask(*list, task);

  }

  sqlite3_finalize(stmt);
  sqlite3_close(db);

  return TD_OK;
}

static int
updateTask(char *list_name, task_T task)
{
  sqlite3 *db;
  sqlite3_stmt *stmt;
  int rc;

  // TODO: need to guard against SQL injection here
  // TODO: prepare this query dynamically
  char sql[MAX_SQL_LEN];
  snprintf(sql, MAX_SQL_LEN,
    "update %s          \
    set                 \
      parent_id   = ?2,   \
      category    = ?3,   \
      name        = ?4,   \
      effort      = ?5,   \
      priority    = ?6    \
    where id = ?1", 
    list_name);

  rc = sqlite3_open_v2(
    FILENAME,              // filename
    &db,                   // db handle
    SQLITE_OPEN_READWRITE, // don't create if database doesn't exist
    NULL                   // OS interface for db connection
  );

  if (rc != SQLITE_OK)
    sqlErr("Unable to open database: %s", sqlite3_errmsg(db));

  rc = sqlite3_prepare_v2(
    db,                // db handle
    sql,               // sql statement
    strlen(sql)+1,     // maximum length of sql, in bytes (including '\0')
    &stmt,             // out: statement handle
    NULL               // out: point to unused portion of sql
  );

  if (rc != SQLITE_OK) 
    sqlErr("Unable to prepare SQL for update tasks: %s", sqlite3_errmsg(db));

  // Need to check if the value before binding it to the SQL statement

#define BIND_INT(ind, val) do {                               \
  if ((val) == 0)                                             \
    sqlite3_bind_null(stmt, (ind));                           \
  else                                                        \
    sqlite3_bind_int(stmt, (ind), (val));                     \
  } while (0);

#define BIND_TEXT(ind, val) \
  sqlite3_bind_text(stmt, (ind), (val), -1, SQLITE_STATIC);

  BIND_TEXT(1, taskGet(task, "id")); // should never be NULL
  BIND_TEXT(2, taskGet(task, "parent_id"));
  BIND_TEXT(3, taskGet(task, "category"));
  BIND_TEXT(4, taskGet(task, "name"));
  BIND_TEXT(5, taskGet(task, "effort"));
  BIND_TEXT(6, taskGet(task, "priority"));

  if ((rc = sqlite3_step(stmt)) != SQLITE_DONE)
    sqlErr("Failed to update database: %s", sqlite3_errmsg(db));

  sqlite3_finalize(stmt);
  sqlite3_close(db);

  return TD_OK;

}

int
writeUpdates(list_T updates)
{
  if (updates == NULL) return -1; // TODO: return error code

  for (int i=0; i < updates->ntasks; i++)
    if (updateTask(updates->name, updates->tasks[i]) != TD_OK) return -1;

  updates->ntasks = 0;

  return TD_OK;

}

int
dumpTasks()
{

  // char command[BUF_LEN];

  char command[] = "sqlite3 -header test/data/test-db.sqlite3 'select * from default_list'";

  int status = system(command);
  if (status == -1) {
    errExit("system");
  }
  // TODO: finish error checking system here

  return TD_OK;
}
