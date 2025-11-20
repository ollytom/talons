/*
 * Claws Mail -- a GTK based, lightweight, and fast e-mail client
 * Copyright (C) 2004-2021 the Claws Mail team and Hiroyuki Yamamoto
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

#include <stdio.h>
#include <stdlib.h>

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "utils.h"
#include "prefs_common.h"
#include "prefs_gtk.h"

#include "gtk/gtkutils.h"
#include "gtk/prefswindow.h"

#include "manage_window.h"

typedef struct _ExtProgPage
{
	PrefsPage page;

	GtkWidget *window;		/* do not modify */

	GtkWidget *uri_label;
	GtkWidget *uri_entry;

	GtkWidget *exteditor_label;
	GtkWidget *exteditor_entry;

	GtkWidget *astextviewer_label;
	GtkWidget *astextviewer_entry;
} ExtProgPage;

static void prefs_ext_prog_create_widget(PrefsPage *_page, GtkWindow *window,
			          gpointer data)
{
	ExtProgPage *prefs_ext_prog = (ExtProgPage *) _page;

	GtkWidget *table;
	GtkWidget *vbox;
	GtkWidget *hint_label;
	GtkWidget *table2;
	GtkWidget *uri_label;
	GtkWidget *uri_entry;
	GtkWidget *exteditor_label;
	GtkWidget *exteditor_entry;

	GtkWidget *astextviewer_label;
	GtkWidget *astextviewer_entry;
	int i = 0;

	table = gtk_grid_new();
	gtk_widget_show(table);
	gtk_container_set_border_width(GTK_CONTAINER(table), 8);
	gtk_grid_set_row_spacing(GTK_GRID(table), 4);
	gtk_grid_set_column_spacing(GTK_GRID(table), 8);

 	vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_widget_show(vbox);

	gtk_grid_attach(GTK_GRID(table), vbox, 0, 0, 1, 1);

	hint_label = gtk_label_new(_("%s will be replaced with file name / URI"));
	gtk_label_set_justify (GTK_LABEL (hint_label), GTK_JUSTIFY_LEFT);
	gtkut_widget_set_small_font_size (hint_label);
	gtk_widget_show(hint_label);
	gtk_box_pack_start(GTK_BOX (vbox),
			   hint_label, FALSE, FALSE, 4);

#ifdef G_OS_UNIX
	hint_label = gtk_label_new(_("For the text editor, %w will be replaced with GtkSocket ID"));
	gtk_label_set_justify (GTK_LABEL (hint_label), GTK_JUSTIFY_LEFT);
	gtkut_widget_set_small_font_size (hint_label);
	gtk_widget_show(hint_label);
	gtk_box_pack_start(GTK_BOX (vbox),
			   hint_label, FALSE, FALSE, 4);
#endif

	table2 = gtk_grid_new();
	gtk_widget_show(table2);
	gtk_container_set_border_width(GTK_CONTAINER(table2), 8);
	gtk_grid_set_row_spacing(GTK_GRID(table2), 4);
	gtk_grid_set_column_spacing(GTK_GRID(table2), 8);

	gtk_grid_attach(GTK_GRID(table), table2, 0, 1, 1, 1);
	gtk_widget_set_hexpand(table2, TRUE);
	gtk_widget_set_halign(table2, GTK_ALIGN_FILL);

	uri_label = gtk_label_new ("Web browser");
	gtk_widget_show(uri_label);
	i++;
	gtk_label_set_justify(GTK_LABEL (uri_label), GTK_JUSTIFY_RIGHT);
	gtk_label_set_xalign(GTK_LABEL (uri_label), 1.0);
	gtk_grid_attach(GTK_GRID(table2), uri_label, 0, i, 1, 1);
	uri_entry = gtk_entry_new();
	gtk_widget_show(uri_entry);
	gtk_entry_set_text(GTK_ENTRY(uri_entry), prefs_common.uri_cmd ? prefs_common.uri_cmd : "");
	gtk_grid_attach(GTK_GRID(table2), uri_entry, 1, i, 1, 1);

	exteditor_label = gtk_label_new ("Text editor");
	gtk_widget_show(exteditor_label);
	i++;
	gtk_label_set_justify(GTK_LABEL (exteditor_label), GTK_JUSTIFY_RIGHT);
	gtk_label_set_xalign(GTK_LABEL (exteditor_label), 1.0);
	gtk_grid_attach(GTK_GRID(table2), exteditor_label, 0, i, 1, 1);
	exteditor_entry = gtk_entry_new();
	gtk_widget_show(exteditor_entry);
	gtk_entry_set_text(GTK_ENTRY(exteditor_entry), prefs_common.ext_editor_cmd ? prefs_common.ext_editor_cmd : "");
	gtk_grid_attach(GTK_GRID(table2), exteditor_entry, 1, i, 1, 1);

	astextviewer_label = gtk_label_new("Display as text");
	gtk_widget_show(astextviewer_label);
	i++;
	gtk_label_set_justify(GTK_LABEL (astextviewer_label), GTK_JUSTIFY_RIGHT);
	gtk_label_set_xalign(GTK_LABEL (astextviewer_label), 1.0);
	gtk_grid_attach(GTK_GRID(table2), astextviewer_label, 0, i, 1, 1);
	astextviewer_entry = gtk_entry_new();
	gtk_widget_show(astextviewer_entry);
	CLAWS_SET_TIP(astextviewer_entry,
			     _("This option enables MIME parts to be displayed in the "
 			       "message view via a script when using the 'Display as text' "
			       "contextual menu item"));
	gtk_grid_attach(GTK_GRID(table2), astextviewer_entry, 1, i, 1, 1);
	gtk_widget_set_hexpand(astextviewer_entry, TRUE);
	gtk_widget_set_halign(astextviewer_entry, GTK_ALIGN_FILL);
	gtk_entry_set_text(GTK_ENTRY(astextviewer_entry),
			   prefs_common.mime_textviewer ? prefs_common.mime_textviewer : "");

	prefs_ext_prog->window			= GTK_WIDGET(window);
	prefs_ext_prog->uri_entry		= uri_entry;
	prefs_ext_prog->exteditor_entry		= exteditor_entry;
	prefs_ext_prog->astextviewer_entry	= astextviewer_entry;
	prefs_ext_prog->page.widget = table;
}

static void prefs_ext_prog_save(PrefsPage *_page)
{
	ExtProgPage *ext_prog = (ExtProgPage *) _page;

	prefs_common.uri_cmd = gtk_editable_get_chars
		(GTK_EDITABLE(ext_prog->uri_entry), 0, -1);
	prefs_common.ext_editor_cmd = gtk_editable_get_chars
		(GTK_EDITABLE(ext_prog->exteditor_entry), 0, -1);
	prefs_common.mime_textviewer = gtk_editable_get_chars
		(GTK_EDITABLE(ext_prog->astextviewer_entry), 0, -1);
}

static void prefs_ext_prog_destroy_widget(PrefsPage *_page)
{
	/* ExtProgPage *ext_prog = (ExtProgPage *) _page; */

}

ExtProgPage *prefs_ext_prog;

void prefs_ext_prog_init(void)
{
	ExtProgPage *page;
	static gchar *path[3];

	path[0] = _("Message View");
	path[1] = _("External Programs");
	path[2] = NULL;

	page = g_new0(ExtProgPage, 1);
	page->page.path = path;
	page->page.create_widget = prefs_ext_prog_create_widget;
	page->page.destroy_widget = prefs_ext_prog_destroy_widget;
	page->page.save_page = prefs_ext_prog_save;
	page->page.weight = 155.0;
	prefs_gtk_register_page((PrefsPage *) page);
	prefs_ext_prog = page;
}

void prefs_ext_prog_done(void)
{
	prefs_gtk_unregister_page((PrefsPage *) prefs_ext_prog);
	g_free(prefs_ext_prog);
}
