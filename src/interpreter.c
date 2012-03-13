#include <glib.h>
#include <gio/gio.h>
#include <string.h>

#include "interpreter.h"
#include "networking.h"

#define IS_SPACE(c) (g_ascii_isspace((c)) || (!(c)))

static wmudCommand command_list[] = {
	{ "quit", gcmd_quit },
	{ NULL, NULL },
};

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
		if (IS_SPACE(*a) && prev_space)
			continue;

		if (IS_SPACE(*a) && !prev_space && !in_string)
		{
			GString *token = g_string_new_len(last_start, a - last_start);
			command_parts = g_slist_prepend(command_parts, token);
			prev_space = TRUE;
			continue;
		}

		if (!IS_SPACE(*a) && prev_space)
		{
			last_start = a;
			prev_space = FALSE;
		}

		if (((*a == '\'') || (*a == '"')) && !in_string)
		{
			in_string = TRUE;
			string_delim = *a;
			prev_space = TRUE;
			continue;
		}

		if (in_string && (*a == string_delim))
		{
			in_string = FALSE;
			string_delim = 0;
			prev_space = TRUE;
			continue;
		}
	}

	if (in_string)
	{
		GString *buf = g_string_new("");
		g_string_printf(buf, "You should close quotation characters, like %c...\r\n", string_delim);
		/* TODO: error checking */
		g_socket_send(client->socket, buf->str, buf->len, NULL, NULL);
		g_string_free(buf, TRUE);
	}

	command_parts = g_slist_reverse(command_parts);
	g_slist_foreach(command_parts, (GFunc)command_part_print, NULL);
}

gboolean
wmud_interpreter_init(void)
{
	return TRUE;
}

