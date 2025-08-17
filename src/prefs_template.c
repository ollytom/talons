/*
 * Claws Mail templates subsystem
 * Copyright (C) 2001 Alexander Barinov
 * Copyright (C) 2001-2025 The Claws Mail team
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

#include "config.h"

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "compose.h"
#include "quote_fmt.h"
#include "prefs_common.h"
#include "account.h"

gboolean prefs_template_string_is_valid(gchar *string, gint *line, gboolean escaped_string, gboolean email)
{
	gboolean result = TRUE;
	if (string && *string != '\0') {
		gchar *tmp = NULL;
		gchar *parsed_buf;
		MsgInfo dummyinfo;
		PrefsAccount *account = account_get_default();

		if (escaped_string) {
			tmp = malloc(strlen(string)+1);
			pref_get_unescaped_pref(tmp, string);
		} else {
			tmp = g_strdup(string);
		}
		memset(&dummyinfo, 0, sizeof(MsgInfo));
		/* init dummy fields, so we can test the result of the parse */
		dummyinfo.date="Sat, 30 May 2009 01:23:45 +0200";
		dummyinfo.fromname="John Doe";
		dummyinfo.from="John Doe <john@example.com>";
		dummyinfo.to="John Doe <john@example.com>";
		dummyinfo.cc="John Doe <john@example.com>";
		dummyinfo.msgid="<1234john@example.com>";
		dummyinfo.inreplyto="<1234john@example.com>";
		dummyinfo.newsgroups="alt.test";
		dummyinfo.subject="subject";


#ifdef USE_ENCHANT
		quote_fmt_init(&dummyinfo, NULL, NULL, TRUE, account, FALSE, NULL);
#else
		quote_fmt_init(&dummyinfo, NULL, NULL, TRUE, account, FALSE);
#endif
		quote_fmt_scan_string(tmp);
		quote_fmt_parse();
		g_free(tmp);
		parsed_buf = quote_fmt_get_buffer();
		if (!parsed_buf) {
			if (line)
				*line = quote_fmt_get_line();
			quote_fmtlex_destroy();
			return FALSE;
		}
		quote_fmt_reset_vartable();
		quote_fmtlex_destroy();
	}
	return result;
}
