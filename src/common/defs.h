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

#ifndef __DEFS_H__
#define __DEFS_H__

#define INBOX_DIR		"inbox"
#define OUTBOX_DIR		"sent"
#define QUEUE_DIR		"queue"
#define DRAFT_DIR		"draft"
#define TRASH_DIR		"trash"
#define RC_DIR			".claws-mail"
#define COMMON_RC		"clawsrc"
#define ACCOUNT_RC		"accountrc"
#define OAUTH2_RC		"oauth2rc"
#define CUSTOM_HEADER_RC	"customheaderrc"
#define DISPLAY_HEADER_RC	"dispheaderrc"
#define FOLDERITEM_RC           "folderitemrc"
#define MENU_RC			"menurc"
#define PASSWORD_STORE_RC			"passwordstorerc"
#define COMPOSE_SAVE_TO_HISTORY	"compose_save_to_history"
#define ADDRESS_BOOK		"addressbook.xml"
#define ADDRBOOK_DIR		"addrbook"
#define HOMEPAGE_URI		"https://www.claws-mail.org/"
#define USERS_ML_URI		"https://lists.claws-mail.org/cgi-bin/mailman/listinfo/users"
#define DONATE_URI		"https://www.claws-mail.org/donations.php"
#define RELEASE_NOTES_FILE	"RELEASE_NOTES"
#define FOLDER_LIST		"folderlist.xml"
#define CACHE_VERSION		24
#define MARK_VERSION		2

#define ACTIONS_RC		"actionsrc"
#define COMMAND_HISTORY	"command_history"
#define DEFAULT_SIGNATURE	".signature"

#define DEFAULT_INC_PATH	"/usr/bin/mh/inc"
#define DEFAULT_SENDMAIL_CMD	"/usr/sbin/sendmail -t -i"
#define DEFAULT_BROWSER_CMD	"firefox '%s'"
#define DEFAULT_EDITOR_CMD	"gedit '%s'"

#define BUFFSIZE			8192

#define RFC822_DATE_BUFFSIZE	128

#define BORDER_WIDTH			2
#define CTREE_INDENT			18
#define FOLDER_SPACING			4
#define COLOR_DIM			((double)35000 / 65535)
#define UI_REFRESH_INTERVAL		50000	/* usec */
#define PROGRESS_UPDATE_INTERVAL	200	/* msec */
#define SESSION_TIMEOUT_INTERVAL	60	/* sec */
#define MAX_HISTORY_SIZE		32
#define HSPACING_NARROW			4
#define VSPACING			10
#define VSPACING_NARROW			4
#define VSPACING_NARROW_2		2
#define VBOX_BORDER			8
#define DEFAULT_ENTRY_WIDTH		80

#define DEFAULT_PIXMAP_THEME	"INTERNAL_DEFAULT"

#define AVATAR_FACE	2

#endif /* __DEFS_H__ */
