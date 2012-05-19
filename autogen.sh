#! /bin/sh

mkdir m4 &> /dev/null
gtkdocize --copy || exit 1
libtoolize --install --copy || exit 1
autoheader || exit 1
aclocal || exit 1
autoconf || exit 1
automake --add-missing --copy || exit 1

