/*
 * Claws Mail -- a GTK based, lightweight, and fast e-mail client
 * Copyright (C) 1999-2024 the Claws Mail team and Hiroyuki Yamamoto
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

#include "defs.h"

#include <glib.h>
#include <glib/gi18n.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include "claws.h"
#include "main.h"
#include "inc.h"
#include "mbox.h"
#include "filesel.h"
#include "foldersel.h"
#include "gtkutils.h"
#include "manage_window.h"
#include "folder.h"
#include "utils.h"
#include "codeconv.h"
#include "alertpanel.h"

static GtkWidget *window;
static GtkWidget *src_entry;
static GtkWidget *file_entry;
static GtkWidget *src_button;
static GtkWidget *file_button;
static GtkWidget *ok_button;
static GtkWidget *cancel_button;
static gboolean export_ok; /* see export_mbox() return values */

static void export_create(void);
static void export_ok_cb(GtkWidget *widget, gpointer data);
static void export_cancel_cb(GtkWidget *widget, gpointer data);
static void export_srcsel_cb(GtkWidget *widget, gpointer data);
static void export_filesel_cb(GtkWidget *widget, gpointer data);
static gint delete_event(GtkWidget *widget, GdkEventAny *event, gpointer data);

gint export_mbox(FolderItem *default_src)
/* return values: -2 skipped/cancelled, -1 error, 0 OK */
{
	gchar *src_id = NULL;

	export_ok = -2;	// skipped or cancelled

	if (!window) {
		export_create();
	}
	else {
		gtk_widget_show(window);
	}

	gtk_window_set_modal(GTK_WINDOW(window), TRUE);
	change_dir(claws_get_startup_dir());

	if (default_src && default_src->path) {
		src_id = folder_item_get_identifier(default_src);
	}

	if (src_id) {
		gtk_entry_set_text(GTK_ENTRY(src_entry), src_id);
		g_free(src_id);
	} else {
		gtk_entry_set_text(GTK_ENTRY(src_entry), "");
	}
	gtk_entry_set_text(GTK_ENTRY(file_entry), "");
	gtk_widget_grab_focus(file_entry);

	manage_window_set_transient(GTK_WINDOW(window));

	gtk_main();

	gtk_widget_hide(window);
	gtk_window_set_modal(GTK_WINDOW(window), FALSE);

	return export_ok;
}

static void export_create(void)
{
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *desc_label;
	GtkWidget *table;
	GtkWidget *file_label;
	GtkWidget *src_label;
	GtkWidget *confirm_area;

	window = gtkut_window_new(GTK_WINDOW_TOPLEVEL, "export");
	gtk_window_set_title(GTK_WINDOW(window), _("Export to mbox file"));
	gtk_container_set_border_width(GTK_CONTAINER(window), 5);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_set_resizable(GTK_WINDOW(window), TRUE);
	gtk_window_set_type_hint(GTK_WINDOW(window), GDK_WINDOW_TYPE_HINT_DIALOG);
	g_signal_connect(G_OBJECT(window), "delete_event",
			 G_CALLBACK(delete_event), NULL);
	MANAGE_WINDOW_SIGNALS_CONNECT(window);

	vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
	gtk_container_add(GTK_CONTAINER(window), vbox);

	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 4);

	desc_label = gtk_label_new
		(_("Locate the folder to export and specify the mbox file."));
	gtk_label_set_line_wrap(GTK_LABEL(desc_label), TRUE);
	gtk_box_pack_start(GTK_BOX(hbox), desc_label, FALSE, FALSE, 0);

	table = gtk_grid_new();
	gtk_box_pack_start(GTK_BOX(vbox), table, FALSE, FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(table), 8);
	gtk_grid_set_row_spacing(GTK_GRID(table), 8);
	gtk_grid_set_column_spacing(GTK_GRID(table), 8);
	gtk_widget_set_size_request(table, 420, -1);

	src_label = gtk_label_new(_("Source folder:"));
	gtk_label_set_xalign(GTK_LABEL(src_label), 1.0);
	gtk_grid_attach(GTK_GRID(table), src_label, 0, 0, 1, 1);

	file_label = gtk_label_new(_("Mbox file:"));
	gtk_label_set_xalign(GTK_LABEL(file_label), 1.0);
	gtk_grid_attach(GTK_GRID(table), file_label, 0, 1, 1, 1);

	src_entry = gtk_entry_new();
	gtk_grid_attach(GTK_GRID(table), src_entry, 1, 0, 1, 1);
	gtk_widget_set_hexpand(src_entry, TRUE);
	gtk_widget_set_halign(src_entry, GTK_ALIGN_FILL);

	file_entry = gtk_entry_new();
	gtk_entry_set_activates_default(GTK_ENTRY(file_entry), TRUE);
	gtk_grid_attach(GTK_GRID(table), file_entry, 1, 1, 1, 1);
	gtk_widget_set_hexpand(file_entry, TRUE);
	gtk_widget_set_halign(file_entry, GTK_ALIGN_FILL);

	src_button = gtkut_get_browse_directory_btn(_("_Browse"));
	gtk_grid_attach(GTK_GRID(table), src_button, 2, 0, 1, 1);

	g_signal_connect(G_OBJECT(src_button), "clicked",
			 G_CALLBACK(export_srcsel_cb), NULL);

	file_button = gtkut_get_browse_file_btn(_("B_rowse"));
	gtk_grid_attach(GTK_GRID(table), file_button, 2, 1, 1, 1);

	g_signal_connect(G_OBJECT(file_button), "clicked",
			 G_CALLBACK(export_filesel_cb), NULL);

	gtkut_stock_button_set_create(&confirm_area,
				      &cancel_button, NULL, _("_Cancel"),
				      &ok_button, NULL, _("_OK"),
				      NULL, NULL, NULL);
	gtk_box_pack_end(GTK_BOX(vbox), confirm_area, FALSE, FALSE, 0);
	gtk_widget_grab_default(ok_button);

	g_signal_connect(G_OBJECT(ok_button), "clicked",
			 G_CALLBACK(export_ok_cb), NULL);
	g_signal_connect(G_OBJECT(cancel_button), "clicked",
			 G_CALLBACK(export_cancel_cb), NULL);

	gtk_widget_show_all(window);
}

static void export_ok_cb(GtkWidget *widget, gpointer data)
{
	const gchar *srcdir, *utf8mbox;
	FolderItem *src;
	gchar *mbox;

	srcdir = gtk_entry_get_text(GTK_ENTRY(src_entry));
	utf8mbox = gtk_entry_get_text(GTK_ENTRY(file_entry));

	if (!utf8mbox || !*utf8mbox) {
		alertpanel_error(_("Mbox file can't be left empty."));
		gtk_widget_grab_focus(file_entry);
		return;
	}
	if (!srcdir || !*srcdir) {
		alertpanel_error(_("Source folder can't be left empty."));
		gtk_widget_grab_focus(src_entry);
		return;
	}

	mbox = g_filename_from_utf8(utf8mbox, -1, NULL, NULL, NULL);
	if (!mbox) {
		g_warning("export_ok_cb(): failed to convert character set");
		mbox = g_strdup(utf8mbox);
	}

	src = folder_find_item_from_identifier(srcdir);
	if (!src) {
		alertpanel_error(_("Couldn't find the source folder."));
		gtk_widget_grab_focus(src_entry);
		g_free(mbox);
		return;
	} else {
		export_ok = export_to_mbox(src, mbox);
	}

	g_free(mbox);

	if (gtk_main_level() > 1)
		gtk_main_quit();
}

static void export_cancel_cb(GtkWidget *widget, gpointer data)
{
	if (gtk_main_level() > 1)
		gtk_main_quit();
}

static void export_filesel_cb(GtkWidget *widget, gpointer data)
{
	gchar *filename;

	filename = filesel_select_file_save(_("Select exporting file"), NULL);
	if (!filename) return;

	if (g_getenv ("G_BROKEN_FILENAMES")) {
		const gchar *oldstr = filename;
		filename = conv_codeset_strdup (filename,
						conv_get_locale_charset_str(),
						CS_UTF_8);
		if (!filename) {
			g_warning("export_filesel_cb(): failed to convert character set");
			filename = g_strdup(oldstr);
		}
		gtk_entry_set_text(GTK_ENTRY(file_entry), filename);
		g_free(filename);
	} else
		gtk_entry_set_text(GTK_ENTRY(file_entry), filename);
}

static void export_srcsel_cb(GtkWidget *widget, gpointer data)
{
	FolderItem *src;

	src = foldersel_folder_sel(NULL, FOLDER_SEL_ALL, NULL, FALSE,
			_("Select folder to export"));
	if (src && src->path)
		gtk_entry_set_text(GTK_ENTRY(src_entry), src->path);
}

static gint delete_event(GtkWidget *widget, GdkEventAny *event, gpointer data)
{
	export_cancel_cb(NULL, NULL);
	return TRUE;
}
