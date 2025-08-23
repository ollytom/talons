# Copyright 1999-2021 the Claws Mail team.
# This file is part of Claws Mail package, and distributed under the
# terms of the General Public License version 3 (or later).
# See COPYING file for license details.


SUBDIRS = common gtk etpan fence .

bin_PROGRAMS = claws-mail

abook_source = \
	addrbook.c \
	addrclip.c \
	addressbook.c \
	addressbook_foldersel.c \
	addrindex.c \
	addrquery.c \
	addrselect.c \
	addrduplicates.c \
	addrmerge.c \
	editaddress.c \
	editbook.c \
	editgroup.c

abook_headers = \
	adbookbase.h \
	addrbook.h \
	addrclip.h \
	addressbook.h \
	addressbook_foldersel.h \
	addrindex.h \
	addrquery.h \
	addrselect.h \
	addrduplicates.h \
	addrmerge.h \
	editaddress.h \
	editbook.h \
	editgroup.h

claws_mail_SOURCES = \
	account.c \
	action.c \
	addrcache.c \
	addr_compl.c \
	addressadd.c \
	addrgather.c \
	addrharvest.c \
	addritem.c \
	advsearch.c \
	alertpanel.c \
	autofaces.c \
	avatars.c \
	compose.c \
	customheader.c \
	displayheader.c \
	entity.c \
	export.c \
	folder.c \
	folder_item_prefs.c \
	foldersel.c \
	folderutils.c \
	folderview.c \
	html.c \
	imap.c \
	imap_gtk.c \
	import.c \
	inc.c \
	localfolder.c \
	main.c \
	mainwindow.c \
	manual.c \
	matcher.c \
	matcher_parser_lex.l \
	matcher_parser_parse.y \
	mbox.c \
	message_search.c \
	messageview.c \
	mh.c \
	mh_gtk.c \
	mimeview.c \
	msgcache.c \
	noticeview.c \
	oauth2.c \
	password.c \
	passwordstore.c \
	pop.c \
	prefs_account.c \
	prefs_actions.c \
	prefs_common.c \
	prefs_compose_writing.c \
	prefs_customheader.c \
	prefs_display_header.c \
	prefs_ext_prog.c \
	prefs_folder_item.c \
	prefs_gtk.c \
	prefs_logging.c \
	prefs_matcher.c \
	prefs_message.c \
	prefs_other.c \
	prefs_quote.c \
	prefs_receive.c \
	prefs_send.c \
	prefs_summaries.c \
	prefs_summary_column.c \
	prefs_summary_open.c \
	prefs_folder_column.c \
	prefs_template.c \
	prefs_themes.c \
	prefs_toolbar.c \
	prefs_wrapping.c \
	printing.c \
	privacy.c \
	procheader.c \
	procmime.c \
	procmsg.c \
	quote_fmt.c \
	quote_fmt_lex.l \
	quote_fmt_parse.y \
	send_message.c \
	setup.c \
	sourcewindow.c \
	ssl_manager.c \
	statusbar.c \
	stock_pixmap.c \
	summary_search.c \
	summaryview.c \
	textview.c \
	toolbar.c \
	undo.c \
	$(abook_source)

claws_mailincludedir = $(pkgincludedir)
claws_mailinclude_HEADERS = \
	account.h \
	action.h \
	addrcache.h \
	addr_compl.h \
	addrdefs.h \
	addressadd.h \
	addritem.h \
	addressitem.h \
	addrgather.h \
	addrharvest.h \
	advsearch.h \
	alertpanel.h \
	autofaces.h \
	avatars.h \
	compose.h \
	customheader.h \
	displayheader.h \
	entity.h \
	export.h \
	folder.h \
	folder_item_prefs.h \
	foldersel.h \
	folderutils.h \
	folderview.h \
	html.h \
	imap.h \
	imap_gtk.h \
	import.h \
	inc.h \
	localfolder.h \
	main.h \
	mainwindow.h \
	manual.h \
	matcher.h \
	matchertypes.h \
	matcher_parser.h \
	matcher_parser_lex.h \
	matcher_parser_parse.h \
	mbox.h \
	message_search.h \
	messageview.h \
	mh.h \
	mh_gtk.h \
	mimeview.h \
	msgcache.h \
	news.h \
	noticeview.h \
	oauth2.h \
	partial_download.h \
	password.h \
	passwordstore.h \
	pop.h \
	prefs_account.h \
	prefs_actions.h \
	prefs_common.h \
	prefs_compose_writing.h \
	prefs_customheader.h \
	prefs_display_header.h \
	prefs_ext_prog.h \
	prefs_folder_item.h \
	prefs_gtk.h \
	prefs_logging.h \
	prefs_matcher.h \
	prefs_message.h \
	prefs_other.h \
	prefs_quote.h \
	prefs_receive.h \
	prefs_send.h \
	prefs_summaries.h \
	prefs_summary_column.h \
	prefs_summary_open.h \
	prefs_folder_column.h \
	prefs_template.h \
	prefs_themes.h \
	prefs_toolbar.h \
	prefs_wrapping.h \
	printing.h \
	privacy.h \
	procheader.h \
	procmime.h \
	procmsg.h \
	proctypes.h \
	quote_fmt.h \
	quote_fmt_lex.h \
	quote_fmt_parse.h \
	remotefolder.h \
	send_message.h \
	setup.h \
	sourcewindow.h \
	ssl_manager.h \
	statusbar.h \
	stock_pixmap.h \
	summary_search.h \
	summaryview.h \
	textview.h \
	toolbar.h \
	undo.h \
	viewtypes.h \
	$(abook_headers)

BUILT_SOURCES = \
	matcher_parser_parse.h \
	quote_fmt_parse.h

CLEANFILES = \
	     $(BUILT_SOURCES)

EXTRA_DIST = \
	pixmaps/addr_one.xpm \
	pixmaps/addr_two.xpm \
	pixmaps/address_book.xpm \
	pixmaps/address_search.xpm \
	pixmaps/address.xpm \
	pixmaps/anonymous.xpm \
	pixmaps/book.xpm \
	pixmaps/cancel.xpm \
	pixmaps/category.xpm \
	pixmaps/checkbox_off.xpm \
	pixmaps/checkbox_on.xpm \
	pixmaps/check_spelling.xpm \
	pixmaps/clipkey.xpm \
	pixmaps/claws_mail_icon.xpm \
	pixmaps/claws_mail_icon_64.xpm \
	pixmaps/claws_mail_compose_logo.xpm \
	pixmaps/claws_mail_logo.xpm \
	pixmaps/claws_mail_logo_small.xpm \
	pixmaps/clip.xpm \
	pixmaps/clip_gpg_signed.xpm \
	pixmaps/close.xpm \
	pixmaps/complete.xpm \
	pixmaps/continue.xpm \
	pixmaps/copied.xpm \
	pixmaps/delete_btn.xpm \
	pixmaps/delete_dup_btn.xpm \
	pixmaps/deleted.xpm \
	pixmaps/dir_close.xpm \
	pixmaps/dir_close_hrm.xpm \
	pixmaps/dir_close_hrm_mark.xpm \
	pixmaps/dir_close_mark.xpm \
	pixmaps/dir_open.xpm \
	pixmaps/dir_open_hrm.xpm \
	pixmaps/dir_open_hrm_mark.xpm \
	pixmaps/dir_open_mark.xpm \
	pixmaps/dir_noselect_close.xpm \
	pixmaps/dir_noselect_close_mark.xpm \
	pixmaps/dir_noselect_open.xpm \
	pixmaps/dir_subs_close_mark.xpm \
	pixmaps/dir_subs_close.xpm \
	pixmaps/dir_subs_open.xpm \
	pixmaps/doc_index.xpm \
	pixmaps/doc_index_close.xpm \
	pixmaps/doc_info.xpm \
	pixmaps/down_arrow.xpm \
	pixmaps/drafts_close.xpm \
	pixmaps/drafts_close_mark.xpm \
	pixmaps/drafts_open.xpm \
	pixmaps/drafts_open_mark.xpm \
	pixmaps/edit_extern.xpm \
	pixmaps/empty.xpm \
	pixmaps/error.xpm \
	pixmaps/exec.xpm \
	pixmaps/first_arrow.xpm \
	pixmaps/forwarded.xpm \
	pixmaps/gpg_signed.xpm \
	pixmaps/go_folders.xpm \
	pixmaps/group.xpm \
	pixmaps/ham_btn.xpm \
	pixmaps/ignorethread.xpm \
	pixmaps/inbox_open.xpm \
	pixmaps/inbox_open_hrm.xpm \
	pixmaps/inbox_open_hrm_mark.xpm \
	pixmaps/inbox_open_mark.xpm \
	pixmaps/inbox_close.xpm \
	pixmaps/inbox_close_hrm.xpm \
	pixmaps/inbox_close_hrm_mark.xpm \
	pixmaps/inbox_close_mark.xpm \
	pixmaps/interface.xpm \
	pixmaps/insert_file.xpm \
	pixmaps/key.xpm \
	pixmaps/key_gpg_signed.xpm \
	pixmaps/last_arrow.xpm \
	pixmaps/left_arrow.xpm \
	pixmaps/linewrap.xpm \
	pixmaps/linewrapcurrent.xpm \
	pixmaps/locked.xpm \
	pixmaps/mail_attach.xpm \
	pixmaps/mail_compose.xpm \
	pixmaps/mail_forward.xpm \
	pixmaps/mail_privacy_encrypted.xpm \
	pixmaps/mail_privacy_signed.xpm \
	pixmaps/mail_receive_all.xpm \
	pixmaps/mail_receive.xpm \
	pixmaps/mail_reply_to_all.xpm \
	pixmaps/mail_reply_to_author.xpm \
	pixmaps/mail_reply_to_list.xpm \
	pixmaps/mail_reply.xpm \
	pixmaps/mail_send_queue.xpm \
	pixmaps/mail_send.xpm \
	pixmaps/mail_sign.xpm \
	pixmaps/mail_draft.xpm \
	pixmaps/mark.xpm \
	pixmaps/mark_allread.xpm \
	pixmaps/mark_allunread.xpm \
	pixmaps/mark_ignorethread.xpm \
	pixmaps/mark_locked.xpm \
	pixmaps/mark_mark.xpm \
	pixmaps/mark_read.xpm \
	pixmaps/mark_unlocked.xpm \
	pixmaps/mark_unmark.xpm \
	pixmaps/mark_unread.xpm \
	pixmaps/mark_watchthread.xpm \
	pixmaps/mime_application.xpm \
	pixmaps/mime_audio.xpm \
	pixmaps/mime_image.xpm \
	pixmaps/mime_message.xpm \
	pixmaps/mime_pdf.xpm \
	pixmaps/mime_ps.xpm \
	pixmaps/mime_calendar.xpm \
	pixmaps/mime_pgpsig.xpm \
	pixmaps/mime_text_html.xpm \
	pixmaps/mime_text_patch.xpm \
	pixmaps/mime_text_plain.xpm \
	pixmaps/mime_unknown.xpm \
	pixmaps/moved.xpm \
	pixmaps/news_compose.xpm \
	pixmaps/new.xpm \
	pixmaps/notice_error.xpm \
	pixmaps/notice_note.xpm \
	pixmaps/notice_warn.xpm \
	pixmaps/offline.xpm \
	pixmaps/online.xpm \
	pixmaps/open_mail.xpm \
	pixmaps/outbox_close.xpm \
	pixmaps/outbox_close_hrm.xpm \
	pixmaps/outbox_open.xpm \
	pixmaps/outbox_open_hrm.xpm \
	pixmaps/outbox_close_mark.xpm \
	pixmaps/outbox_close_hrm_mark.xpm \
	pixmaps/outbox_open_mark.xpm \
	pixmaps/outbox_open_hrm_mark.xpm \
	pixmaps/paste.xpm \
	pixmaps/preferences.xpm \
	pixmaps/printer.xpm \
	pixmaps/printer_btn.xpm \
	pixmaps/privacy_expired.xpm \
	pixmaps/privacy_failed.xpm \
	pixmaps/privacy_passed.xpm \
	pixmaps/privacy_signed.xpm \
	pixmaps/privacy_unknown.xpm \
	pixmaps/privacy_warn.xpm \
	pixmaps/privacy_emblem_encrypted.xpm \
	pixmaps/privacy_emblem_failed.xpm \
	pixmaps/privacy_emblem_passed.xpm \
	pixmaps/privacy_emblem_signed.xpm \
	pixmaps/privacy_emblem_warn.xpm \
	pixmaps/properties.xpm \
	pixmaps/queue_close.xpm \
	pixmaps/queue_close_hrm.xpm \
	pixmaps/queue_close_hrm_mark.xpm \
	pixmaps/queue_close_mark.xpm \
	pixmaps/queue_open.xpm \
	pixmaps/queue_open_hrm.xpm \
	pixmaps/queue_open_hrm_mark.xpm \
	pixmaps/queue_open_mark.xpm \
	pixmaps/quicksearch.xpm \
	pixmaps/replied.xpm \
	pixmaps/replied_and_forwarded.xpm \
	pixmaps/right_arrow.xpm \
	pixmaps/rotate_left.xpm \
	pixmaps/rotate_right.xpm \
	pixmaps/selection.xpm \
	pixmaps/spam.xpm \
	pixmaps/spam_btn.xpm \
	pixmaps/trash_btn.xpm \
	pixmaps/trash_close.xpm \
	pixmaps/trash_close_hrm.xpm \
	pixmaps/trash_close_hrm_mark.xpm \
	pixmaps/trash_close_mark.xpm \
	pixmaps/trash_open.xpm \
	pixmaps/trash_open_hrm.xpm \
	pixmaps/trash_open_hrm_mark.xpm \
	pixmaps/trash_open_mark.xpm \
	pixmaps/tray_newmail_offline.xpm \
	pixmaps/tray_newmail.xpm \
	pixmaps/tray_newmarkedmail_offline.xpm \
	pixmaps/tray_newmarkedmail.xpm \
	pixmaps/tray_nomail_offline.xpm \
	pixmaps/tray_nomail.xpm \
	pixmaps/tray_unreadmail_offline.xpm \
	pixmaps/tray_unreadmail.xpm \
	pixmaps/tray_unreadmarkedmail_offline.xpm \
	pixmaps/tray_unreadmarkedmail.xpm \
	pixmaps/unread.xpm \
	pixmaps/read.xpm \
	pixmaps/up_arrow.xpm \
	pixmaps/watchthread.xpm \
	pixmaps/zoom_fit.xpm \
	pixmaps/zoom_in.xpm \
	pixmaps/zoom_out.xpm \
	pixmaps/zoom_width.xpm

IFLAGS = \
	-I$(srcdir)/common \
	-Icommon \
	-I$(srcdir)/gtk \
	-I$(srcdir)/etpan

claws_mail_LDFLAGS = \
	$(claws_mail_platform_ldflags) \
	-Lfence/zig-out/lib \
	-Wl,-z,nobtcfi

# The linker flag -Wl... disables "Mandatory enforcement of indirect
# branch targets" which I think(?) trips up my shit code.
# https://www.openbsd.org/innovations.html

etpan_library = etpan/libclawsetpan.la

claws_mail_DEPENDENCIES = $(claws_mail_deps) \
	$(etpan_library) \
	gtk/libclawsgtk.la

claws_mail_LDADD = \
	$(etpan_library) \
	gtk/libclawsgtk.la \
	$(INTLLIBS) \
	$(GTK_LIBS) \
	$(GNUTLS_LIBS) \
	$(NETTLE_LIBS) \
	-lpthread \
	$(LIBICONV) \
	$(LIBETPAN_LIBS) \
	$(SVG_LIBS) \
	-lfence

pixmapdir=$(datadir)/icons/hicolor/48x48/apps

AM_CPPFLAGS = \
	-DG_LOG_DOMAIN=\"Claws-Mail\" \
	-DLOCALEDIR=\""$(localedir)"\" \
	-DPIXMAP_DIR=\""$(pixmapdir)"\" \
	-DMANUALDIR=\""$(manualdir)"\" \
	-DTARGET_ALIAS=\""$(target_triplet)"\" \
	-DSYSCONFDIR=\""$(sysconfdir)"\" \
	-DDATAROOTDIR=\""$(datarootdir)"\" \
	$(IFLAGS) \
	$(GTK_CFLAGS) \
	$(GNUTLS_CFLAGS) \
	$(NETTLE_CFLAGS) \
	$(GPGME_CFLAGS) \
	$(LIBETPAN_CPPFLAGS) \
	$(VALGRIND_CFLAGS) \
	-Ifence/zig-out/include

#no-unused-function because of bison stuff
matcher_parser_lex.$(OBJEXT) : AM_CFLAGS += -Wno-unused-function
quote_fmt_lex.$(OBJEXT) : AM_CFLAGS += -Wno-unused-function

AM_YFLAGS = -d
