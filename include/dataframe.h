// 
// -----------------------------------------------------------------------------
// dataframe.h
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

#ifndef DATAFRAME_INCLUDED
#define DATAFRAME_INCLUDED

typedef char *field_T;

typedef struct record {
  int nfields;  // number of fields
  int len;      // length of the fields array
  field_T *fields;
} *record_T;

typedef struct dataframe {
  int nrecords; // number of records
  int nfields;  // number of fields in the header record or first record
  int len;      // length of the records array
  record_T headers;
  // TODO: make this an array of arrays for increased storage efficiency
  record_T *records;
} *dataframe_T;

extern record_T     recordNew();
extern int          recordPush(record_T, const field_T);
extern field_T      recordGet(const record_T, const int);
extern void         recordFree(record_T *);

extern dataframe_T  dataframeNew();
extern int          dataframeSetHeaders(dataframe_T, const record_T);
extern int          dataframePush(dataframe_T, const record_T);
extern void         dataframeFree(dataframe_T *);

#endif // DATAFRAME_INCLUDED
