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
#include <string.h>          // strlen, strcasecmp, strncpy
#include <stdbool.h>         // false
#include <ctype.h>           // isalpha, isalnum
#include <sqlite3.h>
#include "task.h"
#include "list.h"
#include "backend-sqlite3.h"
#include "return-codes.h"
#include "error-functions.h" // errExit

#ifdef TESTING
#define sqlErr(fmt, ...) return -1;
#else
#define sqlErr(fmt, ...) do {               \
    sqlite3_close(db);                      \
    errExit(fmt, ##__VA_ARGS__); \
  } while (0)
#endif

// TODO: is there a non-arbitrary magic number we can use?
#define MAX_SQL_LEN 2048

// TODO: make sure there is error detection for all sqlite3 calls

static int
isValidTableName(const char *tableName)
{
  if (!tableName) return BE_ETBLNMINVALID;
  
  char c = *tableName++;

  // Can only start with [_a-zA-Z]
  if (!(isalpha(c) || c == '_')) return BE_ETBLNMINVALID;

  // Can only include [_0-9a-zA-Z]
  while ((c = *tableName++)) 
    if (!(isalnum(c) || c == '_')) return BE_ETBLNMINVALID;

  return TD_OK;
}

// -----------------------------------------------------------------------------
// Template
// -----------------------------------------------------------------------------

static int
runSQL(const char *filename, list_T list, task_T task,
  int genSQL(const list_T, const task_T, char *, const size_t),
  int bindSQL(sqlite3_stmt *, sqlite3 *, const list_T, const task_T),
  int processSQL(sqlite3_stmt *, sqlite3 *, list_T, task_T))
{
  if (!list) return TD_INVALIDARG;

  int rc = isValidTableName(listName(list));
  if (rc != TD_OK) return rc;

  sqlite3 *db;
  sqlite3_stmt *stmt;

  char sql[MAX_SQL_LEN];
  
  if (genSQL(list, task, sql, MAX_SQL_LEN) != TD_OK)
    return BE_ESQLGEN;

  rc = sqlite3_open_v2(
    filename,              // filename
    &db,                   // db handle
    SQLITE_OPEN_READWRITE, // don't create if database doesn't exist
    NULL                   // OS interface for db connection
  );

  if (rc != SQLITE_OK) 
    return BE_DBNOTEXIST;

  rc = sqlite3_prepare_v2(
    db,                    // db handle
    sql,                   // sql statement
    strlen(sql)+1,         // maximum length of sql, in bytes (including '\0')
    &stmt,                 // out: statement handle
    NULL                   // out: point to unused portion of sql
  );

  if (rc != SQLITE_OK) 
    return BE_ESQLPREP;

  if (bindSQL)
    if (bindSQL(stmt, db, list, task) != TD_OK)
      return BE_ESQLBIND;

  if (processSQL(stmt, db, list, task) != TD_OK)
    return BE_ESQLPROC;

  sqlite3_finalize(stmt);
  sqlite3_close(db);

  return TD_OK;
}

static int
processNoResultSQL(sqlite3_stmt *stmt, sqlite3 *db, list_T list, task_T task)
{
  int rc = sqlite3_step(stmt);

  // We might use this to throw away the results after checking 
  // the existence of a table. Therefore SQLITE_ROW is an acceptable
  // return value
  if (!(rc == SQLITE_DONE || rc == SQLITE_ROW))
    return BE_ESQLPROC;

  return TD_OK;
}

// -----------------------------------------------------------------------------
// Read
// -----------------------------------------------------------------------------

static int
genReadSQL(const list_T list, const task_T unused, char *buf, const size_t len)
{
  if (snprintf(buf, len, "select * from %s", listName(list)) >= len)
    return TD_BUFOVERFLOW;

  return TD_OK;
}

// TODO: check that parent_id / category combinations are valid
// TODO: abstract the query portions
static int
processReadSQL(sqlite3_stmt *stmt, sqlite3 *db, list_T list, task_T task)
{
  int rc;
  int ncols = sqlite3_column_count(stmt);

  for (int i=0; i < ncols; i++)
    listAddKey(list, sqlite3_column_name(stmt, i));

  while ((rc = sqlite3_step(stmt)) != SQLITE_DONE) {

    if (rc == SQLITE_ERROR)
      // TODO: return error code instead of exiting
      sqlErr("Unable to fetch tasks: %s", sqlite3_errmsg(db));

    task_T task = taskNew();
    if (task == NULL)
      sqlErr("Failed to allocate new task");

    for (int i=0; i<ncols; i++)
      taskSet(task, sqlite3_column_name(stmt, i), 
        (char *) sqlite3_column_text(stmt, i));

    if (taskCheckKeys(task) != TD_OK) 
      return BE_ESQLPROC;

    if (strcasecmp(taskGet(task, "status"), "Complete") != 0)
      listSetTask(list, task);

  }
  
  return TD_OK;
}

int
readTasks(list_T list, const char *filename)
{
  return runSQL(filename, list, NULL, 
    genReadSQL, NULL, processReadSQL);
}

// -----------------------------------------------------------------------------
// Check Backend
// -----------------------------------------------------------------------------
//
  // TODO: if table exists, check if columns are consistent and if so, ask
  // user if they want to sync

// TODO: add a check for column consistency
int
backendCheck(const list_T list, const char *filename)
{
  if (!filename) return TD_INVALIDARG;
  return runSQL(filename, list, NULL, genReadSQL, NULL, processNoResultSQL);
}

// -----------------------------------------------------------------------------
// Update
// -----------------------------------------------------------------------------

/**
 * This function constructs a query like the following but with
 * a variable number of columns.
 *
 * "update table_name"
 * "set parent_id = ?2, category = ?3, name = ?4, status = ?5"
 * "where id = ?6"
 */
static int
genUpdateSQL(const list_T list, const task_T task, char *buf, const size_t len)
{
  if (!(buf && len)) return TD_INVALIDARG;

  char tmp[len];

  char *text = "update %s set ";
  snprintf(buf, len, text, listName(list));

  // snprintf will ensure there is a null-terminating byte with this len
  strncpy(tmp, buf, len);

  //      buf (comma) key = ?X
  text = "%s%s%s=?%d";
  char comma[2] = " ";
  char *key;

  int i;
  for (i=0; i < taskSize(task); i++) {
  
    key = elemKey(taskElemInd(task, i));
    if (strcmp(key, "id") == 0) continue;

    snprintf(buf, len, text, tmp, comma, key, i+1);

    strncpy(tmp, buf, len);
    comma[0] = ',';

  }

  text = "%s where id=?%d";

  // Any of the above can cause an overflow, but
  // because we concatenate the strings cumulatively,
  // we only have to check the last one
  if (snprintf(buf, len, text, tmp, i+1) >= len)
    return TD_BUFOVERFLOW;

  return TD_OK;
  
}

static int
bindUpdateSQL(sqlite3_stmt *stmt, sqlite3 *db, const list_T list, const task_T task)
{
  int i;
  elem_T elem;
  for (i=0; i < taskSize(task); i++) {
    elem = taskElemInd(task, i);
    if (strcmp(elemKey(elem), "id") == 0) continue;
    if (sqlite3_bind_text(stmt, i+1, elemVal(elem), -1, SQLITE_STATIC) != SQLITE_OK)
      return BE_ESQLBIND;
  }

  if (sqlite3_bind_text(stmt, i+1, taskGet(task, "id"), -1, SQLITE_STATIC) != SQLITE_OK)
    return BE_ESQLBIND;

  return TD_OK;
}

int
updateTask(list_T list, task_T task, const char *filename)
{
  return runSQL(filename, list, task, 
    genUpdateSQL, bindUpdateSQL, processNoResultSQL);
}

// -----------------------------------------------------------------------------
// Insert
// -----------------------------------------------------------------------------

/**
 * This function constructs a query like the following but with
 * a variable number of columns.
 *
 * "insert into %s (id, parent_id, category, name, status)"
 * "values (?1, ?2, ?3, ?4, ?5)"
 */
static int
genInsertSQL(const list_T list, const task_T task, char *buf, const size_t len)
{
  if (!(buf && len)) return TD_INVALIDARG;

  elem_T elem;
  char tmp[len];

  char *text = "insert into %s (";
  snprintf(buf, len, text, listName(list));

  // snprintf will ensure there is a null-terminating byte with this len
  strncpy(tmp, buf, len);

  char comma[2] = "";
  char *key;


  // Construct this part: id, parent_id, category, name, status, ...
  int i;
  for (i=0; i < taskSize(task); i++) {
  
    key = elemKey(taskElemInd(task, i));
    snprintf(buf, len, "%s%s%s", tmp, comma, key);

    strncpy(tmp, buf, len);
    comma[0] = ',';

  }

  text = "%s) values (";
  snprintf(buf, len, text, tmp);
  strncpy(tmp, buf, len);

// Construct this part: ?1, ?2, ?3, ?4, ?5, ...
  comma[0] = ' ';
  for (int j=0; j<i; j++) {
    snprintf(buf, len, "%s%s?%d", tmp, comma, j+1);
    strncpy(tmp, buf, len);
    comma[0] = ',';
  }

  // Any of the above can cause an overflow, but
  // because we concatenate the strings cumulatively,
  // we only have to check the last one
  if (snprintf(buf, len, "%s)", tmp) >= len)
    return TD_BUFOVERFLOW;

  return TD_OK;
  
}

static int
bindInsertSQL(sqlite3_stmt *stmt, sqlite3 *db, const list_T list, const task_T task)
{
  for (int i=0; i < taskSize(task); i++)
    sqlite3_bind_text(stmt, i+1, taskValInd(task, i), -1, SQLITE_STATIC);

  return TD_OK;
}

int
writeNewTask(list_T list, task_T task, const char *filename)
{
  return runSQL(filename, list, task, 
    genInsertSQL, bindInsertSQL, processNoResultSQL);
}

// -----------------------------------------------------------------------------
// Delete
// -----------------------------------------------------------------------------

static int
genDeleteSQL(const list_T list, const task_T unused, char *buf, const size_t len)
{
  if (snprintf(buf, len, "delete from %s where id = ?", listName(list)) >= len)
    return TD_BUFOVERFLOW;

  return TD_OK;
}

static int
bindDeleteSQL(sqlite3_stmt *stmt, sqlite3 *db, const list_T list, const task_T task)
{
  if (sqlite3_bind_text(stmt, 1, taskGet(task, "id"), -1, SQLITE_STATIC) != SQLITE_OK)
    return BE_ESQLBIND;

  return TD_OK;
}

static int
deleteTask(list_T list, task_T task, const char *filename)
{
  return runSQL(filename, list, task, 
    genDeleteSQL, bindDeleteSQL, processNoResultSQL);
}

int
writeUpdates(list_T list, const char *filename)
{
  task_T *updates = listGetUpdates(list);

  if (!updates)
    errExit("Failed to write updates: null pointer passed as argument"); 

  // TODO: if there is an error, this will do a partial write.
  // See if we can rollback if there's an error.
  for (int i=0; updates[i]; i++) {
    // TODO: check return code for each of these operations
    if (taskGetFlag(updates[i], TF_NEW)) writeNewTask(list, updates[i], filename);
    else if (taskGetFlag(updates[i], TF_DELETE)) deleteTask(list, updates[i], filename);
    else updateTask(list, updates[i], filename);
  }

  free(updates);
  listClearUpdates(list);

  return TD_OK;
}

// -----------------------------------------------------------------------------
// Create
// -----------------------------------------------------------------------------

static int 
genCreateSQL(const list_T list, const task_T unused, char *buf, const size_t len)
{
  if (!(buf && len)) return TD_INVALIDARG;

  elem_T elem;
  char tmp[len];

  char *text ="create table %s (";
  // snprintf will ensure there is a null-terminating byte with this len
  snprintf(buf, len, text, listName(list));
  strncpy(tmp, buf, len);

  // format: "tmp (comma) col (extra-args)"
  text = "%s %s %s text %s";
  char *extra_args;
  char comma[2] = "";
  char **keys = listGetKeys(list);

  for (int i=0; i < listNumKeys(list); i++) {
    if (strcmp(keys[i], "id") == 0)
      extra_args = "primary key";
    else if (strcmp(keys[i], "name") == 0)
      extra_args = "not null";
    else
      extra_args = "";

    snprintf(buf, len, text, tmp, comma, keys[i], extra_args);
    strncpy(tmp, buf, len);
    comma[0] = ',';
  }

  text = "%s)";

  // Any of the above can cause an overflow, but
  // because we concatenate the strings cumulatively,
  // we only have to check the last one
  if (snprintf(buf, len, text, tmp) >= len)
    return TD_BUFOVERFLOW;

  return TD_OK;
}

int
backendCreate(list_T list, const char *filename)
{
  if (!(list && filename)) return TD_INVALIDARG;
  sqlite3 *db;

  int rc = sqlite3_open_v2(
    filename,
    &db,
    SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
    NULL
  );

  if (rc != SQLITE_OK) 
    return BE_DBNOTEXIST; // TODO: make this a more general db open error

  return runSQL(filename, list, NULL, genCreateSQL, NULL, processNoResultSQL);
}
