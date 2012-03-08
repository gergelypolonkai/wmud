AC_INIT([wMUD], [0.1.0])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_SRCDIR([configure.ac])

AM_INIT_AUTOMAKE([wMUD], [0.1.0])

AC_PROG_CC

AC_ARG_ENABLE([memcached], AS_HELP_STRING([--disable-memcached], [Disable Memcached support (default: enabled)]), [], [enable_memcached=yes])
AM_CONDITIONAL([ENABLE_MEMCACHED], [test "$enable_memcached" = "yes"])
if test "$enable_memcached" = "yes"; then
	PKG_CHECK_MODULES([MEMCACHED], libmemcached);
	AC_DEFINE([ENABLE_MEMCACHED], [1], [Define to compile with Memcached support])
fi

PKG_CHECK_MODULES([GIO], gio-2.0)
PKG_CHECK_MODULES([GLIB], glib-2.0)
PKG_CHECK_MODULES([SQLITE3], sqlite3)

AC_OUTPUT(Makefile src/Makefile)
