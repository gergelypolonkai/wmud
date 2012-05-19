AC_INIT([wMUD], [0.1.0])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([wMUD], [0.1.0])

AC_PROG_CC
AC_PROG_LIBTOOL
PKG_PROG_PKG_CONFIG

AC_CHECK_HEADERS([crypt.h])
AC_SEARCH_LIBS([crypt], [crypt])
AC_CHECK_FUNC([crypt], ac_have_crypt=yes, ac_have_crypt=no)
if test "$ac_have_crypt" != "yes"; then
	echo "The crypt() function call can not be found."
	exit 1
fi

AC_ARG_ENABLE([debug], AS_HELP_STRING([--enable-debug], [Enable debugging support (default: disabled)]), [enable_debug=yes; CFLAGS="$CFLAGS -g -Wall"], [enable_debug=no])
AM_CONDITIONAL([ENABLE_DEBUG], [test "$enable_debug" = "yes"])
if test "$enable_debug" = "yes"; then
	AC_DEFINE([DEBUG], [1], [Define to compile with debugging support])
fi

m4_ifdef([GTK_DOC_CHECK], [
GTK_DOC_CHECK([1.14], [--flavour no-tmpl])
],[
AM_CONDITIONAL([ENABLE_GTK_DOC], false)
])

AC_ARG_ENABLE([memcached], AS_HELP_STRING([--disable-memcached], [Disable Memcached support (default: enabled)]), [], [enable_memcached=yes])
AM_CONDITIONAL([ENABLE_MEMCACHED], [test "$enable_memcached" = "yes"])
if test "$enable_memcached" = "yes"; then
	PKG_CHECK_MODULES([MEMCACHED], libmemcached);
	AC_DEFINE([ENABLE_MEMCACHED], [1], [Define to compile with Memcached support])
fi

AC_DEFINE([DEFAULT_PORT], [4000], [The default port number if not specified in the config file])
AC_DEFINE([MAX_RECV_LEN], [1024], [The maximum length of the recv() queue in bytes])

PKG_CHECK_MODULES([GIO], gio-2.0)
PKG_CHECK_MODULES([GLIB], glib-2.0)
PKG_CHECK_MODULES([GTHREAD], gthread-2.0)
PKG_CHECK_MODULES([SQLITE3], sqlite3)
PKG_CHECK_MODULES([CURL], libcurl)
LIBCURL_CHECK_CONFIG
if test "$libcurl_protocol_SMTP" != "yes"; then
	echo "Your libCURL installation does not support the SMTP protocol."
	exit 1
fi

AC_OUTPUT(Makefile libgiosmtp/Makefile wmud/Makefile docs/reference/wmud/Makefile create-world.sh drop-world.sh)
