# Copyright 1999-2024 the Claws Mail team.
# This file is part of Claws Mail package, and distributed under the
# terms of the General Public License version 3 (or later).
# See COPYING file for license details.

if BUILD_TESTS
include $(top_srcdir)/tests.mk
SUBDIRS = . tests
endif

DESKTOPFILEPATH=$(datadir)/applications/claws-mail.desktop

noinst_LTLIBRARIES = libclawscommon.la

arch_files =
arch_header =

libclawscommon_la_SOURCES = $(arch_sources) \
	codeconv.c \
	file-utils.c \
	hooks.c \
	log.c \
	md5.c \
	mgutils.c \
	passcrypt.c \
	prefs.c \
	progressindicator.c \
	proxy.c \
	quoted-printable.c \
	session.c \
	smtp.c \
	socket.c \
	ssl.c \
	ssl_certificate.c \
	string_match.c \
	stringtable.c \
	claws.c \
	template.c \
	utils.c \
	uuencode.c \
	xml.c \
	xmlprops.c \
	pkcs5_pbkdf2.c \
	unmime.c

clawscommonincludedir = $(pkgincludedir)/common
clawscommoninclude_HEADERS = $(arch_headers) \
	codeconv.h \
	file-utils.h \
	defs.h \
	hooks.h \
	log.h \
	md5.h \
	mgutils.h \
	passcrypt.h \
	prefs.h \
	progressindicator.h \
	proxy.h \
	quoted-printable.h \
	session.h \
	smtp.h \
	socket.h \
	ssl_certificate.h \
	ssl.h \
	string_match.h \
	stringtable.h \
	claws.h \
	tags.h \
	template.h \
	utils.h \
	uuencode.h \
	version.h \
	xml.h \
	xmlprops.h \
	pkcs5_pbkdf2.h \
	unmime.h

AM_CPPFLAGS = \
	$(GLIB_CFLAGS) \
	$(VALGRIND_CFLAGS) \
	-DLOCALEDIR=\""$(localedir)"\" \
	-DDATAROOTDIR=\""$(datarootdir)"\" \
	-DDESKTOPFILEPATH=\"$(DESKTOPFILEPATH)\" \
	-DGTK_DISABLE_DEPRECATED \
	-I../fence/zig-out/include

libclawscommon_la_LIBADD = \
	$(GLIB_LIBS) \
	$(CRYPT_LIBS) \
	-lpthread

EXTRA_DIST = \
	version.h.in
