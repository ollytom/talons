# Copyright 1999-2014 the Claws Mail team.
# This file is part of Claws Mail package, and distributed under the
# terms of the General Public License version 3 (or later).
# See COPYING file for license details.

DOCDIR = $(docdir)

noinst_LTLIBRARIES = libclawsgtk.la

EXTRA_DIST = \
	claws-marshal.list

libclawsgtk_la_SOURCES = \
	about.c \
	combobox.c \
	description_window.c \
	filesel.c \
	foldersort.c \
	gtkcmctree.c \
	gtkcmclist.c \
	gtksctree.c \
	gtkunit.c \
	gtkutils.c \
	gtkvscrollbutton.c \
	icon_legend.c \
	inputdialog.c \
	logwindow.c \
	manage_window.c \
	menu.c \
	prefswindow.c \
	progressdialog.c \
	quicksearch.c \
	claws-marshal.c

clawsgtkincludedir = $(pkgincludedir)/gtk
clawsgtkinclude_HEADERS = \
	about.h \
	authors.h \
	combobox.h \
	description_window.h \
	filesel.h \
	foldersort.h \
	gtkutils.h \
	gtkunit.h \
	gtkvscrollbutton.h \
	headers.h \
	icon_legend.h \
	inputdialog.h \
	logwindow.h \
	manage_window.h \
	menu.h \
	prefswindow.h \
	progressdialog.h \
	quicksearch.h \
	claws-marshal.h \
	gtkcmctree.h \
	gtkcmclist.h \
	gtksctree.h

AM_CPPFLAGS = \
	-I$(srcdir)/../common \
	-I../common \
	-I$(srcdir)/.. \
	-I$(builddir)/.. \
	$(GTK_CFLAGS) \
	-DDOCDIR=\"$(DOCDIR)\"

libclawsgtk_la_LIBADD = \
	../common/libclawscommon.la \
	$(GTK_LIBS)

BUILT_SOURCES=claws-marshal.c claws-marshal.h

claws-marshal.h: claws-marshal.list
	$(GLIB_GENMARSHAL) $< --header --prefix=claws_marshal > $@

claws-marshal.c: claws-marshal.list
	$(GLIB_GENMARSHAL) $< --body --prefix=claws_marshal > $@

.PHONY: test
