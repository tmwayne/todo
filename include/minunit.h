//
// -----------------------------------------------------------------------------
// minunit.h
// -----------------------------------------------------------------------------
// 
// Copyright (c) 2012 David Siñuela Pastor, siu.4coders@gmail.com
// 
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

#define mu_assert(message, cond) do { \
    if (!(cond)) return message; \
    else return 0; \
  } while (0)

#define mu_run_test(test) do { \
    char* message = test(); tests_run++; \
    if (message) return message; \
  } while (0)

#define mu_run_all(tests) do {\
    for (int i=0; tests[i]; i++) { \
      char *message = tests[i](); \
      tests_run++; \
      if (message) return message; \
    } \
  } while (0)

extern int tests_run;
