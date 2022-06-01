// 
// -----------------------------------------------------------------------------
// todo.h
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

#ifndef TODO_INCLUDED
#define TODO_INCLUDED

// TODO: iterate on this data model
typedef struct {
  int  id;
  char *name;
  int  *parent_id;
  char *effort;
  char *file_date; // TODO: change this to a date type
  char *due_date;  // TODO: change this to a date type
} *task_T;

extern void todo();

#endif // TODO_INCLUDED
