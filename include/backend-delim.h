// 
// -----------------------------------------------------------------------------
// backend-delim.h
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

#ifndef BACKEND_DELIM_INCLUDED
#define BACKEND_DELIM_INCLUDED

#include "task.h" // task_T
#include "list.h" // list_T

extern void readTasks_delim(list_T, const char *filename);

#endif // BACKEND_DELIM_INCLUDED
