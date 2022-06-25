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

#ifndef RETURNCODES_INCLUDED
#define RETURNCODES_INCLUDED

enum generalRCs {
  TD_OK           = 0,
  TD_INVALIDARG   = -1, // one of the arguments is invalid
  TD_BUFOVERFLOW  = -2  // buffer wasn't large enough for text
};

enum editRCs {
  ET_UNMOD        = 0,  // task wasn't modified
  ET_MOD          = 1   // task was modified
};

enum readerRCs {
  RD_ENOMEM       = -1, // memory allocation failed
  RD_ESYNTAX      = -2  // syntax error
};

enum dataframeRCs {
  DF_OK           = 0,
  DF_NULLARG      = -1, // pointer argument is NULL
  DF_OOBARG       = -2, // argument is out-of-bounds
  DF_ENOMEM       = -3, // memory allocation failed
  DF_CORRUPT      = -4, // values in the dataframe are inconsistent
  DF_INVRECORD    = -5 // record has the wrong number of rows
};

enum backendRCs {
  BE_DBNOTEXIST   = -1, // database file doesn't exist
  BE_TBLNOTEXIST  = -2, // table doesn't exist
  BE_COLINVALID   = -3, // some list keys aren't in table columns
  BE_ESQLGEN      = -4, // error generating SQL statement
  BE_ESQLPREP     = -5, // error preparing SQL statement with SQLite3
  BE_ESQLBIND     = -6, // error binding SQL variables with SQLite3
  BE_ESQLPROC     = -7  // error processing SQL results with SQLite3
};

#endif // RETURNCODES_INCLUDED
