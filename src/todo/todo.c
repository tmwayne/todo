//
// -----------------------------------------------------------------------------
// todo.c
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

#include <stdio.h>           // printf, fprintf
#include <stdlib.h>          // exit, EXIT_SUCCESS, EXIT_FAILURE
#include <string.h>          // strcmp
#include <getopt.h>          // getopt_long
#include "error-functions.h" // usageErr
#include "task.h"            // task_T
#include "view.h"            // view
#include "backend-sqlite3.h" // dumpTasks

const char *USAGE = "Usage: %s [OPTIONS...] COMMAND\n";

const char *VERSION = "\
todo v0.1                                       \n\
Copyright (c) 2022 Tyler Wayne                  \n\
Licensed under the Apache License, Version 2.0  \n\
                                                \n\
Written by Tyler Wayne.                         \n\
";

// TODO: have action specific flags

const char *HELP = "\
Usage: %s [OPTIONS...] COMMAND                              \n\
Manage todo lists                                           \n\
                                                            \n\
Options:                                                    \n\
  -f, --filename=NAME       Load todo list from NAME        \n\
  -h, --help                Print this help                 \n\
  -l, --listname=NAME       Load todo list NAME             \n\
  -s, --sep=SEP             Import using SEP as separator   \n\
  -v, --version             Print version info              \n\
                                                            \n\
Commands:                                                   \n\
  create    Create a new todo list                          \n\
  dump      Dump todo list to stdout in tabular form        \n\
  import    Import tasks from delimited file                \n\
  view      View todo lists and make edits. (default)       \n\
                                                            \n\
Run '%s COMMAND --help' for more information on a command.  \n\
";

int 
main(int argc, char **argv)
{

  char *filename  = "test/data/test-db.sqlite3";
  char *listname  = "default_list";
  char  sep       = ',';

  int option_index = 0;
  struct option longopts[] = {
  // name         has_arg             flag  val
    {"filename",  required_argument,  0,    'f'},
    {"help",      no_argument,        0,    'h'},
    {"listname",  required_argument,  0,    'l'},
    {"sep",       required_argument,  0,    's'},
    {"version",   no_argument,        0,    'V'},
    {0}
  };

  while (1) {

    int opt = getopt_long(argc, argv, "f:hl:V", longopts, &option_index);

    if (opt == -1) break;

    switch (opt) {
    /*
    case 0:
      if (longopts[option_index].flag) 
        break;
      break;
    */

    case 'f': // filename
      filename = strdup(optarg);
      break;

    case 'h': // help
      printf(HELP, argv[0], argv[0]);
      exit(EXIT_SUCCESS);

    case 'l': // listname
      listname = strdup(optarg);
      break;

    case 's': // sep
      sep = *optarg;
      break;

    case 'V': // version
      printf("%s", VERSION);
      exit(EXIT_SUCCESS);

    case '?':
    case ':': // unrecognized argument
      usageErr("Try '%s --help' for more information.\n", argv[0]);
    
    default: 
      break;
    }
  }

  if (optind > argc) usageErr(USAGE, argv[0]);

#define is_arg(x) (strcmp(argv[optind], (x)) == 0)

  if (optind == argc || is_arg("view"))
    view(listname, filename);

  // else if (is_arg("create"))
    // createList();

  else if (is_arg("dump"))
    dumpTasks(listname, filename);

  else if (is_arg("import")) {
    optind++;
    if (optind == argc)
      usageErr("Import requires a file\n");
    printf("Separator is %c for file %s\n", sep, argv[optind]);
  }

  else if (is_arg("help")) {
    printf(HELP, argv[0], argv[0]);
    exit(EXIT_SUCCESS);
  }

  else
    usageErr("Command not recognized\n");

  exit(EXIT_SUCCESS);

}
