ACLOCAL_AMFLAGS = -I m4
SUBDIRS = wmud docs/reference/wmud
DISTCHECK_CONFIGURE_FLAGS = --enable-gtk-doc

csclean:
	-rm -f cscope.out

cscope.out:
	cscope -bR

cscope: csclean cscope.out
