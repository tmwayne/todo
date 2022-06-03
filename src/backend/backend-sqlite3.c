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
#include "todo.h"   // task_T


// TODO: use void * to make arguments generic across different backends
void
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

  while ((rc = sqlite3_step(stmt)) != SQLITE_DONE) {

    if (rc == SQLITE_ERROR) {
      fprintf(stderr, "Error executing query: %s\n", sqlite3_errmsg(db));
      sqlite3_close(db);
      exit(EXIT_FAILURE);
    }

    int ncols = sqlite3_column_count(stmt);

    for (int i=0; i<ncols; i++) {
      const unsigned char *val = sqlite3_column_text(stmt, i);
      if (val)
        printf("%s\n", val);
      else
        printf("(null)\n");
    }
    printf("\n");

  }

  sqlite3_finalize(stmt);
  sqlite3_close(db);
}
