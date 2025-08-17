/*
 * Claws Mail -- a GTK based, lightweight, and fast e-mail client
 * Copyright (C) 1999-2025 the Claws Mail team
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

#include "prefs_common.h"
#include "prefs_gtk.h"

#include "gtk/gtkutils.h"
#include "gtk/prefswindow.h"
#include "gtk/menu.h"

#include "manage_window.h"

#include "log.h"
#include "combobox.h"

typedef struct _LoggingPage
{
	PrefsPage page;

	GtkWidget *window;

	GtkWidget *checkbtn_clip_network_log;
	GtkWidget *spinbtn_network_log_length;
	GtkWidget *checkbtn_log_standard;
	GtkWidget *checkbtn_log_warning;
	GtkWidget *checkbtn_log_error;
	GtkWidget *checkbtn_log_status;
} LoggingPage;

static GtkWidget *prefs_logging_create_check_buttons(GtkWidget **checkbtn1,
			gchar *label1, GtkWidget **checkbtn2, gchar *label2)
{
	GtkWidget *hbox_checkbtn;

	hbox_checkbtn = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, VBOX_BORDER);
	gtk_widget_show(hbox_checkbtn);

	PACK_CHECK_BUTTON (hbox_checkbtn, *checkbtn1, label1);
	gtk_label_set_line_wrap(GTK_LABEL(gtk_bin_get_child(GTK_BIN((*checkbtn1)))), TRUE);

	PACK_CHECK_BUTTON (hbox_checkbtn, *checkbtn2, label2);
	gtk_label_set_line_wrap(GTK_LABEL(gtk_bin_get_child(GTK_BIN((*checkbtn2)))), TRUE);

	return hbox_checkbtn;
}

static void prefs_logging_create_widget(PrefsPage *_page, GtkWindow *window,
			       	  gpointer data)
{
	LoggingPage *prefs_logging = (LoggingPage *) _page;

	GtkWidget *vbox1;

	GtkWidget *frame_logging;
	GtkWidget *vbox_network_log;
	GtkWidget *hbox_clip_network_log;
	GtkWidget *checkbtn_clip_network_log;
	GtkWidget *spinbtn_network_log_length;
	GtkAdjustment *spinbtn_network_log_length_adj;
	GtkWidget *hbox_checkbtn;
	GtkWidget *frame_disk_log;
	GtkWidget *vbox_disk_log;
	GtkWidget *label;
	GtkWidget *hbox;
	GtkWidget *checkbtn_log_standard;
	GtkWidget *checkbtn_log_warning;
	GtkWidget *checkbtn_log_error;
	GtkWidget *checkbtn_log_status;
	GtkSizeGroup *log_size_group;

	vbox1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, VSPACING);
	gtk_widget_show (vbox1);
	gtk_container_set_border_width (GTK_CONTAINER (vbox1), VBOX_BORDER);

	/* Protocol log */
	vbox_network_log = gtkut_get_options_frame(vbox1, &frame_logging, _("Network log"));

	hbox_clip_network_log = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
	gtk_container_add (GTK_CONTAINER (vbox_network_log), hbox_clip_network_log);
	gtk_widget_show (hbox_clip_network_log);

	PACK_CHECK_BUTTON (hbox_clip_network_log, checkbtn_clip_network_log,
			   _("Restrict the log window to"));

	spinbtn_network_log_length_adj = GTK_ADJUSTMENT(gtk_adjustment_new (500, 0, G_MAXINT, 1, 10, 0));
	spinbtn_network_log_length = gtk_spin_button_new
		(GTK_ADJUSTMENT (spinbtn_network_log_length_adj), 1, 0);
	gtk_widget_show (spinbtn_network_log_length);
	gtk_box_pack_start (GTK_BOX (hbox_clip_network_log), spinbtn_network_log_length,
			    FALSE, FALSE, 0);
	gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinbtn_network_log_length), TRUE);

	CLAWS_SET_TIP(spinbtn_network_log_length,
			     _("0 to stop logging in the log window"));

	label = gtk_label_new(_("lines"));
	gtk_widget_show (label);
  	gtk_box_pack_start(GTK_BOX(hbox_clip_network_log), label, FALSE, FALSE, 0);

	SET_TOGGLE_SENSITIVITY(checkbtn_clip_network_log, spinbtn_network_log_length);
	SET_TOGGLE_SENSITIVITY(checkbtn_clip_network_log, label);

	/* disk log */
	vbox_disk_log = gtkut_get_options_frame(vbox1, &frame_disk_log, _("Disk log"));

	label = gtk_label_new(_("Write the following information to disk..."));
	gtk_widget_show(label);
	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
	gtk_container_add (GTK_CONTAINER (vbox_disk_log), hbox);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_widget_show (hbox);

	hbox_checkbtn = prefs_logging_create_check_buttons(&checkbtn_log_warning,
				_("Warning messages"), &checkbtn_log_standard,
				_("Network protocol messages"));
	gtk_box_pack_start(GTK_BOX(vbox_disk_log), hbox_checkbtn, FALSE, FALSE, 0);

	hbox_checkbtn = prefs_logging_create_check_buttons(&checkbtn_log_error,
				_("Error messages"), &checkbtn_log_status,
				_("Status messages for filtering/processing log"));
	gtk_box_pack_start(GTK_BOX(vbox_disk_log), hbox_checkbtn, FALSE, FALSE, 0);

	log_size_group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	gtk_size_group_add_widget(log_size_group, checkbtn_log_warning);
	gtk_size_group_add_widget(log_size_group, checkbtn_log_error);
	g_object_unref(G_OBJECT(log_size_group));

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbtn_clip_network_log),
		prefs_common.cliplog);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbtn_log_standard),
		prefs_common.enable_log_standard);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbtn_log_warning),
		prefs_common.enable_log_warning);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbtn_log_error),
		prefs_common.enable_log_error);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbtn_log_status),
		prefs_common.enable_log_status);

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinbtn_network_log_length),
		prefs_common.loglength);

	prefs_logging->checkbtn_clip_network_log = checkbtn_clip_network_log;
	prefs_logging->spinbtn_network_log_length = spinbtn_network_log_length;
	prefs_logging->checkbtn_log_standard = checkbtn_log_standard;
	prefs_logging->checkbtn_log_warning = checkbtn_log_warning;
	prefs_logging->checkbtn_log_error = checkbtn_log_error;
	prefs_logging->checkbtn_log_status = checkbtn_log_status;
	prefs_logging->page.widget = vbox1;
}

static void prefs_logging_save(PrefsPage *_page)
{
	LoggingPage *page = (LoggingPage *) _page;
	MainWindow *mainwindow;

	prefs_common.cliplog = gtk_toggle_button_get_active(
		GTK_TOGGLE_BUTTON(page->checkbtn_clip_network_log));
	prefs_common.loglength = gtk_spin_button_get_value_as_int(
		GTK_SPIN_BUTTON(page->spinbtn_network_log_length));
	prefs_common.enable_log_standard = gtk_toggle_button_get_active(
		GTK_TOGGLE_BUTTON(page->checkbtn_log_standard));
	prefs_common.enable_log_warning = gtk_toggle_button_get_active(
		GTK_TOGGLE_BUTTON(page->checkbtn_log_warning));
	prefs_common.enable_log_error = gtk_toggle_button_get_active(
		GTK_TOGGLE_BUTTON(page->checkbtn_log_error));
	prefs_common.enable_log_status = gtk_toggle_button_get_active(
		GTK_TOGGLE_BUTTON(page->checkbtn_log_status));
	mainwindow = mainwindow_get_mainwindow();
	log_window_set_clipping(mainwindow->logwin, prefs_common.cliplog, prefs_common.loglength);
}

static void prefs_logging_destroy_widget(PrefsPage *_page)
{
}

LoggingPage *prefs_logging;

void prefs_logging_init(void)
{
	LoggingPage *page;
	static gchar *path[3];

	path[0] = _("Other");
	path[1] = _("Logging");
	path[2] = NULL;

	page = g_new0(LoggingPage, 1);
	page->page.path = path;
	page->page.create_widget = prefs_logging_create_widget;
	page->page.destroy_widget = prefs_logging_destroy_widget;
	page->page.save_page = prefs_logging_save;
	page->page.weight = 5.0;
	prefs_gtk_register_page((PrefsPage *) page);
	prefs_logging = page;
}

void prefs_logging_done(void)
{
	prefs_gtk_unregister_page((PrefsPage *) prefs_logging);
	g_free(prefs_logging);
}
