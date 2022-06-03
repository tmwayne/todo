//
// -----------------------------------------------------------------------------
// main.c
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
#include <stdlib.h>          // exit, EXIT_SUCCESS, EXIT_FAILURE
#include <string.h>          // strcmp
#include <getopt.h>          // getopt_long
#include "error-functions.h" // fatal
#include "helpers.h"         // hello
#include "todo.h"
#include "view.h"
#include "backend-sqlite3.h"

const char *USAGE = "Usage: %s [OPTIONS...] COMMAND\n";

const char *VERSION = "todo v0.1\n"
  "Copyright (c) 2022 Tyler Wayne\n"
  "Licensed under the Apache License, Version 2.0\n"
  "\n"
  "Written by Tyler Wayne.\n";

const char *HELP = "Usage: %s [OPTIONS...] COMMAND\n"
  "Just do it...\n"
  "\n"
  "Options:\n"
  "  -h, --help                Print this help\n"
  "  -v, --version             Print version info\n";

int 
main(int argc, char **argv)
{

  int option_index = 0;
  struct option longopts[] = {
    {"version",   no_argument,  0,  'V' },
    {"help",      no_argument,  0,  'h' },
    {0}
  };

  while (1) {

    int opt = getopt_long(argc, argv, "hV", longopts, &option_index);

    if (opt == -1) break;

    switch (opt) {
    /*
    case 0:
      if (longopts[option_index].flag) break;
      break;
    */

    case 'h':
      printf(HELP, argv[0]);
      exit(EXIT_SUCCESS);

    case 'V':
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

  // printf("todo: %s\n", argv[optind]);
#define is_arg(x) (strcmp(argv[optind], (x)) == 0)

  if (optind == argc || is_arg("view"))
    view();

  else
    usageErr("Command not recognized\n");

  exit(EXIT_SUCCESS);

}
