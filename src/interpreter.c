#include <glib.h>
#include <gio/gio.h>
#include <string.h>

#include "networking.h"

static void
command_part_print(GString *data, gpointer user_data)
{
	g_print("Part: %s\n", data->str);
}

void
wmud_interpret_game_command(wmudClient *client)
{
	GSList *command_parts = NULL;
	gchar *a,
	      *last_start = NULL;
	gboolean in_string = FALSE;
	gchar string_delim = 0;
	gboolean prev_space = TRUE;

	g_print("Interpreting '%s' as a client command\n", client->buffer->str);

	for (a = client->buffer->str; (a - client->buffer->str) < client->buffer->len + 1; a++)
	{
		if ((!g_ascii_isspace(*a) && *a) || in_string)
		{
			if (prev_space)
			{
				last_start = a;
				g_print("New token starts here: %s\n", a);
			}

			prev_space = FALSE;
		}

		if ((!in_string && ((*a == '\'') || (*a == '"'))) || (in_string && (*a == string_delim)))
		{
			g_print("String %s\n", (in_string) ? "ended" : "started");
			in_string = !in_string;
			if (!in_string)
				string_delim = 0;
			prev_space = FALSE;
			continue;
		}

		if (in_string)
			continue;

		if (g_ascii_isspace(*a) || !*a)
		{
			g_print("Space found.\n");
			if (!prev_space)
			{
				GString *token = g_string_new_len(last_start, a - last_start);
				command_parts = g_slist_prepend(command_parts, token);
				g_print("Found new token, %d long\n", a - last_start);
			}
			prev_space = TRUE;
		}
	}

	command_parts = g_slist_reverse(command_parts);
	g_slist_foreach(command_parts, (GFunc)command_part_print, NULL);
}

gboolean
wmud_interpreter_init(void)
{
	return TRUE;
}

