/*
 * Claws Mail -- a GTK based, lightweight, and fast e-mail client
 * Copyright (C) 1999-2025 the Claws Mail team and Hiroyuki Yamamoto
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

#include <glib.h>
#include <glib/gi18n.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <libetpan/libetpan.h>

#include "news.h"
#include "socket.h"
#include "procmsg.h"
#include "procheader.h"
#include "folder.h"
#include "session.h"
#include "statusbar.h"
#include "codeconv.h"
#include "utils.h"
#include "passwordstore.h"
#include "prefs_common.h"
#include "prefs_account.h"
#include "inputdialog.h"
#include "log.h"
#include "progressindicator.h"
#include "remotefolder.h"
#include "alertpanel.h"
#include "inc.h"
#include "account.h"
#ifdef USE_GNUTLS
#  include "ssl.h"
#endif
#include "main.h"
#include "file-utils.h"

#define NNTP_PORT	119
#ifdef USE_GNUTLS
#define NNTPS_PORT	563
#endif

typedef struct _NewsFolder	NewsFolder;
typedef struct _NewsSession	NewsSession;

#define NEWS_FOLDER(obj)	((NewsFolder *)obj)
#define NEWS_SESSION(obj)       ((NewsSession *)obj)

struct _NewsFolder
{
	RemoteFolder rfolder;

	gboolean use_auth;
	gboolean lock_count;
	guint refcnt;
};

struct _NewsSession
{
	Session session;
	Folder *folder;
	gchar *group;
};

static Folder	*news_folder_new	(const gchar	*name,
					 const gchar	*folder);
static void	 news_folder_destroy	(Folder		*folder);

static gchar *news_fetch_msg		(Folder		*folder,
					 FolderItem	*item,
					 gint		 num);
static void news_remove_cached_msg	(Folder 	*folder,
					 FolderItem 	*item,
					 MsgInfo 	*msginfo);
static Session *news_session_new	 (Folder		*folder,
					  const PrefsAccount 	*account,
					  gushort		 port,
					  SSLType		 ssl_type);

static gint news_get_article		 (Folder	*folder,
					  gint		 num,
					  gchar		*filename);

static gint news_select_group		 (Folder	*folder,
					  const gchar	*group,
					  gint		*num,
					  gint		*first,
					  gint		*last);
static MsgInfo *news_parse_xover	 (struct newsnntp_xover_resp_item *item);
static gint news_get_num_list		 	 (Folder 	*folder,
					  FolderItem 	*item,
					  GSList       **list,
					  gboolean	*old_uids_valid);
static MsgInfo *news_get_msginfo		 (Folder 	*folder,
					  FolderItem 	*item,
					  gint 		 num);
static GSList *news_get_msginfos		 (Folder 	*folder,
					  FolderItem 	*item,
					  GSList 	*msgnum_list);
static gboolean news_scan_required		 (Folder 	*folder,
					  FolderItem 	*item);

static gchar *news_folder_get_path	 (Folder	*folder);
static gchar *news_item_get_path		 (Folder	*folder,
					  FolderItem	*item);
static void news_synchronise		 (FolderItem	*item, gint days);
static int news_remove_msg		 (Folder 	*folder,
					  FolderItem 	*item,
					  gint 		 msgnum);
static gint news_rename_folder		 (Folder *folder,
					  FolderItem *item,
					  const gchar *name);
static gint news_remove_folder		 (Folder	*folder,
					  FolderItem	*item);
static FolderClass news_class;

FolderClass *news_get_class(void)
{
	if (news_class.idstr == NULL) {
		news_class.type = F_NEWS;
		news_class.idstr = "news";
		news_class.uistr = "News";
		news_class.supports_server_search = FALSE;

		/* Folder functions */
		news_class.new_folder = news_folder_new;
		news_class.destroy_folder = news_folder_destroy;

		/* FolderItem functions */
		news_class.item_get_path = news_item_get_path;
		news_class.get_num_list = news_get_num_list;
		news_class.scan_required = news_scan_required;
		news_class.rename_folder = news_rename_folder;
		news_class.remove_folder = news_remove_folder;

		/* Message functions */
		news_class.get_msginfo = news_get_msginfo;
		news_class.get_msginfos = news_get_msginfos;
		news_class.fetch_msg = news_fetch_msg;
		news_class.synchronise = news_synchronise;
		news_class.search_msgs = folder_item_search_msgs_local;
		news_class.remove_msg = news_remove_msg;
		news_class.remove_cached_msg = news_remove_cached_msg;
	};

	return &news_class;
}

guint nntp_folder_get_refcnt(Folder *folder)
{
	return ((NewsFolder *)folder)->refcnt;
}

void nntp_folder_ref(Folder *folder)
{
	((NewsFolder *)folder)->refcnt++;
}

void nntp_folder_unref(Folder *folder)
{
	if (((NewsFolder *)folder)->refcnt > 0)
		((NewsFolder *)folder)->refcnt--;
}

static int news_remove_msg		 (Folder 	*folder,
					  FolderItem 	*item,
					  gint 		 msgnum)
{
	gchar *path, *filename;

	cm_return_val_if_fail(folder != NULL, -1);
	cm_return_val_if_fail(item != NULL, -1);

	path = folder_item_get_path(item);
	if (!is_dir_exist(path))
		make_dir_hier(path);

	filename = g_strconcat(path, G_DIR_SEPARATOR_S, itos(msgnum), NULL);
	g_free(path);
	unlink(filename);
	g_free(filename);
	return 0;
}

static void news_folder_lock(NewsFolder *folder)
{
	folder->lock_count++;
}

static void news_folder_unlock(NewsFolder *folder)
{
	if (folder->lock_count > 0)
		folder->lock_count--;
}

int news_folder_locked(Folder *folder)
{
	if (folder == NULL)
		return 0;

	return NEWS_FOLDER(folder)->lock_count;
}

static Folder *news_folder_new(const gchar *name, const gchar *path)
{
	Folder *folder;
	folder = (Folder *)g_new0(NewsFolder, 1);
	folder->klass = &news_class;
	folder_init(folder, name);
	return folder;
}

static void news_folder_destroy(Folder *folder)
{
	gchar *dir;

	while (nntp_folder_get_refcnt(folder) > 0)
		gtk_main_iteration();

	dir = news_folder_get_path(folder);
	if (is_dir_exist(dir))
		remove_dir_recursive(dir);
	g_free(dir);

	RemoteFolder *rfolder = REMOTE_FOLDER(folder);
	if (rfolder->session)
		session_destroy(rfolder->session);
}

static void news_session_destroy(Session *session)
{
	NewsSession *news_session = NEWS_SESSION(session);

	cm_return_if_fail(session != NULL);

	if (news_session->group)
		g_free(news_session->group);
}

static gboolean nntp_ping(gpointer data)
{
	fprintf(stderr, "TODO(otl): delete nntp_ping\n");
	return FALSE;
}


static Session *news_session_new(Folder *folder, const PrefsAccount *account, gushort port, SSLType ssl_type)
{
	fprintf(stderr, "TODO(otl): delete news_session_new\n");
	return NULL;
}

static Session *news_session_new_for_folder(Folder *folder)
{
	fprintf(stderr, "TODO(otl): delete news_session_new_for_folder\n");
	return NULL;
}

static void news_remove_cached_msg(Folder *folder, FolderItem *item, MsgInfo *msginfo)
{
	gchar *path, *filename;

	path = folder_item_get_path(item);

	if (!is_dir_exist(path)) {
		g_free(path);
		return;
	}

	filename = g_strconcat(path, G_DIR_SEPARATOR_S, itos(msginfo->msgnum), NULL);
	g_free(path);

	if (is_file_exist(filename)) {
		unlink(filename);
	}
	g_free(filename);
}

static gchar *news_fetch_msg(Folder *folder, FolderItem *item, gint num)
{
	fprintf(stderr, "TODO: delete news_fetch_msg\n");
	return NULL;
}

static NewsGroupInfo *news_group_info_new(const gchar *name,
					  gint first, gint last, gchar type)
{
	NewsGroupInfo *ginfo;

	ginfo = g_new(NewsGroupInfo, 1);
	ginfo->name = g_strdup(name);
	ginfo->first = first;
	ginfo->last = last;
	ginfo->type = type;

	return ginfo;
}

static void news_group_info_free(NewsGroupInfo *ginfo)
{
	g_free(ginfo->name);
	g_free(ginfo);
}

static gint news_group_info_compare(NewsGroupInfo *ginfo1,
				    NewsGroupInfo *ginfo2)
{
	return g_ascii_strcasecmp(ginfo1->name, ginfo2->name);
}

GSList *news_get_group_list(Folder *folder)
{
	fprintf(stderr, "TODO(otl): delete news_get_group_list\n");
	return NULL;
}

void news_group_list_free(GSList *group_list)
{
	GSList *cur;

	if (!group_list) return;

	for (cur = group_list; cur != NULL; cur = cur->next)
		news_group_info_free((NewsGroupInfo *)cur->data);
	g_slist_free(group_list);
}

void news_remove_group_list_cache(Folder *folder)
{
	gchar *path, *filename;

	cm_return_if_fail(folder != NULL);
	cm_return_if_fail(FOLDER_CLASS(folder) == &news_class);

	path = folder_item_get_path(FOLDER_ITEM(folder->node->data));
	filename = g_strconcat(path, G_DIR_SEPARATOR_S, NEWSGROUP_LIST, NULL);
	g_free(path);

	if (is_file_exist(filename)) {
		if (unlink(filename) < 0)
			FILE_OP_ERROR(filename, "remove");
	}
	g_free(filename);
}

int news_post(Folder *folder, const gchar *file)
{
	fprintf(stderr, "TODO(otl): delete news_post\n");
	return -1;
}

static int news_get_article(Folder *folder, gint num, gchar *filename)
{
	fprintf(stderr, "TODO(otl): delete news_post\n");
	return -1;
}

/**
 * news_select_group:
 * @session: Active NNTP session.
 * @group: Newsgroup name.
 * @num: Estimated number of articles.
 * @first: First article number.
 * @last: Last article number.
 *
 * Select newsgroup @group with the GROUP command if it is not already
 * selected in @session, or article numbers need to be returned.
 *
 * Return value: NNTP result code.
 **/
static gint news_select_group(Folder *folder, const gchar *group,
			      gint *num, gint *first, gint *last)
{
	fprintf(stderr, "TODO(otl): delete news_select_group\n");
	return -1;
}

static MsgInfo *news_parse_xover(struct newsnntp_xover_resp_item *item)
{
	MsgInfo *msginfo;

	/* set MsgInfo */
	msginfo = procmsg_msginfo_new();
	msginfo->msgnum = item->ovr_article;
	msginfo->size = item->ovr_size;

	msginfo->date = g_strdup(item->ovr_date);
	msginfo->date_t = procheader_date_parse(NULL, item->ovr_date, 0);

	msginfo->from = conv_unmime_header(item->ovr_author, NULL, TRUE);
	msginfo->fromname = procheader_get_fromname(msginfo->from);

	msginfo->subject = conv_unmime_header(item->ovr_subject, NULL, FALSE);

	remove_return(msginfo->from);
	remove_return(msginfo->fromname);
	remove_return(msginfo->subject);

        if (item->ovr_message_id) {
		gchar *tmp = g_strdup(item->ovr_message_id);
                extract_parenthesis(tmp, '<', '>');
                remove_space(tmp);
                if (*tmp != '\0')
                        msginfo->msgid = g_strdup(tmp);
		g_free(tmp);
        }

        /* FIXME: this is a quick fix; references' meaning was changed
         * into having the actual list of references in the References: header.
         * We need a GSList here, so msginfo_free() and msginfo_copy() can do
         * their things properly. */
        if (item->ovr_references && *(item->ovr_references)) {
		gchar **ref_tokens = g_strsplit(item->ovr_references, " ", -1);
		guint i = 0;
		char *tmp;
		char *p;
		while (ref_tokens[i]) {
			gchar *cur_ref = ref_tokens[i];
			msginfo->references = references_list_append(msginfo->references,
					cur_ref);
			i++;
		}
		g_strfreev(ref_tokens);

		tmp = g_strdup(item->ovr_references);
                eliminate_parenthesis(tmp, '(', ')');
                if ((p = strrchr(tmp, '<')) != NULL) {
                        extract_parenthesis(p, '<', '>');
                        remove_space(p);
                        if (*p != '\0')
                                msginfo->inreplyto = g_strdup(p);
                }
		g_free(tmp);
	}

	return msginfo;
}

gint news_cancel_article(Folder * folder, MsgInfo * msginfo)
{
	gchar * tmp;
	FILE * tmpfp;
	gchar date[RFC822_DATE_BUFFSIZE];

	tmp = g_strdup_printf("%s%ccancel%p", get_tmp_dir(),
			      G_DIR_SEPARATOR, msginfo);
	if (tmp == NULL)
		return -1;

	if ((tmpfp = g_fopen(tmp, "wb")) == NULL) {
		FILE_OP_ERROR(tmp, "g_fopen");
		g_free(tmp);
		return -1;
	}
	if (change_file_mode_rw(tmpfp, tmp) < 0) {
		FILE_OP_ERROR(tmp, "chmod");
		g_warning("can't change file mode");
	}

	if (prefs_common.hide_timezone)
		get_rfc822_date_hide_tz(date, sizeof(date));
	else
		get_rfc822_date(date, sizeof(date));
	if (fprintf(tmpfp, "From: %s\r\n"
		       "Newsgroups: %s\r\n"
		       "Subject: cmsg cancel <%s>\r\n"
		       "Control: cancel <%s>\r\n"
		       "X-Cancelled-by: %s\r\n"
		       "Date: %s\r\n"
		       "\r\n"
		       "removed with Claws Mail\r\n",
		       msginfo->from,
		       msginfo->newsgroups,
		       msginfo->msgid,
		       msginfo->msgid,
		       msginfo->from,
		       date) < 0) {
		FILE_OP_ERROR(tmp, "fprintf");
		fclose(tmpfp);
		unlink(tmp);
		g_free(tmp);
		return -1;
	}

	if (safe_fclose(tmpfp) == EOF) {
		FILE_OP_ERROR(tmp, "fclose");
		unlink(tmp);
		g_free(tmp);
		return -1;
	}

	news_post(folder, tmp);
	unlink(tmp);

	g_free(tmp);

	return 0;
}

static gchar *news_folder_get_path(Folder *folder)
{
	gchar *folder_path;

        cm_return_val_if_fail(folder->account != NULL, NULL);

        folder_path = g_strconcat(get_news_cache_dir(),
                                  G_DIR_SEPARATOR_S,
                                  folder->account->nntp_server,
                                  NULL);
	return folder_path;
}

static gchar *news_item_get_path(Folder *folder, FolderItem *item)
{
	gchar *folder_path, *path;

	cm_return_val_if_fail(folder != NULL, NULL);
	cm_return_val_if_fail(item != NULL, NULL);
	folder_path = news_folder_get_path(folder);

        cm_return_val_if_fail(folder_path != NULL, NULL);
        if (g_path_is_absolute(folder_path)) {
                if (item->path)
                        path = g_strconcat(folder_path, G_DIR_SEPARATOR_S,
                                           item->path, NULL);
                else
                        path = g_strdup(folder_path);
        } else {
                if (item->path)
                        path = g_strconcat(get_home_dir(), G_DIR_SEPARATOR_S,
                                           folder_path, G_DIR_SEPARATOR_S,
                                           item->path, NULL);
                else
                        path = g_strconcat(get_home_dir(), G_DIR_SEPARATOR_S,
                                           folder_path, NULL);
        }
        g_free(folder_path);
	return path;
}

static gint news_get_num_list(Folder *folder, FolderItem *item, GSList **msgnum_list, gboolean *old_uids_valid)
{
	fprintf(stderr, "TODO: delete news_get_num_list\n");
	return -1;
}

static void news_set_msg_flags(FolderItem *item, MsgInfo *msginfo)
{
	msginfo->flags.tmp_flags = 0;
	if (item->folder->account->mark_crosspost_read && msginfo->msgid) {
		if (item->folder->newsart &&
		    g_hash_table_lookup(item->folder->newsart, msginfo->msgid) != NULL) {
			msginfo->flags.perm_flags = MSG_COLORLABEL_TO_FLAGS(item->folder->account->crosspost_col);

		} else {
			if (!item->folder->newsart)
				item->folder->newsart = g_hash_table_new(g_str_hash, g_str_equal);
			g_hash_table_insert(item->folder->newsart,
					g_strdup(msginfo->msgid), GINT_TO_POINTER(1));
			msginfo->flags.perm_flags = MSG_NEW|MSG_UNREAD;
		}
	} else {
		msginfo->flags.perm_flags = MSG_NEW|MSG_UNREAD;
	}
}

static void news_get_extra_fields(NewsSession *session, FolderItem *item, GSList *msglist)
{
	fprintf(stderr, "TODO(otl): delete news_get_extra_fields\n");
	return;
}

static GSList *news_get_msginfos_for_range(NewsSession *session, FolderItem *item, guint begin, guint end)
{
	fprintf(stderr, "TODO(otl): delete news_get_msginfos_for_range\n");
	return NULL;
}

static MsgInfo *news_get_msginfo(Folder *folder, FolderItem *item, gint num)
{
	fprintf(stderr, "TODO: delete news_get_msginfo\n");
	return NULL;
}

static GSList *news_get_msginfos(Folder *folder, FolderItem *item, GSList *msgnum_list)
{
	fprintf(stderr, "TODO: delete news_get_msginfos\n");
	return NULL;
}

static gboolean news_scan_required(Folder *folder, FolderItem *item)
{
	return TRUE;
}

void news_synchronise(FolderItem *item, gint days)
{
}

static gint news_rename_folder(Folder *folder, FolderItem *item,
				const gchar *name)
{
	gchar *path;

	cm_return_val_if_fail(folder != NULL, -1);
	cm_return_val_if_fail(item != NULL, -1);
	cm_return_val_if_fail(item->path != NULL, -1);
	cm_return_val_if_fail(name != NULL, -1);

	path = folder_item_get_path(item);
	if (!is_dir_exist(path))
		make_dir_hier(path);

	g_free(item->name);
	item->name = g_strdup(name);

	return 0;
}

static gint news_remove_folder(Folder *folder, FolderItem *item)
{
	gchar *path;

	cm_return_val_if_fail(folder != NULL, -1);
	cm_return_val_if_fail(item != NULL, -1);
	cm_return_val_if_fail(item->path != NULL, -1);

	path = folder_item_get_path(item);
	if (remove_dir_recursive(path) < 0) {
		g_warning("can't remove directory '%s'", path);
		g_free(path);
		return -1;
	}

	g_free(path);
	folder_item_remove(item);
	return 0;
}

void nntp_disconnect_all(gboolean have_connectivity)
{
	GList *list;

	for (list = account_get_list(); list != NULL; list = list->next) {
		PrefsAccount *account = list->data;
		if (account->protocol == A_NNTP) {
			RemoteFolder *folder = (RemoteFolder *)account->folder;
			if (folder && folder->session) {
				NewsSession *session = (NewsSession *)folder->session;
				SESSION(session)->state = SESSION_DISCONNECTED;
				SESSION(session)->sock = NULL;
				session_destroy(SESSION(session));
				folder->session = NULL;
			}
		}
	}
}

