CREATE TABLE menu (
	id integer UNIQUE PRIMARY KEY,                    -- ID of the menu item
	menuchar char(1) UNIQUE NOT NULL,                 -- The choice character of the menu item. It will be highlighted in the menu text if found, prepended to the menu text in braces otherwise
	need_active_char boolean NOT NULL DEFAULT FALSE,  -- Menu item is displayed only if the player has a character selected
	placement integer UNIQUE NOT NULL,                -- The menu is ordered based on this field
	display_text varchar(70) NOT NULL UNIQUE,         -- The text to be displayed (see also the menuchar field)
	fnctn varchar(40) NOT NULL                        -- The in-game function to execute
);

