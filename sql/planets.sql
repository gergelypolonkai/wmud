-- Okay, so you update this file. But if you create a table, don't forget to
-- update drop.sql, either!

CREATE TABLE planets (
	id integer NOT NULL PRIMARY KEY,
	name varchar(40) NOT NULL UNIQUE
);

CREATE TABLE planet_planes (
	planet_id integer NOT NULL REFERENCES planets(id),
	plane_id integer NOT NULL REFERENCES planes(id),
	UNIQUE (planet_id, plane_id)
);

