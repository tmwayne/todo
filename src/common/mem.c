//
// -----------------------------------------------------------------------------
// mem.c
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

#include <stdlib.h>
#include <stddef.h>
#include "mem.h"

void *
memAlloc(long nbytes) 
{
  if (nbytes <= 0) return (void *) NULL;
  return malloc(nbytes);
}

void *
memCalloc(long count, long nbytes) 
{
  if (count <= 0 || nbytes <= 0) return (void *) NULL;
  return calloc(count, nbytes);
}

void 
memFree(void *ptr) 
{
  if (ptr) free(ptr);
}

void *
memResize(void *ptr, long nbytes) 
{
  if (ptr == NULL || nbytes <= 0) return NULL;
  return realloc(ptr, nbytes);
}


