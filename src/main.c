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

#include "common/defs.h"

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#ifdef HAVE_VALGRIND
#include <valgrind.h>
#endif

#include "claws.h"
#include "main.h"
#include "mainwindow.h"
#include "folderview.h"
#include "summaryview.h"
#include "prefs_common.h"
#include "prefs_account.h"
#include "prefs_actions.h"
#include "prefs_ext_prog.h"
#include "prefs_message.h"
#include "prefs_receive.h"
#include "prefs_summaries.h"
#include "prefs_themes.h"
#include "prefs_other.h"
#include "prefs_send.h"
#include "prefs_wrapping.h"
#include "prefs_compose_writing.h"
#include "prefs_display_header.h"
#include "account.h"
#include "procmsg.h"
#include "inc.h"
#include "imap.h"
#include "send_message.h"
#include "import.h"
#include "manage_window.h"
#include "alertpanel.h"
#include "statusbar.h"
#include "addressbook.h"
#include "compose.h"
#include "folder.h"
#include "folder_item_prefs.h"
#include "setup.h"
#include "utils.h"
#include "gtkutils.h"
#include "socket.h"
#include "log.h"
#include "prefs_toolbar.h"
#include "mh_gtk.h"
#include "imap_gtk.h"
#include "matcher.h"
#include "hooks.h"
#include "menu.h"
#include "avatars.h"
#include "passwordstore.h"
#include "file-utils.h"
#include "oauth2.h"

#include "etpan/imap-thread.h"
#include "stock_pixmap.h"
#include "ssl.h"

#include "version.h"


gchar *prog_version;

static gint lock_socket = -1;
static gint lock_socket_tag = 0;

typedef enum
{
	ONLINE_MODE_DONT_CHANGE,
 	ONLINE_MODE_ONLINE,
	ONLINE_MODE_OFFLINE
} OnlineMode;

static struct RemoteCmd {
	gboolean receive;
	gboolean receive_all;
	gboolean cancel_receiving;
	gboolean cancel_sending;
	gboolean compose;
	const gchar *compose_mailto;
	GList *attach_files;
	gboolean search;
	const gchar *search_folder;
	const gchar *search_type;
	const gchar *search_request;
	gboolean search_recursive;
	gboolean status;
	gboolean status_full;
	GPtrArray *status_folders;
	GPtrArray *status_full_folders;
	gboolean send;
	int online_mode;
	gboolean exit;
	gboolean subscribe;
	const gchar *subscribe_uri;
	const gchar *target;
	gboolean debug;
	const gchar *geometry;
	const gchar *import_mbox;
} cmd;

static void parse_cmd_opt(int argc, char *argv[]);

static gint prohibit_duplicate_launch	(int		*argc,
					 char		***argv);
static gint lock_socket_remove		(void);
static void lock_socket_input_cb	(gpointer	   data,
					 gint		   source,
					 GIOCondition      condition);

static void open_compose_new		(const gchar	*address,
					 GList		*attach_files);

static void send_queue			(void);
static void initial_processing		(FolderItem *item, gpointer data);
static void quit_signal_handler         (int sig);
static void install_basic_sighandlers   (void);
static void exit_claws			(MainWindow *mainwin);

#define MAKE_DIR_IF_NOT_EXIST(dir) \
{ \
	if (!is_dir_exist(dir)) { \
		if (is_file_exist(dir)) { \
			alertpanel_warning \
				(_("File '%s' already exists.\n" \
				   "Can't create folder."), \
				 dir); \
			return 1; \
		} \
		if (make_dir(dir) < 0) \
			return 1; \
	} \
}

#define STRNCMP(S1, S2) (strncmp((S1), (S2), sizeof((S2)) - 1))

#define CM_FD_WRITE(S) fd_write(sock, (S), strlen((S)))
#define CM_FD_WRITE_ALL(S) fd_write_all(sock, (S), strlen((S)))

static MainWindow *static_mainwindow;

static gboolean emergency_exit = FALSE;

static void claws_gtk_idle(void)
{
	while(gtk_events_pending()) {
		gtk_main_iteration();
	}
	g_usleep(50000);
}

static gboolean sc_starting = FALSE;

static gboolean defer_check_all(void *data)
{
	gboolean autochk = GPOINTER_TO_INT(data);

	if (!sc_starting) {
		inc_all_account_mail(static_mainwindow, autochk, FALSE,
			prefs_common.newmail_notify_manu);

	} else {
		inc_all_account_mail(static_mainwindow, FALSE,
				prefs_common.chk_on_startup,
				prefs_common.newmail_notify_manu);
		sc_starting = FALSE;
		main_window_set_menu_sensitive(static_mainwindow);
		toolbar_main_set_sensitive(static_mainwindow);
	}
	return FALSE;
}

static gboolean defer_check(void *data)
{
	inc_mail(static_mainwindow, prefs_common.newmail_notify_manu);

	if (sc_starting) {
		sc_starting = FALSE;
		main_window_set_menu_sensitive(static_mainwindow);
		toolbar_main_set_sensitive(static_mainwindow);
	}
	return FALSE;
}

static gboolean defer_jump(void *data)
{
	if (cmd.receive_all) {
		defer_check_all(GINT_TO_POINTER(FALSE));
	} else if (prefs_common.chk_on_startup) {
		defer_check_all(GINT_TO_POINTER(TRUE));
	} else if (cmd.receive) {
		defer_check(NULL);
	}
	mainwindow_jump_to(data, FALSE);
	if (sc_starting) {
		sc_starting = FALSE;
		main_window_set_menu_sensitive(static_mainwindow);
		toolbar_main_set_sensitive(static_mainwindow);
	}
	return FALSE;
}

static gboolean sc_exiting = FALSE;

bool verify_folderlist_xml()
{
	GNode *node;
	static gchar *filename = NULL;
	static gchar *bak = NULL;
	time_t date;
	struct tm *ts;
	gchar buf[BUFFSIZE];
	gboolean fileexists, bakexists;

	filename = folder_get_list_path();

	fileexists = is_file_exist(filename);

	bak = g_strconcat(get_rc_dir(), G_DIR_SEPARATOR_S,
 			  FOLDER_LIST, ".bak", NULL);
	bakexists = is_file_exist(bak);

	if (bakexists) {
		date = get_file_mtime(bak);
		ts = localtime(&date);
		strftime(buf, sizeof(buf), "%a %d-%b-%Y %H:%M %Z", ts);
	}

	if (!fileexists && bakexists) {
		AlertValue aval;
		gchar *msg;

		msg = g_strdup_printf
			(_("The file %s is missing! "
			   "Do you want to use the backup file from %s?"), FOLDER_LIST,buf);
		aval = alertpanel(_("Warning"), msg, NULL, _("_No"), NULL, _("_Yes"),
				  NULL, NULL, ALERTFOCUS_FIRST);
		g_free(msg);
		if (aval != G_ALERTALTERNATE)
			return false;
		else {
			if (copy_file(bak,filename,FALSE) < 0) {
				alertpanel_warning(_("Could not copy %s to %s"),bak,filename);
				return false;
			}
			g_free(bak);
			return true;
		}
	}

	if (fileexists) {
		node = xml_parse_file(filename);
		if (!node && is_file_exist(bak)) {
			AlertValue aval;
			gchar *msg;

			msg = g_strdup_printf
				(_("The file %s is empty or corrupted! "
				   "Do you want to use the backup file from %s?"), FOLDER_LIST,buf);
			aval = alertpanel(_("Warning"), msg, NULL, _("_No"), NULL, _("_Yes"),
					  NULL, NULL, ALERTFOCUS_FIRST);
			g_free(msg);
			if (aval != G_ALERTALTERNATE)
				return false;
			else {
				if (copy_file(bak,filename,FALSE) < 0) {
					alertpanel_warning(_("Could not copy %s to %s"),bak,filename);
					return false;
				}
				g_free(bak);
				return true;
			}
		}
		xml_free_tree(node);
  	}

	return true;
}

static void main_dump_features_list(gboolean show_debug_only)
/* display compiled-in features list */
{
	if (show_debug_only && !debug_get_mode())
		return;

	if (show_debug_only)
		debug_print("runtime GTK %d.%d.%d / GLib %d.%d.%d\n",
			   gtk_major_version, gtk_minor_version, gtk_micro_version,
			   glib_major_version, glib_minor_version, glib_micro_version);
	else
		g_print("runtime GTK %d.%d.%d / GLib %d.%d.%d\n",
			   gtk_major_version, gtk_minor_version, gtk_micro_version,
			   glib_major_version, glib_minor_version, glib_micro_version);
	if (show_debug_only)
		debug_print("buildtime GTK %d.%d.%d / GLib %d.%d.%d\n",
			   GTK_MAJOR_VERSION, GTK_MINOR_VERSION, GTK_MICRO_VERSION,
			   GLIB_MAJOR_VERSION, GLIB_MINOR_VERSION, GLIB_MICRO_VERSION);
	else
		g_print("buildtime GTK %d.%d.%d / GLib %d.%d.%d\n",
			   GTK_MAJOR_VERSION, GTK_MINOR_VERSION, GTK_MICRO_VERSION,
			   GLIB_MAJOR_VERSION, GLIB_MINOR_VERSION, GLIB_MICRO_VERSION);

	if (show_debug_only)
		debug_print("Compiled-in features:\n");
	else
		g_print("Compiled-in features:\n");
	if (show_debug_only)
		debug_print(" GnuTLS\n");
	else
		g_print(" GnuTLS\n");
	if (show_debug_only)
		debug_print(" iconv\n");
	else
		g_print(" iconv\n");
	if (show_debug_only)
		debug_print(" libetpan %d.%d\n", LIBETPAN_VERSION_MAJOR, LIBETPAN_VERSION_MINOR);
	else
		g_print(" libetpan %d.%d\n", LIBETPAN_VERSION_MAJOR, LIBETPAN_VERSION_MINOR);
}

int main(int argc, char *argv[])
{
	gchar *userrc;
	MainWindow *mainwin;
	FolderView *folderview;
	GdkPixbuf *icon;
	guint num_folder_class = 0;
	gboolean start_done = TRUE;
	gboolean never_ran = FALSE;
	gint ret;

	sc_starting = TRUE;

	if (!claws_init(&argc, &argv)) {
		return 0;
	}

	prog_version = PROG_VERSION;

	parse_cmd_opt(argc, argv);

	sock_init();

	/* check and create unix domain socket for remote operation */
	lock_socket = prohibit_duplicate_launch(&argc, &argv);
	if (lock_socket < 0) {
		return 0;
	}

	main_dump_features_list(TRUE);
	prefs_prepare_cache();
	install_basic_sighandlers();

	if (cmd.status || cmd.status_full || cmd.search ||
		cmd.cancel_receiving || cmd.cancel_sending ||
		cmd.debug) {
		puts("0 Claws Mail not running.");
		lock_socket_remove();
		return 0;
	}

	if (cmd.exit)
		return 0;

	gtk_init(&argc, &argv);

	gtkut_create_ui_manager();

	/* Create container for all the menus we will be adding */
	MENUITEM_ADDUI("/", "Menus", NULL, GTK_UI_MANAGER_MENUBAR);

	CHDIR_RETURN_VAL_IF_FAIL(get_home_dir(), 1);

	if (!is_dir_exist(get_rc_dir())) {
		if (copy_dir("/etc/skel/.claws-mail", get_rc_dir()) < 0) {
			if (!is_dir_exist(get_rc_dir()) && make_dir(get_rc_dir()) < 0) {
				exit(1);
			}
		}
	}

	userrc = g_strconcat(get_rc_dir(), G_DIR_SEPARATOR_S, "gtkrc-2.0", NULL);
	gtk_rc_parse(userrc);
	g_free(userrc);

	userrc = g_strconcat(get_rc_dir(), G_DIR_SEPARATOR_S, MENU_RC, NULL);

	if (copy_file(userrc, userrc, TRUE) < 0) {
		g_warning("can't copy %s to %s.bak", userrc, userrc);
	}

	gtk_accel_map_load (userrc);
	g_free(userrc);
	CHDIR_RETURN_VAL_IF_FAIL(get_rc_dir(), 1);

	MAKE_DIR_IF_NOT_EXIST(get_mail_base_dir());
	MAKE_DIR_IF_NOT_EXIST(get_imap_cache_dir());
	MAKE_DIR_IF_NOT_EXIST(get_news_cache_dir());
	MAKE_DIR_IF_NOT_EXIST(get_mime_tmp_dir());
	MAKE_DIR_IF_NOT_EXIST(get_tmp_dir());
	MAKE_DIR_IF_NOT_EXIST(UIDL_DIR);

	remove_all_files(get_tmp_dir());
	remove_all_files(get_mime_tmp_dir());

	if (is_file_exist("claws.log")) {
		if (rename_force("claws.log", "claws.log.bak") < 0)
			FILE_OP_ERROR("claws.log", "rename");
	}
	set_log_file(LOG_PROTOCOL, "claws.log");

	if (is_file_exist("filtering.log")) {
		if (rename_force("filtering.log", "filtering.log.bak") < 0)
			FILE_OP_ERROR("filtering.log", "rename");
	}
	set_log_file(LOG_DEBUG_FILTERING, "filtering.log");

	CHDIR_RETURN_VAL_IF_FAIL(get_home_dir(), 1);

	folder_system_init();
	prefs_common_read_config();

	prefs_themes_init();
	prefs_ext_prog_init();
	prefs_wrapping_init();
	prefs_compose_writing_init();
	prefs_summaries_init();
	prefs_message_init();
	prefs_other_init();
	prefs_receive_init();
	prefs_send_init();

	codeconv_set_allow_jisx0201_kana(prefs_common.allow_jisx0201_kana);
	codeconv_set_broken_are_utf8(prefs_common.broken_are_utf8);

	sock_set_io_timeout(prefs_common.io_timeout_secs);
	prefs_actions_read_config();
	prefs_display_header_read_config();
	addressbook_read_file();
	gtkut_widget_init();
	priv_pixbuf_gdk(PRIV_PIXMAP_CLAWS_MAIL_ICON, &icon);
	gtk_window_set_default_icon(icon);

	folderview_initialize();

	mh_gtk_init();
	imap_gtk_init();

	mainwin = main_window_create();

	if (!verify_folderlist_xml())
		exit(1);

	manage_window_focus_in(mainwin->window, NULL, NULL);
	folderview = mainwin->folderview;

	folderview_freeze(mainwin->folderview);
	folder_item_update_freeze();

	if ((ret = passwd_store_read_config()) < 0) {
		debug_print("Password store configuration file version upgrade failed (%d), exiting\n", ret);
		exit(202);
	}

	prefs_account_init();
	account_read_config_all();
	account_read_oauth2_all();

	imap_main_init(prefs_common.skip_ssl_cert_check);
	imap_main_set_timeout(prefs_common.io_timeout_secs);
	/* If we can't read a folder list or don't have accounts,
	 * it means the configuration's not done. Either this is
	 * a brand new install, a failed/refused migration,
	 * or a failed config_version upgrade.
	 */
	if ((ret = folder_read_list()) < 0) {
		debug_print("Folderlist read failed (%d)\n", ret);
		prefs_destroy_cache();

		if (ret == -2) {
			/* config_version update failed in folder_read_list(). */
			debug_print("Folderlist version upgrade failed, exiting\n");
			exit(203);
		}

		main_window_reflect_prefs_all_now();
		folder_write_list();
		never_ran = TRUE;
	}

	if (!account_get_list()) {
		prefs_destroy_cache();
		if(!account_get_list()) {
			exit_claws(mainwin);
			exit(1);
		}
		never_ran = TRUE;
	}


	toolbar_main_set_sensitive(mainwin);
	main_window_set_menu_sensitive(mainwin);

	account_set_missing_folder();
	folder_set_missing_folders();
	folderview_set(folderview);

	/* make one all-folder processing before using claws */
	main_window_cursor_wait(mainwin);
	folder_func_to_all_folders(initial_processing, (gpointer *)mainwin);

	inc_autocheck_timer_init(mainwin);
	if (cmd.online_mode == ONLINE_MODE_OFFLINE) {
		main_window_toggle_work_offline(mainwin, TRUE, FALSE);
	}
	if (cmd.online_mode == ONLINE_MODE_ONLINE) {
		main_window_toggle_work_offline(mainwin, FALSE, FALSE);
	}

	if (cmd.status_folders) {
		g_ptr_array_free(cmd.status_folders, TRUE);
		cmd.status_folders = NULL;
	}
	if (cmd.status_full_folders) {
		g_ptr_array_free(cmd.status_full_folders, TRUE);
		cmd.status_full_folders = NULL;
	}

	claws_register_idle_function(claws_gtk_idle);

	avatars_init();
	prefs_toolbar_init();

	num_folder_class = g_list_length(folder_get_list());

	if (never_ran) {
		prefs_common_write_config();
	}

	main_window_popup(mainwin);

	if (cmd.geometry != NULL) {
		if (!gtk_window_parse_geometry(GTK_WINDOW(mainwin->window), cmd.geometry))
			g_warning("failed to parse geometry '%s'", cmd.geometry);
		else {
			int width, height;

			if (sscanf(cmd.geometry, "%ux%u+", &width, &height) == 2)
				gtk_window_resize(GTK_WINDOW(mainwin->window), width, height);
			else
				g_warning("failed to parse geometry's width/height");
		}
	}

	if (!folder_have_mailbox()) {
		prefs_destroy_cache();
		main_window_cursor_normal(mainwin);
		if (folder_get_list() != NULL) {
			alertpanel_error(_("Claws Mail has detected a configured "
				   "mailbox, but it is incomplete. It is "
				   "possibly due to a failing IMAP account. Use "
				   "\"Rebuild folder tree\" on the mailbox parent "
				   "folder's context menu to try to fix it."));
		} else {
			alertpanel_error("Could not load configured mailbox.");
			exit_claws(mainwin);
			exit(1);
		}
	}

	static_mainwindow = mainwin;
	folder_item_update_thaw();
	folderview_thaw(mainwin->folderview);
	main_window_cursor_normal(mainwin);

	if (cmd.import_mbox) {
		mainwindow_import_mbox(cmd.import_mbox);
	}

	if (!cmd.target && prefs_common.goto_folder_on_startup &&
	    folder_find_item_from_identifier(prefs_common.startup_folder) != NULL) {
		cmd.target = prefs_common.startup_folder;
	} else if (!cmd.target && prefs_common.goto_last_folder_on_startup &&
	    folder_find_item_from_identifier(prefs_common.last_opened_folder) != NULL) {
		cmd.target = prefs_common.last_opened_folder;
	}

	if (cmd.receive_all && !cmd.target) {
		start_done = FALSE;
		g_timeout_add(1000, defer_check_all, GINT_TO_POINTER(FALSE));
	} else if (prefs_common.chk_on_startup && !cmd.target) {
		start_done = FALSE;
		g_timeout_add(1000, defer_check_all, GINT_TO_POINTER(TRUE));
	} else if (cmd.receive && !cmd.target) {
		start_done = FALSE;
		g_timeout_add(1000, defer_check, NULL);
	}
	folderview_grab_focus(folderview);

	if (cmd.compose) {
		open_compose_new(cmd.compose_mailto, cmd.attach_files);
	}
	if (cmd.attach_files) {
		list_free_strings_full(cmd.attach_files);
		cmd.attach_files = NULL;
	}
	if (cmd.subscribe) {
		folder_subscribe(cmd.subscribe_uri);
	}

	if (cmd.send) {
		send_queue();
	}

	if (cmd.target) {
		start_done = FALSE;
		g_timeout_add(500, defer_jump, (gpointer)cmd.target);
	}

	prefs_destroy_cache();

	compose_reopen_exit_drafts();

	if (start_done) {
		sc_starting = FALSE;
		main_window_set_menu_sensitive(mainwin);
		toolbar_main_set_sensitive(mainwin);
	}

	/* register the callback of unix domain socket input */
	lock_socket_tag = claws_input_add(lock_socket,
					G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_PRI,
					lock_socket_input_cb,
					mainwin, TRUE);


	gtk_main();
	utils_free_regex();
	exit_claws(mainwin);

	return 0;
}

static void save_all_caches(FolderItem *item, gpointer data)
{
	if (!item->cache) {
		return;
	}

	if (item->opened) {
		folder_item_close(item);
	}

	folder_item_free_cache(item, TRUE);
}

static void exit_claws(MainWindow *mainwin)
{
	gchar *filename;
	FolderItem *item;

	sc_exiting = TRUE;

	debug_print("shutting down\n");
	inc_autocheck_timer_remove();

	/* save prefs for opened folder */
	if((item = folderview_get_opened_item(mainwin->folderview)) != NULL) {
		summary_save_prefs_to_folderitem(
			mainwin->summaryview, item);
		if (prefs_common.last_opened_folder != NULL)
			g_free(prefs_common.last_opened_folder);
		prefs_common.last_opened_folder =
			!prefs_common.goto_last_folder_on_startup ? NULL :
			folder_item_get_identifier(item);
	}

	/* save all state before exiting */
	folder_func_to_all_folders(save_all_caches, NULL);
	folder_write_list();

	main_window_get_size(mainwin);
	main_window_get_position(mainwin);

	prefs_common_write_config();
	account_write_config_all();
	passwd_store_write_config();
	addressbook_export_to_file();
	filename = g_strconcat(get_rc_dir(), G_DIR_SEPARATOR_S, MENU_RC, NULL);
	gtk_accel_map_save(filename);
	g_free(filename);

	/* delete temporary files */
	remove_all_files(get_tmp_dir());
	remove_all_files(get_mime_tmp_dir());

	close_log_file(LOG_PROTOCOL);
	close_log_file(LOG_DEBUG_FILTERING);

	imap_main_done(TRUE);

	lock_socket_remove();

	main_window_destroy_all();

	prefs_toolbar_done();
	avatars_done();

	addressbook_destroy();
	prefs_themes_done();
	prefs_ext_prog_done();
	prefs_wrapping_done();
	prefs_compose_writing_done();
	prefs_summaries_done();
	prefs_message_done();
	prefs_other_done();
	prefs_receive_done();
	prefs_send_done();
	claws_done();
}

#define G_PRINT_EXIT(msg)	\
	{			\
		g_print(msg);	\
		exit(1);	\
	}

static void parse_cmd_compose_from_file(const gchar *fn, GString *body)
{
	GString *headers = g_string_new(NULL);
	gchar *to = NULL;
	gchar *h;
	gchar *v;
	gchar fb[BUFFSIZE];
	FILE *fp;
	gboolean isstdin;

	if (fn == NULL || *fn == '\0')
		G_PRINT_EXIT(_("Missing filename\n"));
	isstdin = (*fn == '-' && *(fn + 1) == '\0');
	if (isstdin)
		fp = stdin;
	else {
		fp = g_fopen(fn, "r");
		if (!fp)
			G_PRINT_EXIT(_("Cannot open filename for reading\n"));
	}

	while (fgets(fb, sizeof(fb), fp)) {
		gchar *tmp;
		strretchomp(fb);
		if (*fb == '\0')
			break;
		h = fb;
		while (*h && *h != ':') { ++h; } /* search colon */
        	if (*h == '\0')
			G_PRINT_EXIT(_("Malformed header\n"));
		v = h + 1;
		while (*v && *v == ' ') { ++v; } /* trim value start */
		*h = '\0';
		tmp = g_ascii_strdown(fb, -1); /* get header name */
		if (!strcmp(tmp, "to")) {
			if (to != NULL)
				G_PRINT_EXIT(_("Duplicated 'To:' header\n"));
			to = g_strdup(v);
		} else {
			g_string_append_c(headers, '&');
			g_string_append(headers, tmp);
			g_string_append_c(headers, '=');
			g_string_append_uri_escaped(headers, v, NULL, TRUE);
		}
		g_free(tmp);
	}
	if (to == NULL)
		G_PRINT_EXIT(_("Missing required 'To:' header\n"));
	g_string_append(body, to);
	g_free(to);
	g_string_append(body, "?body=");
	while (fgets(fb, sizeof(fb), fp)) {
		g_string_append_uri_escaped(body, fb, NULL, TRUE);
	}
	if (!isstdin)
		fclose(fp);
	/* append the remaining headers */
	g_string_append(body, headers->str);
	g_string_free(headers, TRUE);
}

#undef G_PRINT_EXIT

static void parse_cmd_opt_error(char *errstr, char* optstr)
{
    gchar tmp[BUFSIZ];

	cm_return_if_fail(errstr != NULL);
	cm_return_if_fail(optstr != NULL);

    g_snprintf(tmp, sizeof(tmp), errstr, optstr);
	g_print(_("%s. Try -h or --help for usage.\n"), tmp);
	exit(1);
}

static GString mailto; /* used to feed cmd.compose_mailto when --compose-from-file is used */

static void parse_cmd_opt(int argc, char *argv[])
{
	AttachInfo *ainfo;
	gint i;

	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "--receive-all")) {
			cmd.receive_all = TRUE;
		} else if (!strcmp(argv[i], "--receive")) {
			cmd.receive = TRUE;
		} else if (!strcmp(argv[i], "--cancel-receiving")) {
			cmd.cancel_receiving = TRUE;
		} else if (!strcmp(argv[i], "--cancel-sending")) {
			cmd.cancel_sending = TRUE;
		} else if (!strcmp(argv[i], "--compose-from-file")) {
		    if (i+1 < argc) {
				const gchar *p = argv[i+1];

				parse_cmd_compose_from_file(p, &mailto);
				cmd.compose = TRUE;
				cmd.compose_mailto = mailto.str;
				i++;
		    } else {
                parse_cmd_opt_error(_("Missing file argument for option %s"), argv[i]);
			}
		} else if (!strcmp(argv[i], "--compose")) {
			const gchar *p = (i+1 < argc)?argv[i+1]:NULL;

			cmd.compose = TRUE;
			cmd.compose_mailto = NULL;
			if (p && *p != '\0' && *p != '-') {
				if (!STRNCMP(p, "mailto:")) {
					cmd.compose_mailto = p + 7;
				} else {
					cmd.compose_mailto = p;
				}
				i++;
			}
		} else if (!strcmp(argv[i], "--subscribe")) {
		    if (i+1 < argc) {
				const gchar *p = argv[i+1];
				if (p && *p != '\0' && *p != '-') {
					cmd.subscribe = TRUE;
					cmd.subscribe_uri = p;
				} else {
                    parse_cmd_opt_error(_("Missing or empty uri argument for option %s"), argv[i]);
				}
		    } else {
                parse_cmd_opt_error(_("Missing uri argument for option %s"), argv[i]);
			}
		} else if (!strcmp(argv[i], "--attach") ||
                !strcmp(argv[i], "--insert")) {
		    if (i+1 < argc) {
				const gchar *p = argv[i+1];
				gint ii = i;
				gchar *file = NULL;
				gboolean insert = !strcmp(argv[i], "--insert");

				while (p && *p != '\0' && *p != '-') {
					if ((file = g_filename_from_uri(p, NULL, NULL)) != NULL) {
						if (!is_file_exist(file)) {
							g_free(file);
							file = NULL;
						}
					}
					if (file == NULL && *p != G_DIR_SEPARATOR) {
						file = g_strconcat(claws_get_startup_dir(),
								   G_DIR_SEPARATOR_S,
								   p, NULL);
					} else if (file == NULL) {
						file = g_strdup(p);
					}

					ainfo = g_new0(AttachInfo, 1);
					ainfo->file = file;
					ainfo->insert = insert;
					cmd.attach_files = g_list_append(cmd.attach_files, ainfo);
					ii++;
					p = (ii+1 < argc)?argv[ii+1]:NULL;
				}
				if (ii==i) {
                    parse_cmd_opt_error(_("Missing at least one non-empty file argument for option %s"), argv[i]);
				} else {
                    i=ii;
                }
		    } else {
                parse_cmd_opt_error(_("Missing file argument for option %s"), argv[i]);
			}
		} else if (!strcmp(argv[i], "--send")) {
			cmd.send = TRUE;
		} else if (!strcmp(argv[i], "--version-full") ||
			   !strcmp(argv[i], "-V")) {
			g_print("Claws Mail version " VERSION_GIT_FULL "\n");
			main_dump_features_list(FALSE);
			exit(0);
		} else if (!strcmp(argv[i], "--version") ||
			   !strcmp(argv[i], "-v")) {
			g_print("Claws Mail version " VERSION "\n");
			exit(0);
 		} else if (!strcmp(argv[i], "--status-full")) {
 			const gchar *p = (i+1 < argc)?argv[i+1]:NULL;

 			cmd.status_full = TRUE;
 			while (p && *p != '\0' && *p != '-') {
 				if (!cmd.status_full_folders) {
 					cmd.status_full_folders =
 						g_ptr_array_new();
				}
 				g_ptr_array_add(cmd.status_full_folders,
 						g_strdup(p));
 				i++;
 				p = (i+1 < argc)?argv[i+1]:NULL;
 			}
  		} else if (!strcmp(argv[i], "--status")) {
 			const gchar *p = (i+1 < argc)?argv[i+1]:NULL;

  			cmd.status = TRUE;
 			while (p && *p != '\0' && *p != '-') {
 				if (!cmd.status_folders)
 					cmd.status_folders = g_ptr_array_new();
 				g_ptr_array_add(cmd.status_folders,
 						g_strdup(p));
 				i++;
 				p = (i+1 < argc)?argv[i+1]:NULL;
 			}
		} else if (!strcmp(argv[i], "--search")) {
		    if (i+3 < argc) { /* 3 first arguments are mandatory */
		    	const char* p;
		    	/* only set search parameters if they are valid */
		    	p = argv[i+1];
				cmd.search_folder    = (p && *p != '\0' && *p != '-')?p:NULL;
				p = argv[i+2];
				cmd.search_type      = (p && *p != '\0' && *p != '-')?p:NULL;
				p = argv[i+3];
				cmd.search_request   = (p && *p != '\0' && *p != '-')?p:NULL;
				p = (i+4 < argc)?argv[i+4]:NULL;
				const char* rec      = (p && *p != '\0' && *p != '-')?p:NULL;
				cmd.search_recursive = TRUE;
				if (rec) {
                    i++;
                    if (tolower(*rec)=='n' || tolower(*rec)=='f' || *rec=='0')
    					cmd.search_recursive = FALSE;
                }
				if (cmd.search_folder && cmd.search_type && cmd.search_request) {
					cmd.search = TRUE;
                    i+=3;
                }
		    } else {
                switch (argc-i-1) {
                case 0:
                    parse_cmd_opt_error(_("Missing folder, type and request arguments for option %s"), argv[i]);
                    break;
                case 1:
                    parse_cmd_opt_error(_("Missing type and request arguments for option %s"), argv[i]);
                    break;
                case 2:
                    parse_cmd_opt_error(_("Missing request argument for option %s"), argv[i]);
                }
			}
		} else if (!strcmp(argv[i], "--online")) {
			cmd.online_mode = ONLINE_MODE_ONLINE;
		} else if (!strcmp(argv[i], "--offline")) {
			cmd.online_mode = ONLINE_MODE_OFFLINE;
		} else if (!strcmp(argv[i], "--toggle-debug")) {
			cmd.debug = TRUE;
		} else if (!strcmp(argv[i], "--help") ||
			   !strcmp(argv[i], "-h")) {
			gchar *base = g_path_get_basename(argv[0]);
			g_print(_("Usage: %s [OPTION]...\n"), base);

			g_print("%s\n", _("  --compose [address]    open composition window"));
			g_print("%s\n", _("  --compose-from-file file\n"
				  	  "                         open composition window with data from given file;\n"
			  	  	  "                         use - as file name for reading from standard input;\n"
			  	  	  "                         content format: headers first (To: required) until an\n"
				  	  "                         empty line, then mail body until end of file."));
			g_print("%s\n", _("  --subscribe uri        subscribe to the given URI if possible"));
			g_print("%s\n", _("  --attach file1 [file2]...\n"
					  "                         open composition window with specified files\n"
					  "                         attached"));
			g_print("%s\n", _("  --insert file1 [file2]...\n"
					  "                         open composition window with specified files\n"
					  "                         inserted"));
			g_print("%s\n", _("  --receive              receive new messages"));
			g_print("%s\n", _("  --receive-all          receive new messages of all accounts"));
			g_print("%s\n", _("  --cancel-receiving     cancel receiving of messages"));
			g_print("%s\n", _("  --cancel-sending       cancel sending of messages"));
			g_print("%s\n", _("  --search folder type request [recursive]\n"
					  "                         searches mail\n"
					  "                         folder ex.: \"#mh/Mailbox/inbox\" or \"Mail\"\n"
					  "                         type: s[ubject],f[rom],t[o],e[xtended],m[ixed] or g: tag\n"
					  "                         request: search string\n"
					  "                         recursive: false if arg. starts with 0, n, N, f or F"));

			g_print("%s\n", _("  --send                 send all queued messages"));
 			g_print("%s\n", _("  --status [folder]...   show the total number of messages"));
 			g_print("%s\n", _("  --status-full [folder]...\n"
 			                  "                         show the status of each folder"));
			g_print("%s\n", _("  --select folder[/msg]  jump to the specified folder/message\n"
					  "                         folder is a folder id like 'folder/subfolder', a file:// uri or an absolute path"));
			g_print("%s\n", _("  --import-mbox file     import the specified mbox file\n"));
			g_print("%s\n", _("  --online               switch to online mode"));
			g_print("%s\n", _("  --offline              switch to offline mode"));
			g_print("%s\n", _("  --exit --quit -q       exit Claws Mail"));
			g_print("%s\n", _("  --debug -d             debug mode"));
			g_print("%s\n", _("  --toggle-debug         toggle debug mode"));
			g_print("%s\n", _("  --help -h              display this help"));
			g_print("%s\n", _("  --version -v           output version information"));
			g_print("%s\n", _("  --version-full -V      output version and built-in features information"));
			g_print("%s\n", _("  --alternate-config-dir directory\n"
			                  "                         use specified configuration directory"));
			g_print("%s\n", _("  --geometry -geometry [WxH][+X+Y]\n"
					  "                         set geometry for main window"));

			g_free(base);
			exit(1);
		} else if (!strcmp(argv[i], "--alternate-config-dir")) {
		    if (i+1 < argc) {
				set_rc_dir(argv[i+1]);
                i++;
		    } else {
                parse_cmd_opt_error(_("Missing directory argument for option %s"), argv[i]);
			}
		} else if (!strcmp(argv[i], "--geometry") ||
		        !strcmp(argv[i], "-geometry")) {
		    if (i+1 < argc) {
				cmd.geometry = argv[i+1];
                i++;
		    } else {
                parse_cmd_opt_error(_("Missing geometry argument for option %s"), argv[i]);
			}
		} else if (!strcmp(argv[i], "--exit") ||
			   !strcmp(argv[i], "--quit") ||
			   !strcmp(argv[i], "-q")) {
			cmd.exit = TRUE;
		} else if (!strcmp(argv[i], "--select")) {
		    if (i+1 < argc) {
				cmd.target = argv[i+1];
                i++;
		    } else {
                parse_cmd_opt_error(_("Missing folder argument for option %s"), argv[i]);
			}
		} else if (!strcmp(argv[i], "--import-mbox")) {
			if (i+1 < argc) {
				cmd.import_mbox = argv[i+1];
				i++;
			} else {
				parse_cmd_opt_error(_("Missing file argument for option %s"), argv[i]);
			}
		} else if (i == 1 && argc == 2) {
			/* only one parameter. Do something intelligent about it */
			if ((strstr(argv[i], "@") || !STRNCMP(argv[i], "mailto:")) && !strstr(argv[i], "://")) {
				const gchar *p = argv[i];

				cmd.compose = TRUE;
				cmd.compose_mailto = NULL;
				if (p && *p != '\0' && *p != '-') {
					if (!STRNCMP(p, "mailto:")) {
						cmd.compose_mailto = p + 7;
					} else {
						cmd.compose_mailto = p;
					}
				}
			} else if (!STRNCMP(argv[i], "file://")) {
				cmd.target = argv[i];
			} else if (!STRNCMP(argv[i], "?attach=file://")) {
                /* Thunar support as per 3.3.0cvs19 */
				cmd.compose = TRUE;
				cmd.compose_mailto = argv[i];
			} else if (strstr(argv[i], "://")) {
				const gchar *p = argv[i];
				if (p && *p != '\0' && *p != '-') {
					cmd.subscribe = TRUE;
					cmd.subscribe_uri = p;
				}
			} else if (!strcmp(argv[i], "--sync")) {
				/* gtk debug */
			} else if (is_dir_exist(argv[i]) || is_file_exist(argv[i])) {
				cmd.target = argv[i];
    		} else {
                parse_cmd_opt_error(_("Unknown option %s"), argv[i]);
            }
		} else {
            parse_cmd_opt_error(_("Unknown option %s"), argv[i]);
        }
	}

	if (cmd.attach_files && cmd.compose == FALSE) {
		cmd.compose = TRUE;
		cmd.compose_mailto = NULL;
	}
}

static void initial_processing(FolderItem *item, gpointer data)
{
	MainWindow *mainwin = (MainWindow *)data;
	gchar *buf;

	cm_return_if_fail(item);
	buf = g_strdup_printf(_("Processing (%s)..."),
			      item->path
			      ? item->path
			      : _("top level folder"));
	g_free(buf);

	if (folder_item_parent(item) != NULL && item->prefs->enable_processing) {
		item->processing_pending = FALSE;
	}

	STATUSBAR_POP(mainwin);
}

static gboolean draft_all_messages(void)
{
	const GList *compose_list = NULL;

	compose_clear_exit_drafts();
	compose_list = compose_get_compose_list();
	while (compose_list != NULL) {
		Compose *c = (Compose*)compose_list->data;
		if (!compose_draft(c, COMPOSE_DRAFT_FOR_EXIT))
			return FALSE;
		compose_list = compose_get_compose_list();
	}
	return TRUE;
}
gboolean clean_quit(gpointer data)
{
	static gboolean firstrun = TRUE;

	if (!firstrun) {
		return FALSE;
	}
	firstrun = FALSE;

	/*!< Good idea to have the main window stored in a
	 *   static variable so we can check that variable
	 *   to see if we're really allowed to do things
	 *   that actually the spawner is supposed to
	 *   do (like: sending mail, composing messages).
	 *   Because, really, if we're the spawnee, and
	 *   we touch GTK stuff, we're hosed. See the
	 *   next fixme. */

	/* FIXME: Use something else to signal that we're
	 * in the original spawner, and not in a spawned
	 * child. */
	if (!static_mainwindow) {
		return FALSE;
	}

	draft_all_messages();
	emergency_exit = TRUE;
	exit_claws(static_mainwindow);
	exit(0);

	return FALSE;
}

void app_will_exit(GtkWidget *widget, gpointer data)
{
	MainWindow *mainwin = data;

	if (gtk_main_level() == 0) {
		debug_print("not even started\n");
		return;
	}
	if (sc_exiting == TRUE) {
		debug_print("exit pending\n");
		return;
	}
	sc_exiting = TRUE;
	debug_print("exiting\n");
	if (compose_get_compose_list()) {
		if (!draft_all_messages()) {
			main_window_popup(mainwin);
			sc_exiting = FALSE;
			return;
		}
	}

	if (prefs_common.warn_queued_on_exit && procmsg_have_queued_mails_fast()) {
		if (alertpanel(_("Queued messages"),
			       _("Some unsent messages are queued. Exit now?"),
			       NULL, _("_Cancel"), NULL, _("_OK"), NULL, NULL,
			       ALERTFOCUS_FIRST)
		    != G_ALERTALTERNATE) {
			main_window_popup(mainwin);
		    	sc_exiting = FALSE;
			return;
		}
		manage_window_focus_in(mainwin->window, NULL, NULL);
	}

	sock_cleanup();
#ifdef HAVE_VALGRIND
	if (RUNNING_ON_VALGRIND) {
		summary_clear_list(mainwin->summaryview);
	}
#endif
	if (folderview_get_selected_item(mainwin->folderview))
		folder_item_close(folderview_get_selected_item(mainwin->folderview));
	gtk_main_quit();
}

gboolean claws_is_exiting(void)
{
	return sc_exiting;
}

gboolean claws_is_starting(void)
{
	return sc_starting;
}

gchar *claws_get_socket_name(void)
{
	char *dir = g_strdup_printf("%s/claws-mail", g_get_user_runtime_dir());
	struct stat sb;
	int ok = stat(dir, &sb);
	if (ok < 0 && errno != ENOENT) {
		g_print("stat %s: %s\n", dir, g_strerror(errno));
	}
	if (!is_dir_exist(dir) && make_dir(dir) < 0) {
		g_print("create %s: %s\n", dir, g_strerror(errno));
	}
	char *filename = g_strdup_printf("%s/control.sock", dir);
	g_free(dir);

	debug_print("Using control socket %s\n", filename);
	return filename;
}

static gint prohibit_duplicate_launch(int *argc, char ***argv)
{
	gint sock;
	GList *curr;
	gchar *path;

	path = claws_get_socket_name();
	/* Try to connect to the control socket */
	sock = fd_connect_unix(path);

	if (sock < 0) {
		gint ret;
		gchar *socket_lock;
		gint lock_fd;
		/* If connect failed, no other process is running.
		 * Unlink the potentially existing socket, then
		 * open it. This has to be done locking a temporary
		 * file to avoid the race condition where another
		 * process could have created the socket just in
		 * between.
		 */
		socket_lock = g_strconcat(path, ".lock",
					  NULL);
		lock_fd = g_open(socket_lock, O_RDWR|O_CREAT, 0);
		if (lock_fd < 0) {
			debug_print("Couldn't open %s: %s (%d)\n", socket_lock,
				g_strerror(errno), errno);
			g_free(socket_lock);
			return -1;
		}
		if (flock(lock_fd, LOCK_EX) < 0) {
			debug_print("Couldn't lock %s: %s (%d)\n", socket_lock,
				g_strerror(errno), errno);
			close(lock_fd);
			g_free(socket_lock);
			return -1;
		}

		unlink(path);
		debug_print("Opening socket %s\n", path);
		ret = fd_open_unix(path);
		flock(lock_fd, LOCK_UN);
		close(lock_fd);
		unlink(socket_lock);
		g_free(socket_lock);
		return ret;
	}
	/* remote command mode */

	debug_print("another Claws Mail instance is already running.\n");

	if (cmd.receive_all) {
		CM_FD_WRITE_ALL("receive_all\n");
	} else if (cmd.receive) {
		CM_FD_WRITE_ALL("receive\n");
	} else if (cmd.cancel_receiving) {
		CM_FD_WRITE_ALL("cancel_receiving\n");
	} else if (cmd.cancel_sending) {
		CM_FD_WRITE_ALL("cancel_sending\n");
	} else if (cmd.compose && cmd.attach_files) {
		gchar *str, *compose_str;

		if (cmd.compose_mailto) {
			compose_str = g_strdup_printf("compose_attach %s\n",
						      cmd.compose_mailto);
		} else {
			compose_str = g_strdup("compose_attach\n");
		}

		CM_FD_WRITE_ALL(compose_str);
		g_free(compose_str);

		for (curr = cmd.attach_files; curr != NULL ; curr = curr->next) {
			str = (gchar *) ((AttachInfo *)curr->data)->file;
			if (((AttachInfo *)curr->data)->insert)
				CM_FD_WRITE_ALL("insert ");
			else
				CM_FD_WRITE_ALL("attach ");
			CM_FD_WRITE_ALL(str);
			CM_FD_WRITE_ALL("\n");
		}

		CM_FD_WRITE_ALL(".\n");
	} else if (cmd.compose) {
		gchar *compose_str;

		if (cmd.compose_mailto) {
			compose_str = g_strdup_printf
				("compose %s\n", cmd.compose_mailto);
		} else {
			compose_str = g_strdup("compose\n");
		}

		CM_FD_WRITE_ALL(compose_str);
		g_free(compose_str);
	} else if (cmd.subscribe) {
		gchar *str = g_strdup_printf("subscribe %s\n", cmd.subscribe_uri);
		CM_FD_WRITE_ALL(str);
		g_free(str);
	} else if (cmd.send) {
		CM_FD_WRITE_ALL("send\n");
	} else if (cmd.online_mode == ONLINE_MODE_ONLINE) {
		CM_FD_WRITE("online\n");
	} else if (cmd.online_mode == ONLINE_MODE_OFFLINE) {
		CM_FD_WRITE("offline\n");
	} else if (cmd.debug) {
		CM_FD_WRITE("debug\n");
 	} else if (cmd.status || cmd.status_full) {
  		gchar buf[BUFFSIZE];
 		gint i;
 		const gchar *command;
 		GPtrArray *folders;
 		gchar *folder;

 		command = cmd.status_full ? "status-full\n" : "status\n";
 		folders = cmd.status_full ? cmd.status_full_folders :
 			cmd.status_folders;

 		CM_FD_WRITE_ALL(command);
 		for (i = 0; folders && i < folders->len; ++i) {
 			folder = g_ptr_array_index(folders, i);
 			CM_FD_WRITE_ALL(folder);
 			CM_FD_WRITE_ALL("\n");
 		}
 		CM_FD_WRITE_ALL(".\n");
 		for (;;) {
 			fd_gets(sock, buf, sizeof(buf) - 1);
			buf[sizeof(buf) - 1] = '\0';
 			if (!STRNCMP(buf, ".\n")) break;
			if (fputs(buf, stdout) == EOF) {
				g_warning("writing to stdout failed");
				break;
			}
 		}
	} else if (cmd.exit) {
		CM_FD_WRITE_ALL("exit\n");
	} else if (cmd.target) {
		gchar *str = g_strdup_printf("select %s\n", cmd.target);
		CM_FD_WRITE_ALL(str);
		g_free(str);
	} else if (cmd.import_mbox) {
		gchar *str = g_strdup_printf("import %s\n", cmd.import_mbox);
		CM_FD_WRITE_ALL(str);
		g_free(str);
	} else if (cmd.search) {
		gchar buf[BUFFSIZE];
		gchar *str =
			g_strdup_printf("search %s\n%s\n%s\n%c\n",
							cmd.search_folder, cmd.search_type, cmd.search_request,
							(cmd.search_recursive==TRUE)?'1':'0');
		CM_FD_WRITE_ALL(str);
		g_free(str);
		for (;;) {
			fd_gets(sock, buf, sizeof(buf) - 1);
			buf[sizeof(buf) - 1] = '\0';
			if (!STRNCMP(buf, ".\n")) break;
			if (fputs(buf, stdout) == EOF) {
				g_warning("writing to stdout failed");
				break;
			}
		}
	} else {
#ifdef G_OS_UNIX
		gchar buf[BUFSIZ];
		CM_FD_WRITE_ALL("get_display\n");
		memset(buf, 0, sizeof(buf));
		fd_gets(sock, buf, sizeof(buf) - 1);
		buf[sizeof(buf) - 1] = '\0';

		/* Try to connect to a display; if it is the same one as
		 * the other Claws instance, then ask it to pop up. */
		int diff_display = 1;
		if (gtk_init_check(argc, argv)) {
			GdkDisplay *display = gdk_display_get_default();
			diff_display = g_strcmp0(buf, gdk_display_get_name(display));
		}
		if (diff_display) {
			g_print("Claws Mail is already running on display %s.\n",
				buf);
		} else {
			g_print("Claws Mail is already running on this display (%s).\n",
				buf);
			close(sock);
			sock = fd_connect_unix(path);
			CM_FD_WRITE_ALL("popup\n");
		}
#else
		CM_FD_WRITE_ALL("popup\n");
#endif
	}

	close(sock);
	return -1;
}

static gint lock_socket_remove(void)
{
#ifdef G_OS_UNIX
	gchar *filename, *dirname;
#endif
	if (lock_socket < 0) {
		return -1;
	}

	if (lock_socket_tag > 0) {
		g_source_remove(lock_socket_tag);
	}
	close(lock_socket);

#ifdef G_OS_UNIX
	filename = claws_get_socket_name();
	dirname = g_path_get_dirname(filename);
	if (unlink(filename) < 0)
                FILE_OP_ERROR(filename, "unlink");
	g_rmdir(dirname);
	g_free(dirname);
#endif

	return 0;
}

static GPtrArray *get_folder_item_list(gint sock)
{
	gchar buf[BUFFSIZE];
	FolderItem *item;
	GPtrArray *folders = NULL;

	for (;;) {
		fd_gets(sock, buf, sizeof(buf) - 1);
		buf[sizeof(buf) - 1] = '\0';
		if (!STRNCMP(buf, ".\n")) {
			break;
		}
		strretchomp(buf);
		if (!folders) {
			folders = g_ptr_array_new();
		}
		item = folder_find_item_from_identifier(buf);
		if (item) {
			g_ptr_array_add(folders, item);
		} else {
			g_warning("no such folder: %s", buf);
		}
	}

	return folders;
}

static void lock_socket_input_cb(gpointer data,
				 gint source,
				 GIOCondition condition)
{
	MainWindow *mainwin = (MainWindow *)data;
	gint sock;
	gchar buf[BUFFSIZE];

	sock = fd_accept(source);
	if (sock < 0)
		return;

	fd_gets(sock, buf, sizeof(buf) - 1);
	buf[sizeof(buf) - 1] = '\0';

	if (!STRNCMP(buf, "popup")) {
		main_window_popup(mainwin);
#ifdef G_OS_UNIX
	} else if (!STRNCMP(buf, "get_display")) {
		GdkDisplay* display = gtk_widget_get_display(mainwin->window);
		const gchar *display_name = gdk_display_get_name(display);
		CM_FD_WRITE_ALL(display_name);
#endif
	} else if (!STRNCMP(buf, "receive_all")) {
		inc_all_account_mail(mainwin, FALSE, FALSE,
				     prefs_common.newmail_notify_manu);
	} else if (!STRNCMP(buf, "receive")) {
		inc_mail(mainwin, prefs_common.newmail_notify_manu);
	} else if (!STRNCMP(buf, "cancel_receiving")) {
		inc_cancel_all();
		imap_cancel_all();
	} else if (!STRNCMP(buf, "cancel_sending")) {
		send_cancel();
	} else if (!STRNCMP(buf, "compose_attach")) {
		GList *files = NULL, *curr;
		AttachInfo *ainfo;
		gchar *mailto;

		mailto = g_strdup(buf + strlen("compose_attach") + 1);
		while (fd_gets(sock, buf, sizeof(buf) - 1) > 0) {
			buf[sizeof(buf) - 1] = '\0';
			strretchomp(buf);
			if (!g_strcmp0(buf, "."))
				break;

			ainfo = g_new0(AttachInfo, 1);
			ainfo->file = g_strdup(strstr(buf, " ") + 1);
			ainfo->insert = !STRNCMP(buf, "insert ");
			files = g_list_append(files, ainfo);
		}
		open_compose_new(mailto, files);

		curr = g_list_first(files);
		while (curr != NULL) {
			ainfo = (AttachInfo *)curr->data;
			g_free(ainfo->file);
			g_free(ainfo);
			curr = curr->next;
		}
		g_list_free(files);
		g_free(mailto);
	} else if (!STRNCMP(buf, "compose")) {
		open_compose_new(buf + strlen("compose") + 1, NULL);
	} else if (!STRNCMP(buf, "subscribe")) {
		main_window_popup(mainwin);
		folder_subscribe(buf + strlen("subscribe") + 1);
	} else if (!STRNCMP(buf, "send")) {
		send_queue();
	} else if (!STRNCMP(buf, "online")) {
		main_window_toggle_work_offline(mainwin, FALSE, FALSE);
	} else if (!STRNCMP(buf, "offline")) {
		main_window_toggle_work_offline(mainwin, TRUE, FALSE);
	} else if (!STRNCMP(buf, "debug")) {
		debug_set_mode(debug_get_mode() ? FALSE : TRUE);
 	} else if (!STRNCMP(buf, "status-full") ||
 		   !STRNCMP(buf, "status")) {
 		gchar *status;
 		GPtrArray *folders;

 		folders = get_folder_item_list(sock);
 		status = folder_get_status
 			(folders, !STRNCMP(buf, "status-full"));
 		CM_FD_WRITE_ALL(status);
 		CM_FD_WRITE_ALL(".\n");
 		g_free(status);
 		if (folders) g_ptr_array_free(folders, TRUE);
	} else if (!STRNCMP(buf, "select ")) {
		const gchar *target = buf+7;
		mainwindow_jump_to(target, TRUE);
	} else if (!STRNCMP(buf, "import ")) {
		const gchar *mbox_file = buf + 7;
		mainwindow_import_mbox(mbox_file);
	} else if (!STRNCMP(buf, "exit")) {
		if (prefs_common.clean_on_exit && !prefs_common.ask_on_clean) {
			procmsg_empty_all_trash();
                }
		app_will_exit(NULL, mainwin);
	}
	close(sock);

}

static void open_compose_new(const gchar *address, GList *attach_files)
{
	gchar *addr = NULL;

	if (address) {
		Xstrdup_a(addr, address, return);
		g_strstrip(addr);
	}

	compose_new(NULL, addr, attach_files);
}

static void send_queue(void)
{
	GList *list;
	gchar *errstr = NULL;
	gboolean error = FALSE;
	for (list = folder_get_list(); list != NULL; list = list->next) {
		Folder *folder = list->data;

		if (folder->queue) {
			gint res = procmsg_send_queue
				(folder->queue, prefs_common.savemsg,
				&errstr);

			if (res) {
				folder_item_scan(folder->queue);
			}

			if (res < 0)
				error = TRUE;
		}
	}
	if (errstr) {
		alertpanel_error_log(_("Some errors occurred "
				"while sending queued messages:\n%s"), errstr);
		g_free(errstr);
	} else if (error) {
		alertpanel_error_log("Some errors occurred "
				"while sending queued messages.");
	}
}

static void quit_signal_handler(int sig)
{
	debug_print("Quitting on signal %d\n", sig);

	g_timeout_add(0, clean_quit, NULL);
}

static void install_basic_sighandlers()
{
	sigset_t    mask;
	struct sigaction act;

	sigemptyset(&mask);

#ifdef SIGTERM
	sigaddset(&mask, SIGTERM);
#endif
#ifdef SIGINT
	sigaddset(&mask, SIGINT);
#endif
#ifdef SIGHUP
	sigaddset(&mask, SIGHUP);
#endif

	act.sa_handler = quit_signal_handler;
	act.sa_mask    = mask;
	act.sa_flags   = 0;

#ifdef SIGTERM
	sigaction(SIGTERM, &act, 0);
#endif
#ifdef SIGINT
	sigaction(SIGINT, &act, 0);
#endif
#ifdef SIGHUP
	sigaction(SIGHUP, &act, 0);
#endif

	sigprocmask(SIG_UNBLOCK, &mask, 0);
}
