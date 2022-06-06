//
// -----------------------------------------------------------------------------
// test-prototype.c
// -----------------------------------------------------------------------------
//
// Tyler Wayne (c) 2022
//

#include <stdio.h>
#include "minunit.h"
#include "error-codes.h"
#include "task.h"            // list_T
#include "backend-sqlite3.h" // readTasks

int tests_run = 0;

static char 
*test_prototype() 
{
  int condition = 1;
  mu_assert("Condition is not true", condition);
}

static char
*test_readTasks()
{
  list_T list = NULL;
  mu_assert("Database doesn't exist", readTasks(&list) == TD_OK);
}

static char * 
run_all_tests() 
{
  char *(*all_tests[])() = {
    // test_prototype,
    test_readTasks,
    NULL
  };

  // Returns message of first failing test
  mu_run_all(all_tests);
    
  return 0;
}

int 
main(int argc, char** argv) 
{
  char* result = run_all_tests();

  if (result != 0) printf("%s\n", result);
  else printf("ALL TESTS PASSED\n");

  printf("Tests run: %d\n", tests_run);

  return result != 0;
}
