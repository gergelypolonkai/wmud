#include <string.h>
#include <glib.h>
#include <gnet.h>

#include "wmud.h"
#include "logger.h"

#define COMMAND_DELIMITERS " \t\n\r"

struct _wMUDSplitHelper
{
	gchar **list;
	gint position;
};

static void
_add_words(gpointer data, gpointer user_data)
{
	struct _wMUDSplitHelper *split_data = (struct _wMUDSplitHelper *)user_data;
	gchar *word = (gchar *)data;

	split_data->list[split_data->position] = word;
	split_data->position++;
}

static gint
_wmud_split_irc_line(gchar *command, gchar ***list)
{
	gchar *a;
	gint word_started_at = 0;
	gboolean was_in_word = FALSE;
	gboolean in_string = TRUE;
	guint colon_at = -1;
	GSList *word_list = NULL;
	gchar **ret;
	
	struct _wMUDSplitHelper split_data;
	gint word_count;

	Context;

	for (a = command; in_string; a++)
	{
		if (*a == 0)
		{
			in_string = FALSE;
		}
		if (
				(*a == 0)
				|| (
					(colon_at == -1)
					&& strchr(COMMAND_DELIMITERS, *a)
				)
				|| (
					(colon_at == -1)
					&& (*a == ':')
				)
			)
		{
			if (was_in_word)
			{
				was_in_word = FALSE;
				word_list = g_slist_append(word_list, g_strndup((command + word_started_at), (a - command) - word_started_at));
				word_started_at = (a - command);
			}
			if ((*a == ':') && (colon_at == -1))
			{
				colon_at = (a - command) - 1;
				if (colon_at == -1)
				{
					colon_at = 0;
				}
			}
			word_started_at++;
			continue;
		}
		was_in_word = TRUE;
	}
	
	word_count = g_slist_length(word_list);
	Context ret = g_new0(gchar *, word_count + 1);

	split_data.list = ret;
	split_data.position = 0;

	g_slist_foreach(word_list, _add_words, &split_data);
	g_slist_free(word_list);

	if (list != NULL)
	{
		Context *list = ret;
	}

	return word_count;
}

void
wmud_process_irc_command(GConn *connection, gchar *command)
{
	gchar **command_parts;
	gint word_count;

	word_count = _wmud_split_irc_line(command, &command_parts);

	wmud_log_debug("Got command '%s' with %d parts", command, word_count);
}

