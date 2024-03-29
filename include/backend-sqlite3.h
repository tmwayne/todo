// 
// -----------------------------------------------------------------------------
// backend-sqlite3.h
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

#ifndef BACKEND_SQLITE3_INCLUDED
#define BACKEND_SQLITE3_INCLUDED

#include "task.h" // task_T
#include "list.h" // list_T

enum backendReturnCodes {
  BE_DBNOTEXIST     = -1, // database file doesn't exist
  BE_TBLNOTEXIST    = -2, // table doesn't exist
  BE_COLINVALID     = -3, // some list keys aren't in table columns
  BE_ESQLGEN        = -4, // error generating SQL statement
  BE_ESQLPREP       = -5, // error preparing SQL statement with SQLite3
  BE_ESQLBIND       = -6, // error binding SQL variables with SQLite3
  BE_ESQLPROC       = -7, // error processing SQL results with SQLite3
  BE_ETBLNMINVALID  = -8  // invalid table name. Currently only accepts alphanum
                        // and "_" but not a leading number
};

extern int  readTasks(list_T, const char *filename);
extern int  writeUpdates(list_T, const char *filename);
extern int  backendCheck(const list_T, const char *filename);
extern int  backendCreate(list_T, const char *filename);

#endif // BACKEND_SQLITE3_INCLUDED
