INSERT INTO directions (id, short_name, name) VALUES (1, 'n', 'north');
INSERT INTO directions (id, short_name, name) VALUES (2, 'nw', 'northwest');
INSERT INTO directions (id, short_name, name) VALUES (3, 'w', 'west');
INSERT INTO directions (id, short_name, name) VALUES (4, 'sw', 'southwest');
INSERT INTO directions (id, short_name, name) VALUES (5, 's', 'south');
INSERT INTO directions (id, short_name, name) VALUES (6, 'se', 'southeast');
INSERT INTO directions (id, short_name, name) VALUES (7, 'e', 'east');
INSERT INTO directions (id, short_name, name) VALUES (8, 'ne', 'northeast');
INSERT INTO directions (id, short_name, name) VALUES (9, 'u', 'up');
INSERT INTO directions (id, short_name, name) VALUES (10, 'd', 'down');

INSERT INTO planes (id, name) VALUES (1, 'Erodar');
INSERT INTO planes (id, name) VALUES (2, 'Indiar');
INSERT INTO planes (id, name) VALUES (3, 'Zothar');

INSERT INTO planets (id, name) VALUES (1, 'Erodar');

INSERT INTO planet_planes (planet_id, plane_id) VALUES (1, 1);
INSERT INTO planet_planes (planet_id, plane_id) VALUES (1, 2);
INSERT INTO planet_planes (planet_id, plane_id) VALUES (1, 3);

INSERT INTO areas (id, name) VALUES (1, 'Tutorial Area');

INSERT INTO rooms (id, area, name, distant_description, close_description) VALUES (1, 1, 'Tutorial Welcome Area', 'You see a small room with some tables and chairs in it.', 'You are in a small room with some tables and chairs.');
INSERT INTO rooms (id, area, name, distant_description, close_description) VALUES (2, 1, 'Tutorial Fighting Area', 'You see a small arena barely enough for a few people to fight.', 'You are in a small arena. It is barely large enough for few people to fight.');

INSERT INTO room_exits (room_id, direction, other_side) VALUES (1, 1, 2);
INSERT INTO room_exits (room_id, direction, other_side) VALUES (2, 5, 1);
