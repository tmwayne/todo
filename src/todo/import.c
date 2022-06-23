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

#include <stdio.h>           // printf, getchar, fgets
#include <stdlib.h>          // calloc
#include <limits.h>          // PATH_MAX
#include <string.h>          // strlen
#include "error-functions.h" // errExit
#include "list.h"            // list_T
#include "backend-delim.h"   // readTasks_delim

// TODO: there are issues here reading in the input
void
importTasks(list_T list, char **filename, char *import_filename)
{
  if (!(list && filename)) return;

  // TODO: fix the case when import_filename is NULL so we use stdin, it's hanging
  // TODO: add merge existing and import tasks

  readTasks_delim(list, import_filename);

  if (!(*filename)) {
    printf("Backend file doesn't exist. Create one? (y/n) ");
    // add 2 to PATH_MAX for potential newline and null-terminator
    char *input;
    size_t n;
    ssize_t nread;
    nread = getline(&input, &n, stdin);
    if (nread == -1)
      sysErrExit("Failed to read answer");
    char answer = input[0];
    // TODO: free the input
    // free(input);
    input = NULL;

    switch (answer) {
    case 'y':
      printf("File name: ");
      nread = getline(filename, &n, stdin);
      if (nread == -1)
        sysErrExit("Failed to read filename");

      if ((*filename)[strlen(*filename)-1] == '\n')
        (*filename)[strlen(*filename)-1] = '\0';
      break;
    }
  }
}
