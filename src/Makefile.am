bin_PROGRAMS = wmud
wmud_SOURCES = wmud.c logger.c ansi.c networking.c world.c sessions.c configfile.c commands.c chat.c irc.c modules.c
wmud_CFLAGS = $(CFLAGS) $(EXTLIBS_CFLAGS) -I../libwmud-session -I../libwmud-world -Wall
wmud_LDADD = $(LIBS) $(EXTLIBS_LIBS) -L../libwmud-session -lwmud-session-1.0 -L../libwmud-world -lwmud-world-1.0
