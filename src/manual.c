/*
 * Claws Mail -- a GTK based, lightweight, and fast e-mail client
 * Copyright (C) 1999-2012 Hiroyuki Yamamoto and the Claws Mail team
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

#include "manual.h"
#include "utils.h"

void manual_open(ManualType type, gchar *url_anchor)
{
	if (type == MANUAL_FAQ_CLAWS) {
		open_uri("https://www.claws-mail.org/faq/index.php", "xdg-open '%s'");
		return;
	}
	open_uri("https://www.claws-mail.org/documentation.php", "xdg-open '%s'");
}

void manual_open_with_anchor_cb(GtkWidget *widget, gchar *url_anchor)
{
	manual_open(MANUAL_MANUAL_CLAWS, url_anchor);
}
