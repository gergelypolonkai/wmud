-- Okay, so you update this file. But if you create a table, don't forget to
-- update drop.sql, either!

CREATE TABLE directions (
	id integer NOT NULL PRIMARY KEY,
	short_name varchar(2) NOT NULL UNIQUE,
	name varchar(10) NOT NULL UNIQUE
);

CREATE TABLE areas (
	id integer NOT NULL PRIMARY KEY,
	name varchar(50) NOT NULL
);

CREATE TABLE rooms (
	id integer NOT NULL PRIMARY KEY,
	area integer NOT NULL REFERENCES areas(id),
	name varchar(50) NOT NULL,
	distant_description text,
	close_description text NOT NULL,
	UNIQUE (area, name)
);

CREATE TABLE room_exits (
	room_id integer NOT NULL REFERENCES rooms(id),
	direction integer REFERENCES directions(id),
	other_side integer NOT NULL REFERENCES rooms(id),
	PRIMARY KEY (room_id, direction)
);
