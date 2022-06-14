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
#include <string.h>          // strlen, strcasecmp
#include <stdbool.h>         // false
#include <sqlite3.h>
#include "task.h"
#include "list.h"
#include "error-codes.h"
#include "error-functions.h" // errExit

#ifdef TESTING
#define sqlErr(fmt, ...) return -1;
#else
#define sqlErr(fmt, ...) do {               \
    sqlite3_close(db);                      \
    errExit(fmt, ##__VA_ARGS__); \
  } while (0);
#endif

#define MAX_SQL_LEN 2048

// TODO: check that parent_id / category combinations are valid
int
readTasks(list_T list, char *filename)
{
  if (!list) return TD_INVALIDARG;

  sqlite3 *db;
  sqlite3_stmt *stmt;
  int rc;

  // TODO: need to guard against SQL injection here
  char sql[MAX_SQL_LEN];
  snprintf(sql, MAX_SQL_LEN, "select * from %s", listName(list));

  rc = sqlite3_open_v2(
    filename,              // filename
    &db,                   // db handle
    SQLITE_OPEN_READWRITE, // don't create if database doesn't exist
    NULL                   // OS interface for db connection
  );

  if (rc != SQLITE_OK) 
    sqlErr("Can't open database: %s", sqlite3_errmsg(db));

  rc = sqlite3_prepare_v2(
    db,                    // db handle
    sql,                   // sql statement
    strlen(sql)+1,         // maximum length of sql, in bytes (including '\0')
    &stmt,                 // out: statement handle
    NULL                   // out: point to unused portion of sql
  );

  if (rc != SQLITE_OK) 
    sqlErr("Unable to prepare SQL for fetching tasks: %s", sqlite3_errmsg(db));

  int ncols = sqlite3_column_count(stmt);

  for (int i=0; i < ncols; i++)
    listAddKey(list, sqlite3_column_name(stmt, i));

  while ((rc = sqlite3_step(stmt)) != SQLITE_DONE) {

    if (rc == SQLITE_ERROR)
      sqlErr("Unable to fetch tasks: %s", sqlite3_errmsg(db));

    task_T task = taskNew();
    if (task == NULL)
      sqlErr("Failed to allocate new task");

    for (int i=0; i<ncols; i++)
      taskSet(task, sqlite3_column_name(stmt, i), 
        (char *) sqlite3_column_text(stmt, i));

    if (taskCheckKeys(task) != TD_OK) return -1; // TODO: return error code

    if (strcasecmp(taskGet(task, "status"), "Complete") != 0)
      listSetTask(list, task);

  }

  sqlite3_finalize(stmt);
  sqlite3_close(db);

  return TD_OK;
}

static void
updateTask(list_T list, task_T task, char *filename)
{
  sqlite3 *db;
  sqlite3_stmt *stmt;
  int rc;

  if (!(list && task))
    errExit("Failed to write changes: null pointer passed as argument");

  // TODO: need to guard against SQL injection here
  // TODO: prepare this query dynamically
  // TODO: add remaining columns
  char sql[MAX_SQL_LEN];
  snprintf(sql, MAX_SQL_LEN,
    "update %s            \
    set                   \
      parent_id   = ?2,   \
      category    = ?3,   \
      name        = ?4,   \
      status      = ?5    \
      effort      = ?6,   \
      priority    = ?7,   \
    where id = ?1", 
    listName(list));

  rc = sqlite3_open_v2(
    filename,              // filename
    &db,                   // db handle
    SQLITE_OPEN_READWRITE, // don't create if database doesn't exist
    NULL                   // OS interface for db connection
  );

  if (rc != SQLITE_OK)
    sqlErr("Unable to open database: %s", sqlite3_errmsg(db));

  rc = sqlite3_prepare_v2(
    db,                    // db handle
    sql,                   // sql statement
    strlen(sql)+1,         // maximum length of sql, in bytes (including '\0')
    &stmt,                 // out: statement handle
    NULL                   // out: point to unused portion of sql
  );

  if (rc != SQLITE_OK) 
    sqlErr("Unable to prepare SQL for update tasks: %s", sqlite3_errmsg(db));

#define BIND_TEXT(ind, val) \
  sqlite3_bind_text(stmt, (ind), (val), -1, SQLITE_STATIC);

  BIND_TEXT(1, taskGet(task, "id")); // should never be NULL
  BIND_TEXT(2, taskGet(task, "parent_id"));
  BIND_TEXT(3, taskGet(task, "category"));
  BIND_TEXT(4, taskGet(task, "name"));
  BIND_TEXT(5, taskGet(task, "status"));
  BIND_TEXT(6, taskGet(task, "effort"));
  BIND_TEXT(7, taskGet(task, "priority"));

  if ((rc = sqlite3_step(stmt)) != SQLITE_DONE)
    sqlErr("Failed to update database: %s", sqlite3_errmsg(db));

  sqlite3_finalize(stmt);
  sqlite3_close(db);
}

static void
writeNewTask(list_T list, task_T task, char *filename)
{
  sqlite3 *db;
  sqlite3_stmt *stmt;
  int rc;

  if (!(list && task))
    errExit("Failed to write changes: null pointer passed as argument");

  // TODO: need to guard against SQL injection here
  // TODO: prepare this query dynamically
  char sql[MAX_SQL_LEN];
  snprintf(sql, MAX_SQL_LEN,
    "insert into %s (id, parent_id, category, name, status, effort, priority) \
    values (?1, ?2, ?3, ?4, ?5, ?6, ?7)",
    listName(list));

  rc = sqlite3_open_v2(
    filename,              // filename
    &db,                   // db handle
    SQLITE_OPEN_READWRITE, // don't create if database doesn't exist
    NULL                   // OS interface for db connection
  );

  if (rc != SQLITE_OK)
    sqlErr("Unable to open database: %s", sqlite3_errmsg(db));

  rc = sqlite3_prepare_v2(
    db,                    // db handle
    sql,                   // sql statement
    strlen(sql)+1,         // maximum length of sql, in bytes (including '\0')
    &stmt,                 // out: statement handle
    NULL                   // out: point to unused portion of sql
  );

  if (rc != SQLITE_OK) 
    sqlErr("Unable to prepare SQL for update tasks: %s", sqlite3_errmsg(db));

#define BIND_TEXT(ind, val) \
  sqlite3_bind_text(stmt, (ind), (val), -1, SQLITE_STATIC);

  BIND_TEXT(1, taskGet(task, "id")); // should never be NULL
  BIND_TEXT(2, taskGet(task, "parent_id"));
  BIND_TEXT(3, taskGet(task, "category"));
  BIND_TEXT(4, taskGet(task, "name"));
  BIND_TEXT(5, taskGet(task, "status"));
  BIND_TEXT(6, taskGet(task, "effort"));
  BIND_TEXT(7, taskGet(task, "priority"));

  if ((rc = sqlite3_step(stmt)) != SQLITE_DONE)
    sqlErr("Failed to add task to database: %s", sqlite3_errmsg(db));

  sqlite3_finalize(stmt);
  sqlite3_close(db);
}

int
writeUpdates(list_T list, char *filename)
{
  task_T *updates = listGetUpdates(list);

  if (!updates)
    errExit("Failed to write updates: null pointer passed as argument"); 

  // TODO: if there is an error, this will do a partial write.
  // See if we can rollback if there's an error.
  for (int i=0; updates[i]; i++) {
    if (taskGetFlag(updates[i], TF_NEW)) writeNewTask(list, updates[i], filename);
    else updateTask(list, updates[i], filename);
  }

  free(updates);
  listClearUpdates(list);

  return TD_OK;
}

int
dumpTasks(char *listname, char *filename)
{
  char command[MAX_SQL_LEN];

  // TODO: protect against SQL injection here
  int size = snprintf(command, MAX_SQL_LEN, "sqlite3 -header %s 'select * from %s'",
    filename, listname);

  if (size >= MAX_SQL_LEN)
    errExit("Failed to dump tasks: command is longer than buffer");

  int status = system(command);
  if (status == -1)
    sysErrExit("Failed to dump tasks: sqlite3 process could not be created");

  else if (WIFEXITED(status) && WEXITSTATUS(status) == 127)
    errExit("Failed to dump tasks: Editor returned 127, likely unable to invoke shell");

  else if (WEXITSTATUS(status) > 128)
    errExit("Failed to dump tasks: sqlite3 returned an error");

  return TD_OK;
}
