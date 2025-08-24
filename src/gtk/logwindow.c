/*
 * Claws Mail -- a GTK based, lightweight, and fast e-mail client
 * Copyright (C) 1999-2019 the Claws Mail team and Hiroyuki Yamamoto
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#include "claws-features.h"
#endif

#include <glib.h>
#include <glib/gi18n.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include "logwindow.h"
#include "utils.h"
#include "gtkutils.h"
#include "log.h"
#include "hooks.h"
#include "prefs_common.h"

static void hide_cb				(GtkWidget	*widget,
						 LogWindow	*logwin);
static void size_allocate_cb	(GtkWidget *widget,
					 GtkAllocation *allocation,
					 gpointer data);
static gboolean log_window_append		(gpointer 	 source,
						 gpointer   	 data);
static void log_window_clear			(GtkWidget	*widget,
						 LogWindow	*logwin);
static void log_window_popup_menu_extend	(GtkTextView	*textview,
						 GtkMenu	*menu,
						 LogWindow	*logwin);

/*!
 *\brief	Save Gtk object size to prefs dataset
 */
static void size_allocate_cb(GtkWidget *widget,
					 GtkAllocation *allocation,
					 gpointer data)
{
	gint *prefs_logwin_width = NULL;
	gint *prefs_logwin_height = NULL;
	LogInstance instance = GPOINTER_TO_INT(data);

	cm_return_if_fail(allocation != NULL);

	get_log_prefs(instance, &prefs_logwin_width, &prefs_logwin_height);
	cm_return_if_fail(prefs_logwin_width != NULL);
	cm_return_if_fail(prefs_logwin_height != NULL);

	gtk_window_get_size(GTK_WINDOW(widget),
		prefs_logwin_width, prefs_logwin_height);
}

LogWindow *log_window_create(LogInstance instance)
{
	LogWindow *logwin;
	GtkWidget *window;
	GtkWidget *scrolledwin;
	GtkWidget *text;
	GtkTextBuffer *buffer;
	GtkTextIter iter;
	static GdkGeometry geometry;
	gint *prefs_logwin_width = NULL;
	gint *prefs_logwin_height = NULL;

	debug_print("Creating log window...\n");

	get_log_prefs(instance, &prefs_logwin_width, &prefs_logwin_height);
	cm_return_val_if_fail(prefs_logwin_width != NULL, NULL);
	cm_return_val_if_fail(prefs_logwin_height != NULL, NULL);

	logwin = g_new0(LogWindow, 1);

	window = gtkut_window_new(GTK_WINDOW_TOPLEVEL, "logwindow");
	gtk_window_set_title(GTK_WINDOW(window), get_log_title(instance));
	gtk_window_set_resizable(GTK_WINDOW(window), TRUE);
	gtk_window_set_type_hint(GTK_WINDOW(window), GDK_WINDOW_TYPE_HINT_DIALOG);
	g_signal_connect(G_OBJECT(window), "delete_event",
			 G_CALLBACK(gtk_widget_hide_on_delete), NULL);
	g_signal_connect_after(G_OBJECT(window), "hide",
			 G_CALLBACK(hide_cb), logwin);
	gtk_widget_realize(window);

	scrolledwin = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledwin),
				       GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolledwin),
					    GTK_SHADOW_IN);
	gtk_container_add(GTK_CONTAINER(window), scrolledwin);
	gtk_widget_show(scrolledwin);

	text = gtk_text_view_new();
	gtk_text_view_set_editable(GTK_TEXT_VIEW(text), FALSE);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text), GTK_WRAP_WORD);
	logwin->buffer = buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text));

	g_object_ref(G_OBJECT(logwin->buffer));
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(text), NULL);
	logwin->hidden = TRUE;
	logwin->never_shown = TRUE;

	gtk_text_buffer_get_start_iter(buffer, &iter);
	logwin->end_mark = gtk_text_buffer_create_mark(buffer, "end", &iter, FALSE);

	g_signal_connect(G_OBJECT(text), "populate-popup",
			 G_CALLBACK(log_window_popup_menu_extend), logwin);
	gtk_container_add(GTK_CONTAINER(scrolledwin), text);
	gtk_widget_show(text);

	g_signal_connect(G_OBJECT(window), "size_allocate",
			 G_CALLBACK(size_allocate_cb), GINT_TO_POINTER(instance));

	if (!geometry.min_height) {
		geometry.min_width = 520;
		geometry.min_height = 400;
	}

	gtk_window_set_geometry_hints(GTK_WINDOW(window), NULL, &geometry,
				      GDK_HINT_MIN_SIZE);
	gtk_window_set_default_size(GTK_WINDOW(window), *prefs_logwin_width,
				    *prefs_logwin_height);

	logwin->window = window;
	logwin->scrolledwin = scrolledwin;
	logwin->text = text;
	logwin->hook_id = hooks_register_hook(get_log_hook(instance), log_window_append, logwin);
	logwin->has_error_capability = get_log_error_capability(instance);

	return logwin;
}

#define LOG_COLORS 8

void log_window_init(LogWindow *logwin)
{
	GtkTextBuffer *buffer;

	logwin->msg_color = &prefs_common.color[COL_LOG_MSG];
	logwin->warn_color = &prefs_common.color[COL_LOG_WARN];
	logwin->error_color = &prefs_common.color[COL_LOG_ERROR];
	logwin->in_color = &prefs_common.color[COL_LOG_IN];
	logwin->out_color = &prefs_common.color[COL_LOG_OUT];
	logwin->status_ok_color = &prefs_common.color[COL_LOG_STATUS_OK];
	logwin->status_nok_color = &prefs_common.color[COL_LOG_STATUS_NOK];
	logwin->status_skip_color = &prefs_common.color[COL_LOG_STATUS_SKIP];

	buffer = logwin->buffer;
	gtk_text_buffer_create_tag(buffer, "message",
				   "foreground-rgba", logwin->msg_color,
				   NULL);
	gtk_text_buffer_create_tag(buffer, "warn",
				   "foreground-rgba", logwin->warn_color,
				   NULL);
	logwin->error_tag = gtk_text_buffer_create_tag(buffer, "error",
				   "foreground-rgba", logwin->error_color,
				   NULL);
	gtk_text_buffer_create_tag(buffer, "input",
				   "foreground-rgba", logwin->in_color,
				   NULL);
	gtk_text_buffer_create_tag(buffer, "output",
				   "foreground-rgba", logwin->out_color,
				   NULL);
	gtk_text_buffer_create_tag(buffer, "status_ok",
				   "foreground-rgba", logwin->status_ok_color,
				   NULL);
	gtk_text_buffer_create_tag(buffer, "status_nok",
				   "foreground-rgba", logwin->status_nok_color,
				   NULL);
	gtk_text_buffer_create_tag(buffer, "status_skip",
				   "foreground-rgba", logwin->status_skip_color,
				   NULL);
}

#undef LOG_COLORS

void log_window_show(LogWindow *logwin)
{
	GtkTextView *text = GTK_TEXT_VIEW(logwin->text);
	GtkTextBuffer *buffer = logwin->buffer;
	GtkTextMark *mark;

	logwin->hidden = FALSE;

	if (logwin->never_shown)
		gtk_text_view_set_buffer(GTK_TEXT_VIEW(logwin->text), logwin->buffer);

	logwin->never_shown = FALSE;

	mark = gtk_text_buffer_get_mark(buffer, "end");
	gtk_text_view_scroll_mark_onscreen(text, mark);

	gtk_window_deiconify(GTK_WINDOW(logwin->window));
	gtk_widget_show(logwin->window);
	gtk_window_present(GTK_WINDOW(logwin->window));
}

static void log_window_jump_to_error(LogWindow *logwin)
{
	GtkTextIter iter;
	gtk_text_buffer_get_end_iter(logwin->buffer, &iter);
	if (!gtk_text_iter_backward_to_tag_toggle(&iter, logwin->error_tag))
		return;

	gtk_text_iter_backward_line(&iter);
	gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(logwin->text), &iter, 0, TRUE, 0, 0);
}

void log_window_show_error(LogWindow *logwin)
{
	log_window_show(logwin);
	log_window_jump_to_error(logwin);
}

static gboolean log_window_append(gpointer source, gpointer data)
{
	LogText *logtext = (LogText *) source;
	LogWindow *logwindow = (LogWindow *) data;
	GtkTextView *text;
	GtkTextBuffer *buffer;
	GtkTextIter iter;
	gchar *head = NULL;
	const gchar *tag;

	cm_return_val_if_fail(logtext != NULL, TRUE);
	cm_return_val_if_fail(logtext->text != NULL, TRUE);
	cm_return_val_if_fail(logwindow != NULL, FALSE);

	text = GTK_TEXT_VIEW(logwindow->text);
	buffer = logwindow->buffer;
	gtk_text_buffer_get_iter_at_offset(buffer, &iter, -1);

	switch (logtext->type) {
	case LOG_MSG:
		tag = "message";
		head = "* ";
		break;
	case LOG_WARN:
		tag = "warn";
		head = "** ";
		break;
	case LOG_ERROR:
		tag = "error";
		head = "*** ";
		logwindow->has_error = TRUE;
		break;
	case LOG_STATUS_OK:
		tag = "status_ok";
		head = "> ";
		break;
	case LOG_STATUS_NOK:
		tag = "status_nok";
		head = "> ";
		break;
	case LOG_STATUS_SKIP:
		tag = "status_skip";
		head = "> skipped: ";
		break;
	default:
		tag = NULL;
		break;
	}

	if (logtext->instance == LOG_PROTOCOL) {
		if (tag == NULL) {
			if (strstr(logtext->text, "] POP>")
			||  strstr(logtext->text, "] IMAP>")
			||  strstr(logtext->text, "] SMTP>")
			||  strstr(logtext->text, "] ESMTP>")
			||  strstr(logtext->text, "] NNTP>"))
				tag = "output";
			if (strstr(logtext->text, "] POP<")
			||  strstr(logtext->text, "] IMAP<")
			||  strstr(logtext->text, "] SMTP<")
			||  strstr(logtext->text, "] ESMTP<")
			||  strstr(logtext->text, "] NNTP<"))
				tag = "input";
		}
	}

	if (head)
		gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, head, -1,
							 tag, NULL);

	if (!g_utf8_validate(logtext->text, -1, NULL)) {
		gchar * mybuf = g_malloc(strlen(logtext->text)*2 +1);
		conv_localetodisp(mybuf, strlen(logtext->text)*2 +1, logtext->text);
		gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, mybuf, -1,
							 tag, NULL);
		g_free(mybuf);
	} else {
		gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, logtext->text, -1,
							 tag, NULL);
	}
	gtk_text_buffer_get_start_iter(buffer, &iter);

	if (!logwindow->hidden) {
		GtkAdjustment *vadj = gtk_scrollable_get_vadjustment(GTK_SCROLLABLE(text));
		gfloat upper = gtk_adjustment_get_upper(vadj) -
		    gtk_adjustment_get_page_size(vadj);
		gfloat value = gtk_adjustment_get_value(vadj);
		if (value == upper ||
		    (upper - value < 16 && value < 8))
			gtk_text_view_scroll_mark_onscreen(text, logwindow->end_mark);
	}

	return FALSE;
}

static void hide_cb(GtkWidget *widget, LogWindow *logwin)
{
	logwin->hidden = TRUE;
}

static void log_window_clear(GtkWidget *widget, LogWindow *logwin)
{
	GtkTextBuffer *textbuf = logwin->buffer;
	GtkTextIter start_iter, end_iter;

	gtk_text_buffer_get_start_iter(textbuf, &start_iter);
	gtk_text_buffer_get_end_iter(textbuf, &end_iter);
	gtk_text_buffer_delete(textbuf, &start_iter, &end_iter);
}

static void log_window_go_to_last_error(GtkWidget *widget, LogWindow *logwin)
{
	log_window_jump_to_error(logwin);
}

static void log_window_popup_menu_extend(GtkTextView *textview,
   			GtkMenu *menu, LogWindow *logwin)
{
	GtkWidget *menuitem;

	cm_return_if_fail(menu != NULL);
	cm_return_if_fail(GTK_IS_MENU_SHELL(menu));

	menuitem = gtk_separator_menu_item_new();
	gtk_menu_shell_prepend(GTK_MENU_SHELL(menu), menuitem);
	gtk_widget_show(menuitem);

	if (logwin->has_error_capability) {
		menuitem = gtk_menu_item_new_with_mnemonic(_("_Go to last error"));
		g_signal_connect(G_OBJECT(menuitem), "activate",
				 G_CALLBACK(log_window_go_to_last_error), logwin);
		gtk_menu_shell_prepend(GTK_MENU_SHELL(menu), menuitem);
		gtk_widget_show(menuitem);
	}

	menuitem = gtk_menu_item_new_with_mnemonic(_("Clear _Log"));
	g_signal_connect(G_OBJECT(menuitem), "activate",
			 G_CALLBACK(log_window_clear), logwin);
	gtk_menu_shell_prepend(GTK_MENU_SHELL(menu), menuitem);
	gtk_widget_show(menuitem);
}

