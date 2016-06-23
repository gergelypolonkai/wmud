#include <glib.h>
#include <string.h>

#include "ansi.h"

typedef struct _wmud_tag_to_ansi {
	gchar *open_tag;
	gint open_tag_length;
	gchar *close_tag;
	gint close_tag_length;
	gchar *ansi_sequence;
} wmud_tag_to_ansi;

static wmud_tag_to_ansi tags[] = {
	{ "<bold>",                 6,  "</bold>",      7,  ANSI_BOLD },
	{ "<underline>",            11, "</underline>", 12, ANSI_UNDERLINE },
	{ "<normal>",               8,  "</normal>",    9,  ANSI_NORMAL },
	{ "<black>",                7,  "</black>",     8,  ANSI_COLOR_BLACK },
	{ "<red>",                  5,  "</red>",       6,  ANSI_COLOR_RED },
	{ "<green>",                7,  "</green>",     8,  ANSI_COLOR_GREEN },
	{ "<yellow>",               8,  "</yellow>",    9,  ANSI_COLOR_YELLOW },
	{ "<blue>",                 6,  "</blue>",      7,  ANSI_COLOR_BLUE },
	{ "<magenta>",              9,  "</magenta>",   10, ANSI_COLOR_MAGENTA },
	{ "<cyan>",                 6,  "</cyan>",      7,  ANSI_COLOR_CYAN },
	{ "<white>",                7,  "</colour>",    8,  ANSI_COLOR_WHITE },
};

static void
_str_append_string(gchar **dest, gchar *src)
{
	gchar *temp;

	temp = g_strdup(*dest);
	*dest = g_strdup_printf("%s%s", temp, src);
	g_free(temp);
}

static void
_str_append_char(gchar **dest, gchar src)
{
	gchar *temp;

	temp = g_strdup(*dest);
	*dest = g_strdup_printf("%s%c", temp, src);
	g_free(temp);
}

static void
_add_remaining_formatters(gpointer element, gpointer data)
{

	_str_append_string(data, (gchar *)element);
}

gboolean
wmud_text_to_ansi(gchar *text, gchar **result)
{
	gchar *a;
	GSList *current_formatters = NULL;
	gchar *r = g_strdup("");

	for (a = text; *a; a++)
	{
		gint i;
		gboolean found = FALSE;

		for (i = 0; i < sizeof(tags) / sizeof(wmud_tag_to_ansi); i++)
		{
			if (g_strncasecmp(a, tags[i].open_tag, tags[i].open_tag_length) == 0)
			{
				_str_append_string(&r, tags[i].ansi_sequence);

				current_formatters = g_slist_append(current_formatters, tags[i].ansi_sequence);
				a += tags[i].open_tag_length - 1;
				found = TRUE;
				break;
			}
			if (g_strncasecmp(a, tags[i].close_tag, tags[i].close_tag_length) == 0)
			{
				_str_append_string(&r, ANSI_NORMAL);

				current_formatters = g_slist_delete_link(current_formatters, g_slist_last(current_formatters));
				g_slist_foreach(current_formatters, _add_remaining_formatters, &r);
				a += tags[i].close_tag_length - 1;
				found = TRUE;
				break;
			}
		}

		if (found)
		{
			continue;
		}

		_str_append_char(&r, *a);
	}

	_str_append_string(&r, ANSI_NORMAL);

	*result = r;

	return TRUE;
}

