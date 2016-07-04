ACLOCAL_AMFLAGS = -I m4
SUBDIRS = wmud

if ENABLE_GTK_DOC
    SUBDIRS += docs/reference/wmud
endif

DISTCHECK_CONFIGURE_FLAGS = --enable-gtk-doc

csclean:
	-rm -f cscope.out

cscope.out:
	cscope -bR

cscope: csclean cscope.out
