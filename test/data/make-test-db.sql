--
----------------------------------------------------------------------
-- make-test-db.sql
----------------------------------------------------------------------
--
-- Copyright (c) 2022 Tyler Wayne
-- 
-- Licensed under the Apache License, Version 2.0 (the "License");
-- you may not use this file except in compliance with the License.
-- You may obtain a copy of the License at
-- 
--     http://www.apache.org/licenses/LICENSE-2.0
-- 
-- Unless required by applicable law or agreed to in writing, software
-- distributed under the License is distributed on an "AS IS" BASIS,
-- WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
-- See the License for the specific language governing permissions and
-- limitations under the License.
--

drop table if exists default_list;

create table default_list (
    id int primary key,
    parent_id int,
    name text not null,
    effort text,
    file_date text,
    due_date text
);

insert into default_list (id, parent_id, name, effort, file_date, due_date)
values
    (2, null, "Write todo", "L", "2022-06-01", "2022-07-01"),
    (7, 2, "celebrate!", "S", "2022-06-01", "2022-07-01"),
    (11, 2, "be productive", "S", "2022-06-03", "2022-08-01");



