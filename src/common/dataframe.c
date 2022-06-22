//
// -----------------------------------------------------------------------------
// dataframe.c
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

#include <stdlib.h> // calloc, realloc
#include <string.h> // strdup
#include "dataframe.h"
#include "error-codes.h"

record_T
recordNew()
{
  record_T record;
  record = calloc(1, sizeof(*record));
  if (!record) return NULL;

  record->len = 8;
  record->fields = calloc(record->len, sizeof(field_T));
  if (!record->fields) {
    free(record);
    return NULL;
  }

  return record;
}

int
recordPush(record_T record, const field_T val)
{
  if (!(record && val)) return DF_NULLARG;

  if (record->nfields >= record->len) {
    record->len <<= 1;
    field_T *fields = realloc(record->fields, record->len * sizeof(field_T));
    if (!fields) return DF_ENOMEM;
    else record->fields = fields;
  }

  record->fields[record->nfields++] = strdup(val);

  return DF_OK;
}

field_T
recordGet(const record_T record, const int ind)
{
  if (!(record && ind < record->nfields)) return NULL;
  return record->fields[ind];
}

void
recordFree(record_T *record)
{
  if (!(record && *record)) return;
  for (int i=0; i < (*record)->nfields; i++)
    free((*record)->fields[i]);

  free(*record);
  *record = NULL;
}


dataframe_T
dataframeNew()
{
  dataframe_T df;
  df = calloc(1, sizeof(*df));
  if (!df) return NULL;
  
  df->len = 1024;

  df->records = calloc(df->len, sizeof(df->records));
  if (!df->records) {
    free(df);
    return NULL;
  }

  return df;
}

int
dataframeSetHeaders(dataframe_T df, const record_T headers)
{
  if (!(df && headers)) return DF_NULLARG;
  
  df->headers = headers;
  df->nfields = headers->nfields;

  return DF_OK;
}

int
dataframePush(dataframe_T df, const record_T record)
{
  int nfields = df->nfields;

  if (df->headers || df->nrecords) {
    if (nfields != record->nfields) return DF_INVRECORD;
  } else
    nfields = df->nfields = record->nfields;

  if (df->nrecords >= df->len) {
    df->len <<= 1;
    record_T *records = realloc(df->records, df->len * sizeof(record_T));
    if (!records) return DF_ENOMEM;
    else df->records = records;
  }

  df->records[df->nrecords++] = record;

  return DF_OK;
}

void
dataframeFree(dataframe_T *df)
{
  if (!(df && *df)) return;
  
  if ((*df)->headers) recordFree(&(*df)->headers);

  for (int i=0; i < (*df)->nrecords; i++)
    recordFree(&(*df)->records[i]);

  free(*df);
  *df = NULL;
}
