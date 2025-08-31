/*
 * Claws Mail -- a GTK based, lightweight, and fast e-mail client
 * Copyright (C) 1999-2021 the Claws Mail team and Hiroyuki Yamamoto
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

#include <glib.h>
#include <glib/gi18n.h>

#include "defs.h"
#include "folder.h"
#include "matcher.h"
#include "utils.h"
#include "prefs_gtk.h"
#include "folder_item_prefs.h"

FolderItemPrefs tmp_prefs;

static PrefParam param[] = {
	{"enable_default_from", "", &tmp_prefs.enable_default_from, P_BOOL,
	 NULL, NULL, NULL},
	{"default_from", "", &tmp_prefs.default_from, P_STRING,
	 NULL, NULL, NULL},
	{"enable_default_to", "", &tmp_prefs.enable_default_to, P_BOOL,
	 NULL, NULL, NULL},
	{"default_to", "", &tmp_prefs.default_to, P_STRING,
	 NULL, NULL, NULL},
	{"enable_default_reply_to", "", &tmp_prefs.enable_default_reply_to, P_BOOL,
	 NULL, NULL, NULL},
	{"default_reply_to", "", &tmp_prefs.default_reply_to, P_STRING,
	 NULL, NULL, NULL},
	{"enable_default_cc", "", &tmp_prefs.enable_default_cc, P_BOOL,
	 NULL, NULL, NULL},
	{"default_cc", "", &tmp_prefs.default_cc, P_STRING,
	 NULL, NULL, NULL},
	{"enable_default_bcc", "", &tmp_prefs.enable_default_bcc, P_BOOL,
	 NULL, NULL, NULL},
	{"default_bcc", "", &tmp_prefs.default_bcc, P_STRING,
	 NULL, NULL, NULL},
	{"enable_default_replyto", "", &tmp_prefs.enable_default_replyto, P_BOOL,
	 NULL, NULL, NULL},
	{"default_replyto", "", &tmp_prefs.default_replyto, P_STRING,
	 NULL, NULL, NULL},
	{"enable_simplify_subject", "", &tmp_prefs.enable_simplify_subject, P_BOOL,
	 NULL, NULL, NULL},
	{"simplify_subject_regexp", "", &tmp_prefs.simplify_subject_regexp, P_STRING,
	 NULL, NULL, NULL},
	{"enable_folder_chmod", "", &tmp_prefs.enable_folder_chmod, P_BOOL,
	 NULL, NULL, NULL},
	{"folder_chmod", "", &tmp_prefs.folder_chmod, P_INT,
	 NULL, NULL, NULL},
	{"enable_default_account", "", &tmp_prefs.enable_default_account, P_BOOL,
	 NULL, NULL, NULL},
	{"default_account", NULL, &tmp_prefs.default_account, P_INT,
	 NULL, NULL, NULL},
	{"always_sign", "0", &tmp_prefs.always_sign, P_ENUM,
 	 NULL, NULL, NULL},
	{"always_encrypt", "0", &tmp_prefs.always_encrypt, P_ENUM,
 	 NULL, NULL, NULL},
	{"save_copy_to_folder", NULL, &tmp_prefs.save_copy_to_folder, P_BOOL,
	 NULL, NULL, NULL},
	{"enable_processing", "FALSE", &tmp_prefs.enable_processing, P_BOOL,
	 NULL, NULL, NULL},
	{"enable_processing_when_opening", "TRUE", &tmp_prefs.enable_processing_when_opening, P_BOOL,
	 NULL, NULL, NULL},
	{"newmailcheck", "TRUE", &tmp_prefs.newmailcheck, P_BOOL,
	 NULL, NULL, NULL},
	{"offlinesync", "FALSE", &tmp_prefs.offlinesync, P_BOOL,
	 NULL, NULL, NULL},
	{"offlinesync_days", "0", &tmp_prefs.offlinesync_days, P_INT,
	 NULL, NULL, NULL},
	{"remove_old_bodies", "FALSE", &tmp_prefs.remove_old_bodies, P_BOOL,
	 NULL, NULL, NULL},
	{"render_html", "0", &tmp_prefs.render_html, P_ENUM,
	 NULL, NULL, NULL},
	{"skip_on_goto_unread_or_new", "FALSE", &tmp_prefs.skip_on_goto_unread_or_new, P_BOOL,
	 NULL, NULL, NULL},

	{"config_version", "-1", &tmp_prefs.config_version, P_INT,
	 NULL, NULL, NULL},
	{NULL, NULL, NULL, P_OTHER, NULL, NULL, NULL}
};

static FolderItemPrefs *folder_item_prefs_clear(FolderItemPrefs *prefs);

void folder_item_prefs_read_config(FolderItem * item)
{
	gchar * id;
	gchar *rcpath;

	id = folder_item_get_identifier(item);
	folder_item_prefs_clear(&tmp_prefs);
	rcpath = g_strconcat(get_rc_dir(), G_DIR_SEPARATOR_S, FOLDERITEM_RC, NULL);
	prefs_read_config(param, id, rcpath, NULL);
	g_free(id);
	g_free(rcpath);

	*item->prefs = tmp_prefs;
}

void folder_item_prefs_save_config(FolderItem * item)
{
	gchar * id;

	tmp_prefs = * item->prefs;

	id = folder_item_get_identifier(item);
	if (id == NULL)
		return;

	debug_print("saving prefs for %s\n", id);
	prefs_write_config(param, id, FOLDERITEM_RC);
	g_free(id);
}

static gboolean folder_item_prefs_save_config_func(GNode *node, gpointer data)
{
	FolderItem *item = (FolderItem *) node->data;
	folder_item_prefs_save_config(item);
	return FALSE;
}

void folder_item_prefs_save_config_recursive(FolderItem * item)
{
	g_node_traverse(item->node, G_PRE_ORDER, G_TRAVERSE_ALL,
			-1, folder_item_prefs_save_config_func, NULL);
}

void folder_prefs_save_config_recursive(Folder *folder)
{
	g_node_traverse(folder->node, G_PRE_ORDER, G_TRAVERSE_ALL,
			-1, folder_item_prefs_save_config_func, NULL);
}

static FolderItemPrefs *folder_item_prefs_clear(FolderItemPrefs *prefs)
{
	prefs->enable_default_from = FALSE;
	prefs->default_from = NULL;
	prefs->enable_default_to = FALSE;
	prefs->default_to = NULL;
	prefs->enable_default_reply_to = FALSE;
	prefs->default_reply_to = NULL;
	prefs->enable_default_cc = FALSE;
	prefs->default_cc = NULL;
	prefs->enable_default_bcc = FALSE;
	prefs->default_bcc = NULL;
	prefs->enable_default_replyto = FALSE;
	prefs->default_replyto = NULL;
	prefs->enable_simplify_subject = FALSE;
	prefs->simplify_subject_regexp = NULL;
	prefs->enable_folder_chmod = FALSE;
	prefs->folder_chmod = 0;
	prefs->enable_default_account = FALSE;
	prefs->default_account = 0;
	prefs->always_sign = SIGN_OR_ENCRYPT_DEFAULT;
	prefs->always_encrypt = SIGN_OR_ENCRYPT_DEFAULT;
	prefs->save_copy_to_folder = FALSE;

	prefs->enable_processing = FALSE;
	prefs->enable_processing_when_opening = FALSE;
	prefs->processing = NULL;

	prefs->newmailcheck = TRUE;
	prefs->offlinesync = FALSE;
	prefs->offlinesync_days = 0;
	prefs->remove_old_bodies = FALSE;
	prefs->render_html = HTML_RENDER_DEFAULT;
	prefs->skip_on_goto_unread_or_new = FALSE;
	return prefs;
}

FolderItemPrefs * folder_item_prefs_new(void)
{
	FolderItemPrefs * prefs;

	prefs = g_new0(FolderItemPrefs, 1);

	return folder_item_prefs_clear(prefs);
}

void folder_item_prefs_free(FolderItemPrefs * prefs)
{
	g_free(prefs->default_from);
	g_free(prefs->default_to);
	g_free(prefs->default_reply_to);
	g_free(prefs->default_cc);
	g_free(prefs->default_bcc);
	g_free(prefs->default_replyto);
	g_free(prefs);
}

#define SAFE_STRING(str) \
	(str) ? (str) : ""

void folder_item_prefs_copy_prefs(FolderItem * src, FolderItem * dest)
{
	GSList *tmp_prop_list = NULL;
	folder_item_prefs_read_config(src);

	tmp_prefs.directory			= g_strdup(src->prefs->directory);
        tmp_prefs.enable_processing             = src->prefs->enable_processing;
        tmp_prefs.enable_processing_when_opening = src->prefs->enable_processing_when_opening;
	tmp_prefs.newmailcheck                  = src->prefs->newmailcheck;
	tmp_prefs.offlinesync                   = src->prefs->offlinesync;
	tmp_prefs.offlinesync_days              = src->prefs->offlinesync_days;
	tmp_prefs.remove_old_bodies             = src->prefs->remove_old_bodies;
	tmp_prefs.render_html                   = src->prefs->render_html;
	tmp_prefs.skip_on_goto_unread_or_new    = src->prefs->skip_on_goto_unread_or_new;

	prefs_matcher_read_config();

	tmp_prefs.processing			= tmp_prop_list;

	tmp_prefs.enable_default_from		= src->prefs->enable_default_from;
	tmp_prefs.default_from			= g_strdup(src->prefs->default_from);
	tmp_prefs.enable_default_to		= src->prefs->enable_default_to;
	tmp_prefs.default_to			= g_strdup(src->prefs->default_to);
	tmp_prefs.enable_default_reply_to	= src->prefs->enable_default_reply_to;
	tmp_prefs.default_reply_to		= g_strdup(src->prefs->default_reply_to);
	tmp_prefs.enable_default_cc		= src->prefs->enable_default_cc;
	tmp_prefs.default_cc			= g_strdup(src->prefs->default_cc);
	tmp_prefs.enable_default_bcc		= src->prefs->enable_default_bcc;
	tmp_prefs.default_bcc			= g_strdup(src->prefs->default_bcc);
	tmp_prefs.enable_default_replyto		= src->prefs->enable_default_replyto;
	tmp_prefs.default_replyto			= g_strdup(src->prefs->default_replyto);
	tmp_prefs.enable_simplify_subject	= src->prefs->enable_simplify_subject;
	tmp_prefs.simplify_subject_regexp	= g_strdup(src->prefs->simplify_subject_regexp);
	tmp_prefs.enable_folder_chmod		= src->prefs->enable_folder_chmod;
	tmp_prefs.folder_chmod			= src->prefs->folder_chmod;
	tmp_prefs.enable_default_account	= src->prefs->enable_default_account;
	tmp_prefs.default_account		= src->prefs->default_account;
	tmp_prefs.always_sign    	= src->prefs->always_sign;
	tmp_prefs.always_encrypt    = src->prefs->always_encrypt;
	tmp_prefs.save_copy_to_folder		= src->prefs->save_copy_to_folder;

	*dest->prefs = tmp_prefs;
	folder_item_prefs_save_config(dest);
	prefs_matcher_write_config();

	dest->collapsed = src->collapsed;
	dest->thread_collapsed = src->thread_collapsed;
	dest->threaded  = src->threaded;
	dest->hide_read_msgs = src->hide_read_msgs;
	dest->hide_del_msgs = src->hide_del_msgs;
	dest->hide_read_threads = src->hide_read_threads;
	dest->sort_key  = src->sort_key;
	dest->sort_type = src->sort_type;
}
