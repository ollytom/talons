/*
 * Claws Mail -- a GTK based, lightweight, and fast e-mail client
 * Copyright (C) 1999-2024 the Claws Mail team and Colin Leroy
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

#ifndef __CLAWS_IO_H__
#define __CLAWS_IO_H__

#include <stdio.h>
#include <glib.h>

gint file_strip_crs		(const gchar	*file);
gint append_file		(const gchar	*src,
				 const gchar	*dest,
				 gboolean	 keep_backup);
gint copy_file			(const gchar	*src,
				 const gchar	*dest,
				 gboolean	 keep_backup);
gint move_file			(const gchar	*src,
				 const gchar	*dest,
				 gboolean	 overwrite);
gint copy_file_part_to_fp	(FILE		*fp,
				 off_t		 offset,
				 size_t		 length,
				 FILE		*dest_fp);
gint copy_file_part		(FILE		*fp,
				 off_t		 offset,
				 size_t		 length,
				 const gchar	*dest);
gint canonicalize_file		(const gchar	*src,
				 const gchar	*dest);
gint canonicalize_file_replace	(const gchar	*file);
gchar *file_read_to_str		(const gchar	*file);
gchar *file_read_to_str_no_recode(const gchar	*file);
gchar *file_read_stream_to_str	(FILE		*fp);
gchar *file_read_stream_to_str_no_recode(FILE	*fp);

gint rename_force		(const gchar	*oldpath,
				 const gchar	*newpath);
gint copy_dir			(const gchar	*src,
				 const gchar	*dest);
gint change_file_mode_rw	(FILE		*fp,
				 const gchar	*file);
FILE *my_tmpfile		(void);
FILE *get_tmpfile_in_dir	(const gchar 	*dir,
				 gchar	       **filename);
FILE *str_open_as_stream	(const gchar	*str);
gint str_write_to_file		(const gchar *str, char *file);

gint prefs_chmod_mode		(gchar *chmod_pref);


#endif
