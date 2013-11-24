bin_PROGRAMS = wmud
AM_CPPFLAGS = -DWMUD_STATEDIR=\""$(localstatedir)"\" -DWMUD_CONFDIR=\""$(sysconfdir)"\" -DG_LOG_DOMAIN=\""wMUD"\" $(MEMCACHED_CFLAGS) $(GLIB_CFLAGS) $(GIO_CFLAGS) $(GTHREAD_CFLAGS) $(GDA_CFLAGS) $(CURL_CFLAGS)

wmud_SOURCES =            \
	main.c            \
	main.h            \
	game-networking.c \
	game-networking.h \
	interpreter.c     \
	interpreter.h     \
	db.c              \
	db.h              \
	players.c         \
	players.h         \
	maintenance.c     \
	maintenance.h     \
	game.c            \
	game.h            \
	configuration.c   \
	configuration.h   \
	world.c           \
	world.h           \
	menu.c            \
	menu.h            \
	texts.c           \
	texts.h           \
	wmudclient.c      \
	wmudclient.h      \
	wmudplayer.c      \
	wmudplayer.h      \
	wmudclientstate.h \
	enumtypes.h       \
	enumtypes.c            \
	wmudworld.c            \
	wmudworld.h

wmud_LDADD = $(MEMCACHED_LIBS) $(GLIB_LIBS) $(GIO_LIBS) $(GTHREAD_LIBS) $(GDA_LIBS) $(CURL_LIBS)

enum_headers = wmudclientstate.h

enumtypes.h: $(enum_headers) enumtypes.h.template
	$(AM_V_GEN) ( top_builddir=`cd $(top_builddir) && pwd`; \
	  cd $(srcdir) && $(GLIB_MKENUMS) --template enumtypes.h.template $(enum_headers) ) > \
	    enumtypes.h.tmp && mv enumtypes.h.tmp enumtypes.h

enumtypes.c: $(enum_headers) enumtypes.h enumtypes.c.template
	$(AM_V_GEN) ( top_builddir=`cd $(top_builddir) && pwd`; \
	  cd $(srcdir) && $(GLIB_MKENUMS) --template enumtypes.c.template $(enum_headers) ) > \
	    enumtypes.c.tmp && mv enumtypes.c.tmp enumtypes.c

