CREATE TABLE preferences (
	id integer NOT NULL PRIMARY KEY,
	internal_name varchar(20) NOT NULL UNIQUE,
	description varchar(240) NOT NULL,
);

INSERT INTO preferences (id, internal_name, description) VALUES (1,  'BRIEF',     'Room descriptions won''t normally be shown');
INSERT INTO preferences (id, internal_name, description) VALUES (2,  'COMPACT',   'No extra CRLF pair before prompt');
INSERT INTO preferences (id, internal_name, description) VALUES (3,  'DEAF',      'Can''t hear shouts');
INSERT INTO preferences (id, internal_name, description) VALUES (4,  'NO_TELL',   'Can''t receive tells');
INSERT INTO preferences (id, internal_name, description) VALUES (5,  'D_HP',      'Display Hit Points in prompt');
INSERT INTO preferences (id, internal_name, description) VALUES (6,  'D_MANA',    'Display Mana Points in prompt');
INSERT INTO preferences (id, internal_name, description) VALUES (7,  'D_MOVE',    'Display Move Points in prompt');
INSERT INTO preferences (id, internal_name, description) VALUES (8,  'AUTOEX',    'Display exits in a room');
INSERT INTO preferences (id, internal_name, description) VALUES (9,  'NO_HASS',   'Aggressive mobs won''t attack');
INSERT INTO preferences (id, internal_name, description) VALUES (10, 'QUEST',     'Player is on a quest');
INSERT INTO preferences (id, internal_name, description) VALUES (11, 'SUMN',      'Can be summoned');
INSERT INTO preferences (id, internal_name, description) VALUES (12, 'NO_REP',    'No repetition of comm commands');
INSERT INTO preferences (id, internal_name, description) VALUES (13, 'LIGHT',     'Holy Light enabled');
INSERT INTO preferences (id, internal_name, description) VALUES (14, 'C1',        'Color (low bit)');
INSERT INTO preferences (id, internal_name, description) VALUES (15, 'C2',        'Color (high bit)');
INSERT INTO preferences (id, internal_name, description) VALUES (16, 'NO_WIZ',    'Can''t hear wizline');
INSERT INTO preferences (id, internal_name, description) VALUES (17, 'L1',        'Online system log (low bit)');
INSERT INTO preferences (id, internal_name, description) VALUES (18, 'L2',        'Online system log (high bit)');
INSERT INTO preferences (id, internal_name, description) VALUES (19, 'NO_AUC',    'Can''t hear auction channel');
INSERT INTO preferences (id, internal_name, description) VALUES (20, 'NO_GOS',    'Can''t hear gossip channel');
INSERT INTO preferences (id, internal_name, description) VALUES (21, 'NO_GTZ',    'Can''t hear grats channel');
INSERT INTO preferences (id, internal_name, description) VALUES (22, 'RMFLG',     'Can see room flags');
INSERT INTO preferences (id, internal_name, description) VALUES (23, 'DISPAUTO',  'Show prompt HP, MP, MV when < 30%');
INSERT INTO preferences (id, internal_name, description) VALUES (23, 'AUTOLOOT',  'Automatically loot corpses on kill');
INSERT INTO preferences (id, internal_name, description) VALUES (24, 'AUTODRAIN', 'Automatically drain corpses on kill');

CREATE TABLE players (
	id integer NOT NULL, -- Used to be idnum!
	name varchar(20) UNIQUE NOT NULL PRIMARY KEY,
	description varchar(240),
	title varchar(80),
	sex int2 NOT NULL,
	class int2 NOT NULL,
	race int2 NOT NULL,
	level int2 NOT NULL DEFAULT 1,
	hometown unsigned big int NOT NULL,
	birth datetime NOT NULL,
	played unsigned big int NOT NULL DEFAULT 0,
	weight integer NOT NULL,
	height integer NOT NULL,
	pwd varchar(10) NOT NULL,
	alignment integer NOT NULL,
long /*bitvector_t*/ act;    /* act flag for NPC's; player flag for PC's */
long /*bitvector_t*/ affected_by; /* Bitvector for spells/skills affected by */
sh_int apply_saving_throw[5]; /* Saving throw (Bonuses)              */

byte skills[MAX_SKILLS+1];   /* array of skills plus skill 0         */
bool talks[MAX_TONGUE];      /* PC s Tongues 0 for NPC               */
	wimp integer,
	freeze_level int2,
	invis_level int2,
	load_room unsigned big int,
long /*bitvector_t*/ pref;   /* preference flags for PC's.           */
	bad_pws integer NOT NULL DEFAULT 0,
	drunk_level int2,
	hunger_level int2,
	thirst_level int2,
	spells_to_learn integer,

	strength int2,
	strength_add int2,
	intelligence int2,
	wisdom int2,
	dexterity int2,
	constitution int2,
	charisma int2,

	cur_mana integer,
	max_mana integer,
	cur_hit integer,
	max_hit integer,
	cur_move integer,
	max_move integer,
	armour int2,
	gold bigint,
	bank bigint,
	experience bigint,
	hitroll int2,
	damroll int2,

struct affected_type affected[MAX_AFFECT];

	last_logon datetime,
	last_host varchar(100)
);

CREATE TABLE player_preferences (
	player_id integer NOT NULL REFERENCES players(id),
	preference_id integer NOT NULL REFERENCES preferences(id)
);

