//
// -----------------------------------------------------------------------------
// import.c
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

#include <stdio.h>             // printf
#include <stdlib.h>            // free
#include <readline/readline.h> // readline
#include "error-functions.h"   // errExit
#include "list.h"              // list_T
#include "backend-delim.h"     // readTasks_delim

void
importTasks(list_T list, char **filename, char *import_filename)
{
  if (!(list && filename)) return;

  // TODO: fix the case when import_filename is NULL so we use stdin, it's hanging

  readTasks_delim(list, import_filename);

  if (!(*filename)) {
    char *line = readline("Backend file doesn't exist. Create one? (y/n) ");

    if (line) {
      switch (line[0]) {
      case 'y':
        *filename = readline("File name: ");

      default:
        free(line);
        break;
      }
    }
  }
}
