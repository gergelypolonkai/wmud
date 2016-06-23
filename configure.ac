AC_INIT([wmud], [1.0], [polesz@w00d5t0ck.info])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
LT_INIT
AC_ARG_ENABLE(debug, [AC_HELP_STRING([--enable-debug], [compile with debugging support. Be warned that debugging support can eat a large amount of CPU when many clients are connected. Also, debug logs can become very large!])], , enable_debug=no)
               
if test "x$enable_debug" = "xyes" ; then
	AC_DEFINE([DEBUG], [1], [Define if debugging is enabled.])
fi

AC_PROG_CC
AC_PROG_LIBTOOL
PKG_PROG_PKG_CONFIG
PKG_CHECK_MODULES([GLIB], [glib-2.0])
PKG_CHECK_MODULES([GOBJECT], [gobject-2.0])
PKG_CHECK_MODULES([GNET], [gnet-2.0])
PKG_CHECK_MODULES([EXTLIBS], [libxml-2.0, glib-2.0, gobject-2.0, gthread-2.0, gnet-2.0, gnome-vfs-2.0, sqlite3 gmodule-2.0])
GTK_DOC_CHECK([1.10])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
	Makefile
	libwmud-session/Makefile
	libwmud-world/Makefile
	libwmud-state-sqlite3/Makefile
	libwmud-protocol-telnet/Makefile
	src/Makefile
	doc/reference/libwmud-session/Makefile
	doc/reference/libwmud-world/Makefile
])
AC_OUTPUT
