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

#ifndef ERRORCODES_INCLUDED
#define ERRORCODES_INCLUDED

// TODO: rename this file return-codes.h

enum generalRCs {
  TD_BUFOVERFLOW  = -2,
  TD_INVALIDARG   = -1, // one of the arguments is invalid
  TD_OK           = 0
};

enum editRCs {
  ET_UNMOD        = 0,  // task wasn't modified
  ET_MOD          = 1   // task was modified
};

enum readerRCs {
  RD_ESYNTAX      = -2, // syntax error
  RD_ENOMEM       = -1  // memory allocation failed
};

enum dataframeRCs {
  DF_INVRECORD    = -5, // record has the wrong number of rows
  DF_CORRUPT      = -4, // values in the dataframe are inconsistent
  DF_ENOMEM       = -3, // memory allocation failed
  DF_OOBARG       = -2, // argument is out-of-bounds
  DF_NULLARG      = -1, // pointer argument is NULL
  DF_OK           = 0
};


#endif // ERRORCODES_INCLUDED
