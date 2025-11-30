/*
 * Claws Mail -- a GTK based, lightweight, and fast e-mail client
 * Copyright (C) 1999-2022 the Claws Mail team and Hiroyuki Yamamoto
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

#include <stdio.h>
#include "defs.h"
#include <glib.h>
#include <glib/gi18n.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>

#include "mbox.h"
#include "procmsg.h"
#include "folder.h"
#include "prefs_common.h"
#include "prefs_account.h"
#include "account.h"
#include "utils.h"
#include "alertpanel.h"
#include "statusbar.h"
#include "file-utils.h"

#define FPUTS_TO_TMP_ABORT_IF_FAIL(s) \
{ \
	lines++; \
	if (fputs(s, tmp_fp) == EOF) { \
		g_warning("can't write to temporary file"); \
		fclose(tmp_fp); \
		fclose(mbox_fp); \
		unlink(tmp_file); \
		g_free(tmp_file); \
		return -1; \
	} \
}

gint proc_mbox(FolderItem *dest, const gchar *mbox, PrefsAccount *account)
/* return values: -1 error, >=0 number of msgs added */
{
	FILE *mbox_fp;
	char buf[BUFSIZ];
	gchar *tmp_file;
	gint msgs = 0;
	gint lines;
	gboolean more;
	GSList *to_add = NULL;
	gboolean printed = FALSE;
	FolderItem *dropfolder;
	GStatBuf src_stat;

	cm_return_val_if_fail(dest != NULL, -1);
	cm_return_val_if_fail(mbox != NULL, -1);

	debug_print("Getting messages from %s into %s...\n", mbox, dest->path);

	if (g_stat(mbox, &src_stat) < 0) {
		FILE_OP_ERROR(mbox, "g_stat");
		alertpanel_error(_("Could not stat mbox file:\n%s\n"), mbox);
		return -1;
	}

	if ((mbox_fp = g_fopen(mbox, "rb")) == NULL) {
		FILE_OP_ERROR(mbox, "g_fopen");
		alertpanel_error(_("Could not open mbox file:\n%s\n"), mbox);
		return -1;
	}

	/* ignore empty lines on the head */
	do {
		if (fgets(buf, sizeof(buf), mbox_fp) == NULL) {
			g_warning("can't read mbox file");
			fclose(mbox_fp);
			return -1;
		}
	} while (buf[0] == '\n' || buf[0] == '\r');

	if (strncmp(buf, "From ", 5) != 0) {
		g_warning("invalid mbox format: %s", mbox);
		fclose(mbox_fp);
		return -1;
	}

	tmp_file = get_tmp_file();

	folder_item_update_freeze();

	dropfolder = dest;

	do {
		FILE *tmp_fp;
		gint empty_lines;

		if (msgs%10 == 0) {
			long cur_offset_mb = ftell(mbox_fp) / (1024 * 1024);
			if (printed)
				statusbar_pop_all();
			statusbar_print_all("Importing from mbox... (%ld MB imported)", cur_offset_mb);
			statusbar_progress_all(cur_offset_mb, src_stat.st_size / (1024*1024), 1);
			printed = TRUE;
			GTK_EVENTS_FLUSH();
		}

		if ((tmp_fp = g_fopen(tmp_file, "wb")) == NULL) {
			FILE_OP_ERROR(tmp_file, "g_fopen");
			g_warning("can't open temporary file");
			fclose(mbox_fp);
			g_free(tmp_file);
			return -1;
		}

		empty_lines = 0;
		lines = 0;

		/* process all lines from mboxrc file */
		while (fgets(buf, sizeof(buf), mbox_fp) != NULL) {
			int offset;

			/* eat empty lines */
			if (buf[0] == '\n' || buf[0] == '\r') {
				empty_lines++;
				continue;
			}

			/* From separator or quoted From */
			offset = 0;
			/* detect leading '>' char(s) */
			while (buf[offset] == '>') {
				offset++;
			}
			if (!strncmp(buf+offset, "From ", 5)) {
				/* From separator: */
				if (offset == 0) {
					/* expect next mbox item */
					break;
				}

				/* quoted From: */
				/* flush any eaten empty line */
				if (empty_lines > 0) {
					while (empty_lines-- > 0) {
						FPUTS_TO_TMP_ABORT_IF_FAIL("\n");
				}
					empty_lines = 0;
				}
				/* store the unquoted line */
				FPUTS_TO_TMP_ABORT_IF_FAIL(buf + 1);
				continue;
			}

			/* other line */
			/* flush any eaten empty line */
			if (empty_lines > 0) {
				while (empty_lines-- > 0) {
					FPUTS_TO_TMP_ABORT_IF_FAIL("\n");
			}
				empty_lines = 0;
			}
			/* store the line itself */
					FPUTS_TO_TMP_ABORT_IF_FAIL(buf);
		}
		/* end of mbox item or end of mbox */

		/* flush any eaten empty line (but the last one) */
		if (empty_lines > 0) {
			while (--empty_lines > 0) {
				FPUTS_TO_TMP_ABORT_IF_FAIL("\n");
			}
		}

		/* more emails to expect? */
		more = !feof(mbox_fp);

		/* warn if email part is empty (it's the minimum check
		   we can do */
		if (lines == 0) {
			g_warning("malformed mbox: %s: message %d is empty", mbox, msgs);
			fclose(tmp_fp);
			fclose(mbox_fp);
			unlink(tmp_file);
			return -1;
		}

		if (fclose(tmp_fp) == EOF) {
			FILE_OP_ERROR(tmp_file, "fclose");
			g_warning("can't write to temporary file");
			fclose(mbox_fp);
			unlink(tmp_file);
			g_free(tmp_file);
			return -1;
		}


		MsgFileInfo *finfo = g_new0(MsgFileInfo, 1);
		finfo->file = tmp_file;

		to_add = g_slist_prepend(to_add, finfo);
		tmp_file = get_tmp_file();

		/* flush every 500 */
		if (msgs > 0 && msgs % 500 == 0) {
			folder_item_add_msgs(dropfolder, to_add, TRUE);
			procmsg_message_file_list_free(to_add);
			to_add = NULL;
		}
		msgs++;
	} while (more);

	if (printed) {
		statusbar_pop_all();
		statusbar_progress_all(0, 0, 0);
	}

	if (to_add) {
		folder_item_add_msgs(dropfolder, to_add, TRUE);
		procmsg_message_file_list_free(to_add);
		to_add = NULL;
	}

	folder_item_update_thaw();

	g_free(tmp_file);
	fclose(mbox_fp);
	debug_print("%d messages found.\n", msgs);

	return msgs;
}

int copy_mbox(gint srcfd, const gchar *dest) {
	ssize_t n_read;
	char buf[BUFSIZ];

	FILE *dest_fp = fopen(dest, "w");
	if (dest_fp == NULL) {
		warn("open %s", dest);
		return -1;
	}
	if (chmod(dest, S_IRUSR|S_IWUSR) < 0)
		warn("chmod %s", dest);

	while ((n_read = read(srcfd, buf, sizeof(buf))) > 0) {
		if (fwrite(buf, 1, n_read, dest_fp) < n_read) {
			g_warning("writing to %s failed", dest);
			fclose(dest_fp);
			unlink(dest);
			return -1;
		}
	}
	return fclose(dest_fp);
}

gint export_list_to_mbox(GSList *mlist, const gchar *mbox)
/* return values: -2 skipped, -1 error, 0 OK */
{
	GSList *cur;
	MsgInfo *msginfo;
	FILE *msg_fp;
	FILE *mbox_fp;
	gchar buf[BUFFSIZE];
	int err = 0;

	gint msgs = 1, total = g_slist_length(mlist);
	if (g_file_test(mbox, G_FILE_TEST_EXISTS) == TRUE) {
		if (alertpanel_full(_("Overwrite mbox file"),
					_("This file already exists. Do you want to overwrite it?"),
					NULL, _("_Cancel"), NULL, _("Overwrite"), NULL, NULL,
					ALERTFOCUS_FIRST, FALSE, NULL, ALERT_WARNING)
				!= G_ALERTALTERNATE) {
			return -2;
		}
	}

	if ((mbox_fp = g_fopen(mbox, "wb")) == NULL) {
		FILE_OP_ERROR(mbox, "g_fopen");
		alertpanel_error(_("Could not create mbox file:\n%s\n"), mbox);
		return -1;
	}

	statusbar_print_all(_("Exporting to mbox..."));
	for (cur = mlist; cur != NULL; cur = cur->next) {
		int len;
		gchar buft[BUFFSIZE];
		msginfo = (MsgInfo *)cur->data;

		msg_fp = procmsg_open_message(msginfo, TRUE);
		if (!msg_fp) {
			continue;
		}

		strncpy2(buf,
			 msginfo->from ? msginfo->from :
			 cur_account && cur_account->address ?
			 cur_account->address : "unknown",
			 sizeof(buf));
		extract_address(buf);

		if (fprintf(mbox_fp, "From %s %s",
			buf, ctime_r(&msginfo->date_t, buft)) < 0) {
			err = -1;
			fclose(msg_fp);
			goto out;
		}

		buf[0] = '\0';

		/* write email to mboxrc */
		while (fgets(buf, sizeof(buf), msg_fp) != NULL) {
			/* quote any From, >From, >>From, etc., according to mbox format specs */
			int offset;

			offset = 0;
			/* detect leading '>' char(s) */
			while (buf[offset] == '>') {
				offset++;
			}
			if (!strncmp(buf+offset, "From ", 5)) {
				if (fputc('>', mbox_fp) == EOF) {
					err = -1;
					fclose(msg_fp);
					goto out;
				}
			}
			if (fputs(buf, mbox_fp) == EOF) {
				err = -1;
				fclose(msg_fp);
				goto out;
			}
		}

		/* force last line to end w/ a newline */
		len = strlen(buf);
		if (len > 0) {
			len--;
			if ((buf[len] != '\n') && (buf[len] != '\r')) {
				if (fputc('\n', mbox_fp) == EOF) {
					err = -1;
					fclose(msg_fp);
					goto out;
				}
			}
		}

		/* add a trailing empty line */
		if (fputc('\n', mbox_fp) == EOF) {
			err = -1;
			fclose(msg_fp);
			goto out;
		}

		fclose(msg_fp);
		statusbar_progress_all(msgs++,total, 500);
		if (msgs%500 == 0)
			GTK_EVENTS_FLUSH();
	}

out:
	statusbar_progress_all(0,0,0);
	statusbar_pop_all();

	fclose(mbox_fp);

	return err;
}

/* read all messages in SRC, and store them into one MBOX file. */
/* return values: -2 skipped, -1 error, 0 OK */
gint export_to_mbox(FolderItem *src, const gchar *mbox)
{
	GSList *mlist;
	gint ret;

	cm_return_val_if_fail(src != NULL, -1);
	cm_return_val_if_fail(src->folder != NULL, -1);
	cm_return_val_if_fail(mbox != NULL, -1);

	debug_print("Exporting messages from %s into %s...\n",
		    src->path, mbox);

	mlist = folder_item_get_msg_list(src);

	folder_item_update_freeze();
	ret = export_list_to_mbox(mlist, mbox);
	folder_item_update_thaw();

	procmsg_msg_list_free(mlist);

	return ret;
}
