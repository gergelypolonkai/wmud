LIBWMUD_PROTOCOL_TELNET_VERSION=1:0:0
lib_LTLIBRARIES = libwmud-protocol-telnet-1.0.la
libwmud_protocol_telnet_1_0_la_SOURCES = wmud-protocol-telnet.c
libwmud_protocol_telnet_1_0_la_CFLAGS = $(CFLAGS) $(GLIB_CFLAGS) $(GOBJECT_CFLAGS) $(GNET_CFLAGS) -Wall -I../src -I../libwmud-session
libwmud_protocol_telnet_1_0_la_LIBADD = $(LIBS) $(GLIB_LIBS) $(GOBJECT_LIBS) $(GNET_LIBS)
libwmud_protocol_telnet_1_0_la_LDFLAGS = -version-info $(LIBWMUD_PROTOCOL_TELNET_VERSION)
