/*
 * Claws Mail -- a GTK based, lightweight, and fast e-mail client
 * Copyright (C) 2005-2024 the Claws Mail Team and Colin Leroy
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
#include "prefs_display_header.h"

#include "gtk/gtkutils.h"
#include "gtk/prefswindow.h"

#include "manage_window.h"

typedef struct _MessagePage
{
	PrefsPage page;

	GtkWidget *window;

	GtkWidget *checkbtn_disphdr;
	GtkWidget *checkbtn_dispxface;
	GtkWidget *checkbtn_savexface;

	GtkWidget *checkbtn_html;
	GtkWidget *checkbtn_promote_html_part;
	GtkWidget *spinbtn_linespc;

	GtkWidget *checkbtn_halfpage;
	GtkWidget *checkbtn_hide_quoted;

	GtkWidget *checkbtn_attach_desc;
} MessagePage;

static void prefs_message_create_widget(PrefsPage *_page, GtkWindow *window,
			       	  gpointer data)
{
	MessagePage *prefs_message = (MessagePage *) _page;

	GtkWidget *vbox1;
	GtkWidget *vbox2;
	GtkWidget *hbox1;
	GtkWidget *checkbtn_disphdr;
	GtkWidget *checkbtn_dispxface;
	GtkWidget *checkbtn_savexface;

	GtkWidget *button_edit_disphdr;
	GtkWidget *checkbtn_html;
	GtkWidget *checkbtn_promote_html_part;
	GtkWidget *hbox_linespc;
	GtkWidget *label_linespc;
	GtkAdjustment *spinbtn_linespc_adj;
	GtkWidget *spinbtn_linespc;

	GtkWidget *frame;
	GtkWidget *vbox_scr;
	GtkWidget *hbox_scr;
	GtkWidget *label_scr;
	GtkWidget *checkbtn_halfpage;
	GtkWidget *checkbtn_hide_quoted;

	GtkWidget *checkbtn_attach_desc;

	GtkWidget *frame_quote;
	GtkWidget *hbox2;
	GtkWidget *vbox_quote;

	vbox1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, VSPACING);
	gtk_widget_show (vbox1);
	gtk_container_set_border_width (GTK_CONTAINER (vbox1), VBOX_BORDER);

	vbox2 = gtkut_get_options_frame(vbox1, &frame, _("Headers"));
	PACK_CHECK_BUTTON(vbox2, checkbtn_dispxface,
			  _("Display Face in message view"));
	PACK_CHECK_BUTTON(vbox2, checkbtn_savexface,
			  _("Save Face in address book if possible"));

	hbox1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
	gtk_widget_show (hbox1);
	gtk_box_pack_start (GTK_BOX (vbox2), hbox1, FALSE, TRUE, 0);

	PACK_CHECK_BUTTON(hbox1, checkbtn_disphdr,
			  _("Display headers in message view"));

	button_edit_disphdr = gtk_button_new_with_mnemonic(_("_Edit"));
	gtk_widget_show (button_edit_disphdr);
	gtk_box_pack_start (GTK_BOX (hbox1), button_edit_disphdr,
			  FALSE, TRUE, 0);
	g_signal_connect (G_OBJECT (button_edit_disphdr), "clicked",
			  G_CALLBACK (prefs_display_header_open),
			  NULL);

	SET_TOGGLE_SENSITIVITY(checkbtn_disphdr, button_edit_disphdr);

	vbox2 = gtkut_get_options_frame(vbox1, &frame, _("HTML messages"));

	PACK_CHECK_BUTTON(vbox2, checkbtn_html,
			  _("Render HTML messages as text"));

	PACK_CHECK_BUTTON(vbox2, checkbtn_promote_html_part,
			  _("Select the HTML part of multipart/alternative messages"));

	hbox1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 32);
	gtk_widget_show (hbox1);
	gtk_box_pack_start (GTK_BOX (vbox1), hbox1, FALSE, TRUE, 0);

	hbox_linespc = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
	gtk_widget_show (hbox1);
	gtk_box_pack_start (GTK_BOX (hbox1), hbox_linespc, FALSE, TRUE, 0);

	label_linespc = gtk_label_new (_("Line space"));
	gtk_widget_show (label_linespc);
	gtk_box_pack_start (GTK_BOX (hbox_linespc), label_linespc,
			    FALSE, FALSE, 0);

	spinbtn_linespc_adj = GTK_ADJUSTMENT(gtk_adjustment_new (2, 0, 16, 1, 1, 0));
	spinbtn_linespc = gtk_spin_button_new
		(GTK_ADJUSTMENT (spinbtn_linespc_adj), 1, 0);
	gtk_widget_show (spinbtn_linespc);
	gtk_box_pack_start (GTK_BOX (hbox_linespc), spinbtn_linespc,
			    FALSE, FALSE, 0);
	gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinbtn_linespc), TRUE);

	label_linespc = gtk_label_new (_("pixels"));
	gtk_widget_show (label_linespc);
	gtk_box_pack_start (GTK_BOX (hbox_linespc), label_linespc,
			    FALSE, FALSE, 0);
	gtk_widget_show_all (hbox1);

	vbox_scr = gtkut_get_options_frame(vbox1, &frame, _("Scroll"));

	PACK_CHECK_BUTTON(vbox_scr, checkbtn_halfpage, _("Half page"));

	hbox1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 32);
	gtk_widget_show (hbox1);
	gtk_box_pack_start (GTK_BOX (vbox_scr), hbox1, FALSE, TRUE, 0);

	hbox_scr = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
	gtk_widget_show (hbox_scr);
	gtk_box_pack_start (GTK_BOX (hbox1), hbox_scr, FALSE, FALSE, 0);

	PACK_CHECK_BUTTON(vbox1, checkbtn_attach_desc,
			  _("Show attachment descriptions (rather than names)"));

	/* quote chars */
	PACK_FRAME (vbox1, frame_quote, _("Quotation"));

	vbox_quote = gtk_box_new(GTK_ORIENTATION_VERTICAL, VSPACING_NARROW);
	gtk_widget_show (vbox_quote);
	gtk_container_add (GTK_CONTAINER (frame_quote), vbox_quote);
	gtk_container_set_border_width (GTK_CONTAINER (vbox_quote), 8);

	hbox1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 32);
	gtk_widget_show (hbox1);
	PACK_CHECK_BUTTON(vbox_quote, checkbtn_hide_quoted, _("Collapse quoted text on double click"));
	gtk_box_pack_start (GTK_BOX (vbox_quote), hbox1, FALSE, FALSE, 0);

	hbox2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
	gtk_widget_show (hbox2);
	gtk_box_pack_start (GTK_BOX (hbox1), hbox2, FALSE, FALSE, 0);

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbtn_dispxface),
		prefs_common.display_xface);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbtn_savexface),
		prefs_common.save_xface);

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbtn_disphdr),
		prefs_common.display_header);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbtn_html),
		prefs_common.render_html);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbtn_promote_html_part),
		prefs_common.promote_html_part);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbtn_hide_quoted),
		prefs_common.hide_quoted);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbtn_halfpage),
		prefs_common.scroll_halfpage);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbtn_attach_desc),
		prefs_common.attach_desc);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinbtn_linespc),
		prefs_common.line_space);

	prefs_message->window = GTK_WIDGET(window);
	prefs_message->checkbtn_dispxface = checkbtn_dispxface;
	prefs_message->checkbtn_savexface = checkbtn_savexface;
	prefs_message->checkbtn_disphdr = checkbtn_disphdr;
	prefs_message->checkbtn_html = checkbtn_html;
	prefs_message->checkbtn_promote_html_part = checkbtn_promote_html_part;
	prefs_message->spinbtn_linespc = spinbtn_linespc;
	prefs_message->checkbtn_hide_quoted = checkbtn_hide_quoted;
	prefs_message->checkbtn_halfpage = checkbtn_halfpage;
	prefs_message->checkbtn_attach_desc = checkbtn_attach_desc;

	prefs_message->page.widget = vbox1;
}

static void prefs_message_save(PrefsPage *_page)
{
	MessagePage *page = (MessagePage *) _page;

	prefs_common.display_xface = gtk_toggle_button_get_active(
		GTK_TOGGLE_BUTTON(page->checkbtn_dispxface));
	prefs_common.save_xface = gtk_toggle_button_get_active(
		GTK_TOGGLE_BUTTON(page->checkbtn_savexface));
	prefs_common.display_header = gtk_toggle_button_get_active(
		GTK_TOGGLE_BUTTON(page->checkbtn_disphdr));
	prefs_common.render_html = gtk_toggle_button_get_active(
		GTK_TOGGLE_BUTTON(page->checkbtn_html));
	prefs_common.promote_html_part = gtk_toggle_button_get_active(
		GTK_TOGGLE_BUTTON(page->checkbtn_promote_html_part));
	prefs_common.scroll_halfpage = gtk_toggle_button_get_active(
		GTK_TOGGLE_BUTTON(page->checkbtn_halfpage));
	prefs_common.hide_quoted = gtk_toggle_button_get_active(
		GTK_TOGGLE_BUTTON(page->checkbtn_hide_quoted));
	prefs_common.attach_desc = gtk_toggle_button_get_active(
		GTK_TOGGLE_BUTTON(page->checkbtn_attach_desc));
	prefs_common.line_space = gtk_spin_button_get_value_as_int(
		GTK_SPIN_BUTTON(page->spinbtn_linespc));

	main_window_reflect_prefs_all_real(FALSE);
}

static void prefs_message_destroy_widget(PrefsPage *_page)
{
}

MessagePage *prefs_message;

void prefs_message_init(void)
{
	MessagePage *page;
	static gchar *path[3];

	path[0] = _("Message View");
	path[1] = _("Text Options");
	path[2] = NULL;

	page = g_new0(MessagePage, 1);
	page->page.path = path;
	page->page.create_widget = prefs_message_create_widget;
	page->page.destroy_widget = prefs_message_destroy_widget;
	page->page.save_page = prefs_message_save;
	page->page.weight = 170.0;
	prefs_gtk_register_page((PrefsPage *) page);
	prefs_message = page;
}

void prefs_message_done(void)
{
	prefs_gtk_unregister_page((PrefsPage *) prefs_message);
	g_free(prefs_message);
}
