LIBWMUD_SESSION_VERSION=1:0:0
lib_LTLIBRARIES = libwmud-session-1.0.la
libwmud_session_1_0_la_SOURCES = wmud-session.c
libwmud_session_1_0_la_CFLAGS = $(CFLAGS) $(GLIB_CFLAGS) $(GOBJECT_CFLAGS) $(GNET_CFLAGS) -Wall
libwmud_session_1_0_la_LIBADD = $(LIBS) $(GLIB_LIBS) $(GOBJECT_LIBS) $(GNET_LIBS)
libwmud_session_1_0_la_LDFLAGS = -version-info $(LIBWMUD_SESSION_VERSION)
