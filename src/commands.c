#include <glib.h>
#include <gnet.h>

#include <string.h>

#include "wmud.h"
#include "logger.h"
#include "networking.h"
#include "sessions.h"

#include "wmud-session.h"

#define COMMAND_DELIMITERS " \t\n\r"

#define WMUD_COMMAND_ERROR g_quark_from_string("wMUDCommandError")
enum {
	WMUD_COMMAND_ERROR_SUCCESS   /* Will never be used */
};

/* Internal command callback function for prototypes and function headers. If
 * you modify this, don't forget to modify the wMUDCommandCallback type!
 */
#define WMUD_INTERNAL_COMMAND(x) gboolean x (wMUDSession *session, gchar **params, GError **error)

/* Internal command callback function type to be usd in the wMUDCommand struct.
 * If you modify this, don't forget to modify the WMUD_INTERNAL_COMMAND(x)
 * macro!
 */
typedef gboolean (*wMUDCommandCallback)(wMUDSession *session, gchar **params, GError **error);

typedef struct _wMUDCommand
{
	gchar *cmd;
	gchar *collate_key;
	gint min_params;
	gint max_params;
	wMUDCommandCallback callback;
} wMUDCommand;

struct _wMUDSplitHelper
{
	gchar **list;
	gint position;
};

WMUD_INTERNAL_COMMAND(_wmud_command_internal_quit);

static wMUDCommand _wmud_command_list[] = {
	{ "QUIT", NULL, 0, 0, _wmud_command_internal_quit },
	{ NULL,   NULL, 0, 0, NULL }
};

WMUD_INTERNAL_COMMAND(_wmud_command_internal_quit)
{
	wmud_log_debug("Here in the QUIT function");
	wmud_finish_session(session, "Good bye");
	return TRUE;
}

static void
_add_words(gpointer data, gpointer user_data)
{
	struct _wMUDSplitHelper *split_data = (struct _wMUDSplitHelper *)user_data;
	gchar *word = (gchar *)data;

	Context split_data->list[split_data->position] = word;
	split_data->position++;
}

static void
_free_words(gpointer data, gpointer user_data)
{
	g_free(data);
}

static gint
_wmud_split_command(gchar *command, gchar ***list)
{
	gchar *a;
	gint word_started_at = 0;
	gint apos_at = -1;
	gint quot_at = -1;
	gboolean was_in_word = FALSE;
	gboolean in_string = TRUE;
	GSList *word_list = NULL;
	gchar **ret;
	struct _wMUDSplitHelper split_data;
	gint word_count;

	for (a = command; in_string; a++)
	{
		if (*a == 0)
		{
			in_string = FALSE;
		}
		if (
				(*a == 0)
				|| (
					(quot_at == -1)
					&& (*a == '\'')
				)
				|| (
					(apos_at == -1)
					&& (*a == '"')
				)
				|| (
					(apos_at != -1)
					&& (*a == '\'')
				)
				|| (
					(quot_at != -1)
					&& (*a == '"')
				)
				|| (
					(apos_at == -1)
					&& (quot_at == -1)
					&& strchr(COMMAND_DELIMITERS, *a)
				)
			)
		{
			if (was_in_word)
			{
				was_in_word = FALSE;
				word_list = g_slist_append(word_list, g_strndup((command + word_started_at), (a - command) - word_started_at));
				word_started_at = (a - command);
			}
			if ((apos_at == -1) && (*a == '"'))
			{
				if (quot_at == -1)
				{
					quot_at = (a - command) - 1;
					if (quot_at == -1)
					{
						quot_at = 0;
					}
				}
				else
				{
					quot_at = -1;
				}
			}
			if ((quot_at == -1) && (*a == '\''))
			{
				if (apos_at == -1)
				{
					apos_at = (a - command) - 1;
					if (apos_at == -1)
					{
						apos_at = 0;
					}
				}
				else
				{
					apos_at = -1;
				}
			}
			word_started_at++;
			continue;
		}
		was_in_word = TRUE;
	}
	
	if ((quot_at != -1) || (apos_at != -1))
	{
		wmud_log_debug("Got illegal string: %s", command);

		g_slist_foreach(word_list, _free_words, NULL);
		g_slist_free(word_list);

		return -1;
	}

	word_count = g_slist_length(word_list);
	Context ret = g_new0(gchar *, word_count + 1);

	split_data.list = ret;
	split_data.position = 0;

	Context g_slist_foreach(word_list, _add_words, &split_data);
	g_slist_free(word_list);

	if (list != NULL)
	{
		Context *list = ret;
	}

	return word_count;
}

void
wmud_process_command(GConn *connection, gchar *command)
{
	gchar **command_parts = NULL,
		  *command_casefold,
		  *command_key;
	gint word_count;
	wMUDCommand *command_rec;
	gboolean command_found = FALSE;
	wMUDSession *session;

	session = wmud_session_for_connection(connection);

	if (session == NULL)
	{
		wmud_log_error("Processing command for a non-existant session!");
		return;
	}

	Context;
	wmud_log_debug("Processing command %s", command);

	Context word_count = _wmud_split_command(command, &command_parts);

	if (word_count == -1)
	{
		Context wmud_connection_send(connection, "Illegal command. Maybe you forgot a closing apostroph or quote?");
		Context return;
	}

	if (word_count == 0)
	{
		wmud_log_debug("Got an empty line");
		return;
	}

	command_casefold = g_utf8_casefold(command_parts[0], -1);
	command_key = g_utf8_collate_key(command_casefold, -1);
	g_free(command_casefold);
	for (command_rec = _wmud_command_list; command_rec->cmd; command_rec++)
	{
		if (command_rec->collate_key == NULL)
		{
			gchar *temp;

			temp = g_utf8_casefold(command_rec->cmd, -1);
			command_rec->collate_key = g_utf8_collate_key(temp, -1);
			g_free(temp);
		}

		if (strcmp(command_key, command_rec->collate_key) == 0)
		{
			gint param_count = g_strv_length(command_parts) - 1;
			GError *error = NULL;
			command_found = TRUE;

			if ((param_count < command_rec->min_params) || (param_count > command_rec->max_params))
			{
				Context wmud_connection_send(connection, "Wrong number of parameters. Maybe you should try HELP %s", command_rec->cmd);
			}
			wmud_log_debug("Executing command %s, having %d parameters", command_rec->cmd, param_count);
			(command_rec->callback)(session, command_parts + 1, &error);
			break;
		}
	}
	g_free(command_key);

	if (!command_found)
	{
		/* TODO: command prediction (Maybe you wanted to type...) */
		Context wmud_connection_send(connection, "Unknown command.");
	}

	Context g_strfreev(command_parts);
}

