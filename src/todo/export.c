//
// -----------------------------------------------------------------------------
// export.c
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

#include <stdio.h>           // snprintf
#include <stdlib.h>          // system, WIFEXITED, WEXITSTATUS
#include "error-functions.h" // errExit, sysErrExit

// TODO: is there a non-arbitrary magic number we can use?
#define MAX_SQL_LEN 2048

void
exportTasks(char *listname, const char *filename)
{
  // TODO: what error handling do we want here?
  if (!(listname && filename)) return;

  char command[MAX_SQL_LEN];

  // TODO: protect against SQL injection here
  int size = snprintf(command, MAX_SQL_LEN, "sqlite3 -header %s 'select * from %s'",
    filename, listname);

  if (size >= MAX_SQL_LEN)
    errExit("Failed to export tasks: invalid list or filename");

  int status = system(command);
  if (status == -1)
    sysErrExit("Failed to export tasks: sqlite3 process could not be created");

  else if (WIFEXITED(status) && WEXITSTATUS(status) == 127)
    errExit("Failed to export tasks: Editor returned 127, likely unable to invoke shell");

  else if (WEXITSTATUS(status) > 128)
    errExit("Failed to export tasks: sqlite3 returned an error");
}
