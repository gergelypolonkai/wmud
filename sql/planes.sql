-- Okay, so you update this file. But if you create a table, don't forget to
-- update drop.sql, either!

CREATE TABLE planes (
	id integer primary key,
	name varchar(40) not null unique
);

