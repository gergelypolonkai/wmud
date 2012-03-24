bin_PROGRAMS = wmud
AM_CPPFLAGS = -DWMUD_STATEDIR=\""$(localstatedir)"\" -DWMUD_CONFDIR=\""$(sysconfdir)"\" $(MEMCACHED_CFLAGS) $(GLIB_CFLAGS) $(GIO_CFLAGS) $(GTHREAD_CFLAGS) $(SQLITE3_CFLAGS)

wmud_SOURCES = main.c networking.c interpreter.c db.c players.c maintenance.c
wmud_LDADD = $(MEMCACHED_LIBS) $(GLIB_LIBS) $(GIO_LIBS) $(GTHREAD_LIBS) $(SQLITE3_LIBS)
