#include <string.h>

#include "conf.h"
#include "sysdep.h"
#include "structs.h"
#include "interpreter.h"
#include "utils.h"

const char *race_abbrevs[] = {
	"Hao",
	"Utn",
	"Dua",
	"\n"
};

const char *pc_race_types[] = {
	"Haoon",
	"Utnir",
	"Duaron",
	"\n"
};

const char *race_menu = "\r\n"
"Select race:\r\n"
"( 1) Haoon\r\n"
"( 2) Utnir\r\n"
"( 3) Duaron\r\n";

int
parse_race(char *arg)
{
	if (strcmp(arg, "1") == 0)
	{
		return RACE_HAOON;
	}
	else if (strcmp(arg, "2") == 0)
	{
		return RACE_UTNIR;
	}
	else if (strcmp(arg, "3") == 0)
	{
		return RACE_DUARON;
	}

	return RACE_UNDEFINED;
}

long
find_race_bitvector(char arg)
{
	arg = LOWER(arg);

	switch (arg)
	{
		case '1':
			return (1 << 0);
			break;
		case '2':
			return (1 << 1);
			break;
		case '3':
			return (1 << 2);
			break;
		default:
			return 0;
			break;
	}
}

