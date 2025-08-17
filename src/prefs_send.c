/*
 * Claws Mail -- a GTK based, lightweight, and fast e-mail client
 * Copyright (C) 2005-2019 Colin Leroy & The Claws Mail Team
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

#include "defs.h"

#include <stdio.h>
#include <stdlib.h>

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "prefs_common.h"
#include "prefs_gtk.h"
#include "gtk/menu.h"

#include "gtk/gtkutils.h"
#include "gtk/prefswindow.h"
#include "gtk/combobox.h"

#include "manage_window.h"

typedef struct _SendPage
{
	PrefsPage page;

	GtkWidget *window;

	GtkWidget *checkbtn_savemsg;
	GtkWidget *checkbtn_confirm_send_queued_messages;
	GtkWidget *checkbtn_never_send_retrcpt;
	GtkWidget *checkbtn_senddialog;
	GtkWidget *checkbtn_warn_empty_subj;
	GtkWidget *checkbtn_warn_multiple_recipients;
	GtkWidget *spinbtn_warn_multiple_recipients;
	GtkWidget *combobox_encoding_method;
} SendPage;

typedef struct _combobox_sel_by_data_ctx {
	GtkComboBox *combobox;
	gchar *data;
} ComboboxSelCtx;

static void checkbtn_warn_multiple_recipients_toggled(GtkToggleButton *button,
		gpointer user_data)
{
	gboolean active = gtk_toggle_button_get_active(button);
	GtkWidget *spin = GTK_WIDGET(user_data);

	gtk_widget_set_sensitive(spin, active);
}

static void prefs_send_create_widget(PrefsPage *_page, GtkWindow *window,
			       	  gpointer data)
{
	SendPage *prefs_send = (SendPage *) _page;
	GtkWidget *frame;
	GtkWidget *vbox1, *vbox2, *hbox1;
	GtkWidget *checkbtn_savemsg;
	GtkListStore *optmenu;
	GtkTreeIter iter;
	GtkWidget *combobox_encoding;
	GtkWidget *label_encoding;
	GtkWidget *checkbtn_senddialog;
	GtkWidget *checkbtn_confirm_send_queued_messages;
	GtkWidget *checkbtn_never_send_retrcpt;
	GtkWidget *checkbtn_warn_empty_subj;
	GtkWidget *checkbtn_warn_multiple_recipients;
	GtkWidget *spinbtn_warn_multiple_recipients;
	GtkWidget *table;

	vbox1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, VSPACING);
	gtk_widget_show (vbox1);
	gtk_container_set_border_width (GTK_CONTAINER (vbox1), VBOX_BORDER);

	/* messages frame */
	vbox2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_widget_show (vbox2);

	PACK_CHECK_BUTTON(vbox2, checkbtn_savemsg,
			_("Save sent messages"));

	PACK_CHECK_BUTTON(vbox2, checkbtn_never_send_retrcpt,
			_("Never send Return Receipts"));

	/* encoding table */
	table = gtk_grid_new();
	gtk_widget_show(table);
	gtk_container_add (GTK_CONTAINER (vbox2), table);
	gtk_grid_set_row_spacing(GTK_GRID(table), 4);
	gtk_grid_set_column_spacing(GTK_GRID(table), 8);

	PACK_FRAME (vbox1, frame, _("Messages"))
	gtk_container_set_border_width(GTK_CONTAINER(vbox2), 8);
	gtk_container_add(GTK_CONTAINER(frame), vbox2);

	/* interface frame */
	vbox2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_widget_show (vbox2);

	PACK_CHECK_BUTTON(vbox2, checkbtn_confirm_send_queued_messages,
			_("Confirm before sending queued messages"));

	PACK_CHECK_BUTTON(vbox2, checkbtn_senddialog,
			_("Show send dialog"));

	PACK_CHECK_BUTTON(vbox2, checkbtn_warn_empty_subj,
			_("Warn when Subject is empty"));

	hbox1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_widget_show (hbox1);

	PACK_CHECK_BUTTON(hbox1, checkbtn_warn_multiple_recipients,
			_("Warn when sending to more recipients than"));

	spinbtn_warn_multiple_recipients = gtk_spin_button_new_with_range(2, 999, 1);
	gtk_widget_show(spinbtn_warn_multiple_recipients);
	gtk_box_pack_start(GTK_BOX(hbox1), spinbtn_warn_multiple_recipients, FALSE, FALSE, 0);
	g_signal_connect(G_OBJECT(checkbtn_warn_multiple_recipients), "toggled",
			G_CALLBACK(checkbtn_warn_multiple_recipients_toggled),
			spinbtn_warn_multiple_recipients);

	gtk_box_pack_start(GTK_BOX(vbox2), hbox1, FALSE, FALSE, 0);

	PACK_FRAME (vbox1, frame, _("Interface"))
	gtk_container_set_border_width(GTK_CONTAINER(vbox2), 8);
	gtk_container_add(GTK_CONTAINER(frame), vbox2);

	label_encoding = gtk_label_new (_("Transfer encoding"));
	gtk_widget_show (label_encoding);
	gtk_label_set_justify(GTK_LABEL(label_encoding), GTK_JUSTIFY_RIGHT);
	gtk_label_set_xalign(GTK_LABEL(label_encoding), 1.0);
	gtk_grid_attach(GTK_GRID(table), label_encoding, 0, 2, 1, 1);

	combobox_encoding = gtkut_sc_combobox_create(NULL, FALSE);
	gtk_widget_show (combobox_encoding);
	CLAWS_SET_TIP(combobox_encoding,
			     _("Specify Content-Transfer-Encoding used when"
		   	       " message body contains non-ASCII characters"));
	gtk_grid_attach(GTK_GRID(table), combobox_encoding, 1, 2, 1, 1);

	optmenu = GTK_LIST_STORE(gtk_combo_box_get_model(
				GTK_COMBO_BOX(combobox_encoding)));

	COMBOBOX_ADD(optmenu, _("Automatic"),	 CTE_AUTO);
	COMBOBOX_ADD(optmenu, NULL, 0);
	COMBOBOX_ADD(optmenu, "base64",		 CTE_BASE64);
	COMBOBOX_ADD(optmenu, "quoted-printable", CTE_QUOTED_PRINTABLE);
	COMBOBOX_ADD(optmenu, "8bit",		 CTE_8BIT);

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbtn_savemsg),
		prefs_common.savemsg);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbtn_confirm_send_queued_messages),
		prefs_common.confirm_send_queued_messages);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbtn_never_send_retrcpt),
		prefs_common.never_send_retrcpt);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbtn_senddialog),
		!prefs_common.send_dialog_invisible);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbtn_warn_empty_subj),
		prefs_common.warn_empty_subj);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbtn_warn_multiple_recipients), prefs_common.warn_sending_many_recipients_num > 0);

	if (prefs_common.warn_sending_many_recipients_num > 0)
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinbtn_warn_multiple_recipients),
				prefs_common.warn_sending_many_recipients_num);
	else
		gtk_widget_set_sensitive(spinbtn_warn_multiple_recipients, FALSE);

	combobox_select_by_data(GTK_COMBO_BOX(combobox_encoding),
		prefs_common.encoding_method);

	prefs_send->window = GTK_WIDGET(window);

	prefs_send->checkbtn_savemsg = checkbtn_savemsg;
	prefs_send->checkbtn_confirm_send_queued_messages = checkbtn_confirm_send_queued_messages;
 	prefs_send->checkbtn_never_send_retrcpt = checkbtn_never_send_retrcpt;
	prefs_send->checkbtn_senddialog = checkbtn_senddialog;
	prefs_send->checkbtn_warn_empty_subj = checkbtn_warn_empty_subj;
	prefs_send->checkbtn_warn_multiple_recipients = checkbtn_warn_multiple_recipients;
	prefs_send->spinbtn_warn_multiple_recipients = spinbtn_warn_multiple_recipients;
	prefs_send->combobox_encoding_method = combobox_encoding;

	prefs_send->page.widget = vbox1;
}

static void prefs_send_save(PrefsPage *_page)
{
	SendPage *page = (SendPage *) _page;

	prefs_common.savemsg = gtk_toggle_button_get_active(
		GTK_TOGGLE_BUTTON(page->checkbtn_savemsg));
	prefs_common.confirm_send_queued_messages = gtk_toggle_button_get_active(
		GTK_TOGGLE_BUTTON(page->checkbtn_confirm_send_queued_messages));
	prefs_common.never_send_retrcpt = gtk_toggle_button_get_active(
		GTK_TOGGLE_BUTTON(page->checkbtn_never_send_retrcpt));
	prefs_common.send_dialog_invisible = !gtk_toggle_button_get_active(
		GTK_TOGGLE_BUTTON(page->checkbtn_senddialog));
	prefs_common.warn_empty_subj = gtk_toggle_button_get_active(
		GTK_TOGGLE_BUTTON(page->checkbtn_warn_empty_subj));

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(page->checkbtn_warn_multiple_recipients)))
		prefs_common.warn_sending_many_recipients_num =
			gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(page->spinbtn_warn_multiple_recipients));
	else
		prefs_common.warn_sending_many_recipients_num = 0;

	prefs_common.encoding_method = combobox_get_active_data(GTK_COMBO_BOX(page->combobox_encoding_method));
}

static void prefs_send_destroy_widget(PrefsPage *_page)
{
}

SendPage *prefs_send;

void prefs_send_init(void)
{
	SendPage *page;
	static gchar *path[3];

	path[0] = _("Mail Handling");
	path[1] = _("Sending");
	path[2] = NULL;

	page = g_new0(SendPage, 1);
	page->page.path = path;
	page->page.create_widget = prefs_send_create_widget;
	page->page.destroy_widget = prefs_send_destroy_widget;
	page->page.save_page = prefs_send_save;
	page->page.weight = 195.0;
	prefs_gtk_register_page((PrefsPage *) page);
	prefs_send = page;
}

void prefs_send_done(void)
{
	prefs_gtk_unregister_page((PrefsPage *) prefs_send);
	g_free(prefs_send);
}
