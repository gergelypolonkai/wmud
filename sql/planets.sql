CREATE TABLE planets (
	id integer NOT NULL PRIMARY KEY,
	name varchar(40) NOT NULL UNIQUE
);

CREATE TABLE planet_planes (
	planet_id integer NOT NULL REFERENCES planets(id),
	plane_id integer NOT NULL REFERENCES planes(id),
	UNIQUE (planet_id, plane_id)
);

