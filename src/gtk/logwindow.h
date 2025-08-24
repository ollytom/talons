/*
 * Claws Mail -- a GTK based, lightweight, and fast e-mail client
 * Copyright (C) 1999-2012 Hiroyuki Yamamoto and the Claws Mail team
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
 *
 */

#ifndef __LOGWINDOW_H__
#define __LOGWINDOW_H__

#include <glib.h>
#include <gtk/gtk.h>

#include "log.h"

typedef struct _LogWindow	LogWindow;

struct _LogWindow
{
	GtkWidget *window;
	GtkWidget *scrolledwin;
	GtkWidget *text;

	GdkRGBA *msg_color;
	GdkRGBA *warn_color;
	GdkRGBA *error_color;
	GdkRGBA *in_color;
	GdkRGBA *out_color;
	GdkRGBA *status_ok_color;
	GdkRGBA *status_nok_color;
	GdkRGBA *status_skip_color;

	gulong 	 hook_id;
	GtkTextBuffer *buffer;
	GtkTextTag *error_tag;
	GtkTextMark *end_mark;
	gboolean hidden;
	gboolean never_shown;
	gboolean has_error;
	gboolean has_error_capability;
};

LogWindow *log_window_create(LogInstance instance);
void log_window_init(LogWindow *logwin);
void log_window_show(LogWindow *logwin);
void log_window_show_error(LogWindow *logwin);

#endif /* __LOGWINDOW_H__ */
