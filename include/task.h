// 
// -----------------------------------------------------------------------------
// task.h
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

#ifndef TASK_INCLUDED
#define TASK_INCLUDED

#define TASK_NCOLS 6

typedef struct {
  int id;
  int parent_id;
  char *name;
  char *effort;
  char *file_date; 
  char *due_date; 
} *task_T;

extern task_T Task_new();
extern void Task_free(task_T *);

extern int taskFromArray(task_T, char **);

#endif // TASK_INCLUDED
