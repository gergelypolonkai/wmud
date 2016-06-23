#ifndef __WMUD_CORE_ANSI_H__
#define __WMUD_CORE_ANSI_H__

#define ANSI_CSI "\x1b["
#define ANSI_NORMAL ANSI_CSI "0m"
#define ANSI_BOLD ANSI_CSI "1m"
#define ANSI_UNDERLINE ANSI_CSI "4m"
#define ANSI_COLOR_BLACK ANSI_CSI "30m"
#define ANSI_COLOR_RED ANSI_CSI "31m"
#define ANSI_COLOR_GREEN ANSI_CSI "32m"
#define ANSI_COLOR_YELLOW ANSI_CSI "33m"
#define ANSI_COLOR_BLUE ANSI_CSI "34m"
#define ANSI_COLOR_MAGENTA ANSI_CSI "35m"
#define ANSI_COLOR_CYAN ANSI_CSI "36m"
#define ANSI_COLOR_WHITE ANSI_CSI "37m"

gboolean wmud_text_to_ansi(gchar *text, gchar **result);

#endif /* __WMUD_CODE__ANSI_H__ */

