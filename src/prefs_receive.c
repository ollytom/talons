/*
 * Claws Mail -- a GTK based, lightweight, and fast e-mail client
 * Copyright (C) 2005-2024 the Claws Mail team and Colin Leroy
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

#include "prefs_common.h"
#include "prefs_receive.h"
#include "prefs_gtk.h"
#include "inc.h"

#include "gtk/gtkutils.h"
#include "gtk/prefswindow.h"
#include "gtk/menu.h"

#include "manage_window.h"
#include "combobox.h"

typedef struct _ReceivePage
{
	PrefsPage page;

	GtkWidget *window;

	GtkWidget *checkbtn_incext;
	GtkWidget *entry_incext;
	GtkWidget *checkbtn_newmail_auto;
	GtkWidget *checkbtn_newmail_manu;
	GtkWidget *entry_newmail_notify_cmd;
	GtkWidget *hbox_newmail_notify;
} ReceivePage;

ReceivePage *prefs_receive;

static void prefs_common_recv_dialog_newmail_notify_toggle_cb(GtkWidget *w, gpointer data)
{
	gboolean toggled;

	toggled = gtk_toggle_button_get_active
			(GTK_TOGGLE_BUTTON(prefs_receive->checkbtn_newmail_manu)) ||
		  gtk_toggle_button_get_active
			(GTK_TOGGLE_BUTTON(prefs_receive->checkbtn_newmail_auto));
	gtk_widget_set_sensitive(prefs_receive->hbox_newmail_notify, toggled);
}

static void prefs_receive_create_widget(PrefsPage *_page, GtkWindow *window,
			       	  gpointer data)
{
	ReceivePage *prefs_receive = (ReceivePage *) _page;

	GtkWidget *vbox1;
	GtkWidget *vbox2;
	GtkWidget *checkbtn_incext;
	GtkWidget *hbox;
	GtkWidget *label_incext;
	GtkWidget *entry_incext;

	GtkWidget *frame;
	GtkWidget *vbox3;
	GtkWidget *hbox_newmail_notify;
	GtkWidget *checkbtn_newmail_auto;
	GtkWidget *checkbtn_newmail_manu;
	GtkWidget *entry_newmail_notify_cmd;
	GtkWidget *label_newmail_notify_cmd;
	GtkWidget *label_newmail_notify_cmd_syntax;

	vbox1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, VSPACING);
	gtk_widget_show (vbox1);
	gtk_container_set_border_width (GTK_CONTAINER (vbox1), VBOX_BORDER);

	/* Use of external incorporation program */
	vbox2 = gtkut_get_options_frame(vbox1, &frame, _("External incorporation program"));

	PACK_CHECK_BUTTON (vbox2, checkbtn_incext,
			   _("Use external program for receiving mail"));

	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
	gtk_widget_show (hbox);
	gtk_box_pack_start (GTK_BOX (vbox2), hbox, FALSE, FALSE, 0);
	SET_TOGGLE_SENSITIVITY (checkbtn_incext, hbox);

	label_incext = gtk_label_new (_("Command"));
	gtk_widget_show (label_incext);
	gtk_box_pack_start (GTK_BOX (hbox), label_incext, FALSE, FALSE, 0);

	entry_incext = gtk_entry_new ();
	gtk_widget_show (entry_incext);
	gtk_box_pack_start (GTK_BOX (hbox), entry_incext, TRUE, TRUE, 0);

 	vbox3 = gtkut_get_options_frame(vbox1, &frame, _("Run command after receiving new mail"));

	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
	gtk_widget_show (hbox);
	PACK_CHECK_BUTTON (hbox, checkbtn_newmail_auto,
			   _("after automatic check"));
	PACK_CHECK_BUTTON (hbox, checkbtn_newmail_manu,
			   _("after manual check"));
	gtk_box_pack_start (GTK_BOX(vbox3), hbox, FALSE, FALSE, 0);

	hbox_newmail_notify = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
	gtk_widget_show (hbox_newmail_notify);
	gtk_box_pack_start (GTK_BOX (vbox3), hbox_newmail_notify, FALSE,
			    FALSE, 0);

	label_newmail_notify_cmd = gtk_label_new (_("Command to execute"));
	gtk_label_set_justify(GTK_LABEL(label_newmail_notify_cmd),
			      GTK_JUSTIFY_RIGHT);
	gtk_widget_show (label_newmail_notify_cmd);
	gtk_box_pack_start (GTK_BOX (hbox_newmail_notify),
			    label_newmail_notify_cmd, FALSE, FALSE, 0);

	entry_newmail_notify_cmd = gtk_entry_new ();
	gtk_widget_show (entry_newmail_notify_cmd);
	gtk_box_pack_start (GTK_BOX (hbox_newmail_notify),
			    entry_newmail_notify_cmd, TRUE, TRUE, 0);

	label_newmail_notify_cmd_syntax =  gtk_label_new (_("Use %d as number of new mails"));
	gtk_widget_show (label_newmail_notify_cmd_syntax);
	gtkut_widget_set_small_font_size (label_newmail_notify_cmd_syntax);
	gtk_box_pack_start (GTK_BOX (hbox_newmail_notify),
			    label_newmail_notify_cmd_syntax, FALSE, FALSE, 0);

	gtk_widget_set_sensitive(hbox_newmail_notify,
				 prefs_common.newmail_notify_auto ||
				 prefs_common.newmail_notify_manu);

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbtn_newmail_auto),
		prefs_common.newmail_notify_auto);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbtn_newmail_manu),
		prefs_common.newmail_notify_manu);
	gtk_entry_set_text(GTK_ENTRY(entry_newmail_notify_cmd),
		prefs_common.newmail_notify_cmd);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbtn_incext),
		prefs_common.use_extinc);

	gtk_entry_set_text(GTK_ENTRY(entry_incext),
		prefs_common.extinc_cmd);

	prefs_receive->window = GTK_WIDGET(window);
	prefs_receive->checkbtn_incext = checkbtn_incext;
	prefs_receive->entry_incext = entry_incext;
	prefs_receive->checkbtn_newmail_auto = checkbtn_newmail_auto;
	prefs_receive->checkbtn_newmail_manu = checkbtn_newmail_manu;
	prefs_receive->entry_newmail_notify_cmd = entry_newmail_notify_cmd;
	prefs_receive->hbox_newmail_notify = hbox_newmail_notify;

	prefs_receive->page.widget = vbox1;

	g_signal_connect(G_OBJECT(checkbtn_newmail_auto), "toggled",
			 G_CALLBACK(prefs_common_recv_dialog_newmail_notify_toggle_cb),
			 NULL);
	g_signal_connect(G_OBJECT(checkbtn_newmail_manu), "toggled",
			 G_CALLBACK(prefs_common_recv_dialog_newmail_notify_toggle_cb),
			 NULL);
}

static void prefs_receive_save(PrefsPage *_page)
{
	ReceivePage *page = (ReceivePage *) _page;
	gchar *tmp;

	prefs_common.use_extinc = gtk_toggle_button_get_active(
		GTK_TOGGLE_BUTTON(page->checkbtn_incext));
	prefs_common.newmail_notify_auto = gtk_toggle_button_get_active(
		GTK_TOGGLE_BUTTON(page->checkbtn_newmail_auto));
	prefs_common.newmail_notify_manu = gtk_toggle_button_get_active(
		GTK_TOGGLE_BUTTON(page->checkbtn_newmail_manu));

	tmp = gtk_editable_get_chars(GTK_EDITABLE(page->entry_incext), 0, -1);
	g_free(prefs_common.extinc_cmd);
	prefs_common.extinc_cmd = tmp;

	tmp = gtk_editable_get_chars(GTK_EDITABLE(page->entry_newmail_notify_cmd), 0, -1);
	g_free(prefs_common.newmail_notify_cmd);
	prefs_common.newmail_notify_cmd = tmp;
}

static void prefs_receive_destroy_widget(PrefsPage *_page) {}

void prefs_receive_init(void)
{
	ReceivePage *page;
	static gchar *path[3];

	path[0] = _("Mail Handling");
	path[1] = _("Receiving");
	path[2] = NULL;

	page = g_new0(ReceivePage, 1);
	page->page.path = path;
	page->page.create_widget = prefs_receive_create_widget;
	page->page.destroy_widget = prefs_receive_destroy_widget;
	page->page.save_page = prefs_receive_save;
	page->page.weight = 200.0;
	prefs_gtk_register_page((PrefsPage *) page);
	prefs_receive = page;
}

void prefs_receive_done(void)
{
	prefs_gtk_unregister_page((PrefsPage *) prefs_receive);
	g_free(prefs_receive);
}
