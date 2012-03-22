#include <glib.h>
#include <sqlite3.h>

#include "db.h"

sqlite3 *dbh = NULL;

gboolean
wmud_db_init(GError **err)
{
	return FALSE;
}

