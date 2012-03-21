CREATE TABLE players (
	id integer primary key,
	login varchar(50) not null unique,
	password varchar(50),
	email varchar(150) not null unique
);

