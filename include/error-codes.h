// 
// -----------------------------------------------------------------------------
// error-codes.h
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

#ifndef ERROR_CODES_INCLUDED
#define ERROR_CODES_INCLUDED

enum generalReturnCode {
  TD_INVALIDARG = -1, // one of the arguments is invalid
  TD_OK = 0
};

enum editReturnCodes {
  ET_UNMOD = 0,       // task wasn't modified
  ET_MOD = 1          // task was modified
};

#endif // ERROR_CODES_INCLUDED
