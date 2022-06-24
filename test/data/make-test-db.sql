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
    id text primary key,
    parent_id text,
    category text,
    name text not null,
    effort text,
    priority text,
    timing text,
    file_date text,
    due_date text,
    status text,
    next_steps text,
    description text,
    keywords text
);

insert into default_list 
values
    (2, null, "Productivity", "Write todo", 
    "L", "P0", "This week", "2022-06-01", "2022-07-01",
    "On track", null, null, null),

    (7, 2, "Productivity", "celebrate!", 
    "S", "P1", "Inbound", "2022-06-01", "2022-07-01",
    "Yet to start", null, null, null),

    (11, null, "Heidi", "Buy Heidi's bday gifts",
    "M", "P0", "This week", "2022-06-05", "2022-06-10",
    "Yet to start", null, null, null),

    (12, 11, "Heidi", "Book lavender fest hotel",
    "XS", "P0", "This week", "2022-06-05", "2022-06-10",
    "Complete", null, null, null);
