#include <glib.h>
#include <gmodule.h>

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include "modules.h"
#include "configfile.h"
#include "logger.h"
#include "wmud.h"

#include "wmud-module.h"

static GModule *statesave_module = NULL;

gboolean (*wmud_save_state)(GError **error) = NULL;
gpointer wMUDCallables[WMUD_CALLABLE_LAST];

void
wmud_fill_callable_table(void)
{
	wMUDCallables[WMUD_CALLABLE_WMUD_LOG] = wmud_log;
#ifdef DEBUG
	wMUDCallables[WMUD_CALLABLE_WMUD_PRINT_CONTEXT] = wmud_print_context;
#endif /* DEBUG */
	wMUDCallables[WMUD_CALLABLE_LAST] = NULL;
}

gboolean
wmud_load_config_modules(wMUDConfiguration *config)
{
	gchar *statesave_module_short_name,
		  *statesave_module_file,
		  *func_name;
	gboolean (*module_is_statesaving)(void);
	gboolean (*module_load)(wMUDConfiguration *config);

	if (!g_module_supported())
	{
		wmud_log_error("Module loading is not supported!");
		return FALSE;
	}
	Context;

	if (config->modules_dir == NULL)
	{
		wmud_log_error("Module directory must be set in the configfile!");
		return FALSE;
	}

	if (config->statesave_module == NULL)
	{
		wmud_log_error("State saving module must be set in the configfile!");
		return FALSE;
	}

	statesave_module_short_name = g_strdup_printf("state-%s", config->statesave_module);
	statesave_module_file = g_module_build_path(config->modules_dir, statesave_module_short_name);
	g_free(statesave_module_short_name);

	wmud_log_debug("Trying to load \"%s\" as state-saving module", statesave_module_file);

	if ((statesave_module = g_module_open(statesave_module_file, G_MODULE_BIND_LAZY | G_MODULE_BIND_LOCAL)) == NULL)
	{
		wmud_log_error("Unable to load state saving module %s", statesave_module_file);
		return FALSE;
	}

	func_name = g_strdup_printf("wmud_statesave_%s_is_statesave", config->statesave_module);
	if (g_module_symbol(statesave_module, func_name, (gpointer *)&module_is_statesaving))
	{
		if (!module_is_statesaving())
		{
			wmud_log_error("Module %s is not marked as statesaving!", statesave_module_file);
			g_module_close(statesave_module);
			g_free(func_name);
			return FALSE;
		}
	}
	else
	{
		wmud_log_error("Undefined symbol \"%s\" in statesave module %s", func_name, statesave_module_file);
		g_module_close(statesave_module);
		g_free(func_name);
		return FALSE;
	}
	g_free(func_name);

	func_name = g_strdup_printf("wmud_statesave_%s_load", config->statesave_module);
	if (g_module_symbol(statesave_module, func_name, (gpointer *)&module_load))
	{
		if (!module_load(config))
		{
			wmud_log_error("Module initialization failed for %s", statesave_module_file);
			g_module_close(statesave_module);
			g_free(func_name);
			return FALSE;
		}
	}
	else
	{
		wmud_log_error("Undefined symbol \"%s\" in statesave module %s", func_name, statesave_module_file);
		g_module_close(statesave_module);
		g_free(func_name);
		return FALSE;
	}

	g_free(statesave_module_file);
	return TRUE;
}

