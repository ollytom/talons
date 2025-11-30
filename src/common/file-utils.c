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

#include <glib.h>

#include <sys/wait.h>

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include "defs.h"
#include "codeconv.h"
#include "utils.h"
#include "file-utils.h"

gint file_strip_crs(const gchar *file)
{
	FILE *fp = NULL, *outfp = NULL;
	gchar buf[4096];
	gchar *out = get_tmp_file();
	if (file == NULL)
		goto freeout;

	fp = g_fopen(file, "rb");
	if (!fp)
		goto freeout;

	outfp = g_fopen(out, "wb");
	if (!outfp) {
		fclose(fp);
		goto freeout;
	}

	while (fgets(buf, sizeof (buf), fp) != NULL) {
		strcrchomp(buf);
		if (fputs(buf, outfp) == EOF) {
			fclose(fp);
			fclose(outfp);
			goto unlinkout;
		}
	}

	fclose(fp);
	if (fclose(outfp) == EOF) {
		goto unlinkout;
	}
	if (rename(out, file) < 0)
		goto unlinkout;

	g_free(out);
	return 0;
unlinkout:
	unlink(out);
freeout:
	g_free(out);
	return -1;
}

/*
 * Append src file body to the tail of dest file.
 * Now keep_backup has no effects.
 */
gint append_file(const gchar *src, const gchar *dest, gboolean keep_backup)
{
	FILE *src_fp, *dest_fp;
	gint n_read;
	gchar buf[BUFSIZ];

	gboolean err = FALSE;

	if ((src_fp = g_fopen(src, "rb")) == NULL) {
		FILE_OP_ERROR(src, "g_fopen");
		return -1;
	}

	if ((dest_fp = g_fopen(dest, "ab")) == NULL) {
		FILE_OP_ERROR(dest, "g_fopen");
		fclose(src_fp);
		return -1;
	}

	while ((n_read = fread(buf, sizeof(gchar), sizeof(buf), src_fp)) > 0) {
		if (n_read < sizeof(buf) && ferror(src_fp))
			break;
		if (fwrite(buf, 1, n_read, dest_fp) < n_read) {
			g_warning("writing to %s failed", dest);
			fclose(dest_fp);
			fclose(src_fp);
			unlink(dest);
			return -1;
		}
	}

	if (ferror(src_fp)) {
		FILE_OP_ERROR(src, "fread");
		err = TRUE;
	}
	fclose(src_fp);
	if (fclose(dest_fp) == EOF) {
		FILE_OP_ERROR(dest, "fclose");
		err = TRUE;
	}

	if (err) {
		unlink(dest);
		return -1;
	}

	return 0;
}

gint copy_file(const gchar *src, const gchar *dest, gboolean keep_backup)
{
	FILE *src_fp, *dest_fp;
	gint n_read;
	gchar buf[BUFSIZ];
	gchar *dest_bak = NULL;
	gboolean err = FALSE;

	if ((src_fp = g_fopen(src, "rb")) == NULL) {
		FILE_OP_ERROR(src, "g_fopen");
		return -1;
	}
	if (is_file_exist(dest)) {
		dest_bak = g_strconcat(dest, ".bak", NULL);
		if (rename(dest, dest_bak) < 0) {
			warn("rename %s to %s", dest, dest_bak);
			fclose(src_fp);
			g_free(dest_bak);
			return -1;
		}
	}

	if ((dest_fp = g_fopen(dest, "wb")) == NULL) {
		FILE_OP_ERROR(dest, "g_fopen");
		fclose(src_fp);
		if (dest_bak) {
			if (rename(dest_bak, dest) < 0)
				FILE_OP_ERROR(dest_bak, "rename");
			g_free(dest_bak);
		}
		return -1;
	}

	while ((n_read = fread(buf, sizeof(gchar), sizeof(buf), src_fp)) > 0) {
		if (n_read < sizeof(buf) && ferror(src_fp))
			break;
		if (fwrite(buf, 1, n_read, dest_fp) < n_read) {
			g_warning("writing to %s failed", dest);
			fclose(dest_fp);
			fclose(src_fp);
			if (unlink(dest) < 0)
                                FILE_OP_ERROR(dest, "unlink");
			if (dest_bak) {
				if (rename(dest_bak, dest) < 0)
					FILE_OP_ERROR(dest_bak, "rename");
				g_free(dest_bak);
			}
			return -1;
		}
	}

	if (ferror(src_fp)) {
		FILE_OP_ERROR(src, "fread");
		err = TRUE;
	}
	fclose(src_fp);
	if (fclose(dest_fp) == EOF) {
		FILE_OP_ERROR(dest, "fclose");
		err = TRUE;
	}

	if (err) {
		if (unlink(dest) < 0)
                        FILE_OP_ERROR(dest, "unlink");
		if (dest_bak) {
			if (rename(dest_bak, dest) < 0)
				FILE_OP_ERROR(dest_bak, "rename");
			g_free(dest_bak);
		}
		return -1;
	}

	if (keep_backup == FALSE && dest_bak)
		if (unlink(dest_bak) < 0)
                        FILE_OP_ERROR(dest_bak, "unlink");

	g_free(dest_bak);

	return 0;
}

gint copy_file_part_to_fp(FILE *fp, off_t offset, size_t length, FILE *dest_fp)
{
	gint n_read;
	gint bytes_left, to_read;
	gchar buf[BUFSIZ];

	if (fseek(fp, offset, SEEK_SET) < 0) {
		perror("fseek");
		return -1;
	}

	bytes_left = length;
	to_read = MIN(bytes_left, sizeof(buf));

	while ((n_read = fread(buf, sizeof(gchar), to_read, fp)) > 0) {
		if (n_read < to_read && ferror(fp))
			break;
		if (fwrite(buf, 1, n_read, dest_fp) < n_read) {
			return -1;
		}
		bytes_left -= n_read;
		if (bytes_left == 0)
			break;
		to_read = MIN(bytes_left, sizeof(buf));
	}

	if (ferror(fp)) {
		perror("fread");
		return -1;
	}

	return 0;
}

gint copy_file_part(FILE *fp, off_t offset, size_t length, const gchar *dest)
{
	FILE *dest_fp;
	gboolean err = FALSE;

	if ((dest_fp = g_fopen(dest, "wb")) == NULL) {
		FILE_OP_ERROR(dest, "g_fopen");
		return -1;
	}

	if (copy_file_part_to_fp(fp, offset, length, dest_fp) < 0)
		err = TRUE;

	if (fclose(dest_fp) == EOF) {
		FILE_OP_ERROR(dest, "fclose");
		err = TRUE;
	}

	if (err) {
		g_warning("writing to %s failed", dest);
		unlink(dest);
		return -1;
	}

	return 0;
}

gint canonicalize_file(const gchar *src, const gchar *dest)
{
	FILE *src_fp, *dest_fp;
	gchar buf[BUFFSIZE];
	gint len;
	gboolean err = FALSE;
	gboolean last_linebreak = FALSE;

	if (src == NULL || dest == NULL)
		return -1;

	if ((src_fp = g_fopen(src, "rb")) == NULL) {
		FILE_OP_ERROR(src, "g_fopen");
		return -1;
	}

	if ((dest_fp = g_fopen(dest, "wb")) == NULL) {
		FILE_OP_ERROR(dest, "g_fopen");
		fclose(src_fp);
		return -1;
	}

	while (fgets(buf, sizeof(buf), src_fp) != NULL) {
		gint r = 0;

		len = strlen(buf);
		if (len == 0) break;
		last_linebreak = FALSE;

		if (buf[len - 1] != '\n') {
			last_linebreak = TRUE;
			r = fputs(buf, dest_fp);
		} else if (len > 1 && buf[len - 1] == '\n' && buf[len - 2] == '\r') {
			r = fputs(buf, dest_fp);
		} else {
			if (len > 1) {
				r = fwrite(buf, 1, len - 1, dest_fp);
				if (r != (len -1))
					r = EOF;
			}
			if (r != EOF)
				r = fputs("\r\n", dest_fp);
		}

		if (r == EOF) {
			g_warning("writing to %s failed", dest);
			fclose(dest_fp);
			fclose(src_fp);
			unlink(dest);
			return -1;
		}
	}

	if (last_linebreak == TRUE) {
		if (fputs("\r\n", dest_fp) == EOF)
			err = TRUE;
	}

	if (ferror(src_fp)) {
		FILE_OP_ERROR(src, "fgets");
		err = TRUE;
	}
	fclose(src_fp);
	if (fclose(dest_fp) == EOF) {
		FILE_OP_ERROR(dest, "fclose");
		err = TRUE;
	}

	if (err) {
		unlink(dest);
		return -1;
	}

	return 0;
}

gint str_write_to_file(const gchar *str, char *file)
{
	if (strlen(str) == 0)
		return 0;

	FILE *fp = fopen(file, "wb");
	if (fp == NULL) {
		warn("open %s", file);
		return -1;
	}

	size_t len = strlen(str);
	size_t n = fwrite(str, 1, strlen(str), fp);
	if (n != len) {
		warn("short write to %s: expected %d bytes but wrote %d", file, len, n);
		fclose(fp);
		remove(file);
		return -1;
	}

	if (fclose(fp) == EOF) {
		warn("close %s", file);
		remove(file);
		return -1;
	}
	return 0;
}

static gchar *file_read_stream_to_str_full(FILE *fp, gboolean recode)
{
	GByteArray *array;
	guchar buf[BUFSIZ];
	gint n_read;
	gchar *str;

	cm_return_val_if_fail(fp != NULL, NULL);

	array = g_byte_array_new();

	while ((n_read = fread(buf, sizeof(gchar), sizeof(buf), fp)) > 0) {
		if (n_read < sizeof(buf) && ferror(fp))
			break;
		g_byte_array_append(array, buf, n_read);
	}

	if (ferror(fp)) {
		FILE_OP_ERROR("file stream", "fread");
		g_byte_array_free(array, TRUE);
		return NULL;
	}

	buf[0] = '\0';
	g_byte_array_append(array, buf, 1);
	str = (gchar *)array->data;
	g_byte_array_free(array, FALSE);

	if (recode && !g_utf8_validate(str, -1, NULL)) {
		const gchar *src_codeset, *dest_codeset;
		gchar *tmp = NULL;
		src_codeset = conv_get_locale_charset_str();
		dest_codeset = CS_UTF_8;
		tmp = conv_codeset_strdup(str, src_codeset, dest_codeset);
		g_free(str);
		str = tmp;
	}

	return str;
}

static gchar *file_read_to_str_full(const gchar *file, gboolean recode)
{
	FILE *fp;
	gchar *str;
	GStatBuf s;
	gint fd, err;
	struct timeval timeout = {1, 0};
	fd_set fds;
	int fflags = 0;

	cm_return_val_if_fail(file != NULL, NULL);

	if (g_stat(file, &s) != 0) {
		FILE_OP_ERROR(file, "stat");
		return NULL;
	}
	if (S_ISDIR(s.st_mode)) {
		g_warning("%s: is a directory", file);
		return NULL;
	}

	/* test whether the file is readable without blocking */
	fd = g_open(file, O_RDONLY | O_NONBLOCK, 0);
	if (fd == -1) {
		FILE_OP_ERROR(file, "open");
		return NULL;
	}

	FD_ZERO(&fds);
	FD_SET(fd, &fds);

	/* allow for one second */
	err = select(fd+1, &fds, NULL, NULL, &timeout);
	if (err <= 0 || !FD_ISSET(fd, &fds)) {
		if (err < 0) {
			FILE_OP_ERROR(file, "select");
		} else {
			g_warning("%s: doesn't seem readable", file);
		}
		close(fd);
		return NULL;
	}

	/* Now clear O_NONBLOCK */
	if ((fflags = fcntl(fd, F_GETFL)) < 0) {
		FILE_OP_ERROR(file, "fcntl (F_GETFL)");
		close(fd);
		return NULL;
	}
	if (fcntl(fd, F_SETFL, (fflags & ~O_NONBLOCK)) < 0) {
		FILE_OP_ERROR(file, "fcntl (F_SETFL)");
		close(fd);
		return NULL;
	}

	/* get the FILE pointer */
	fp = fdopen(fd, "rb");

	if (fp == NULL) {
		FILE_OP_ERROR(file, "fdopen");
		close(fd); /* if fp isn't NULL, we'll use fclose instead! */
		return NULL;
	}

	str = file_read_stream_to_str_full(fp, recode);

	fclose(fp);

	return str;
}

gchar *file_read_to_str(const gchar *file)
{
	return file_read_to_str_full(file, TRUE);
}
gchar *file_read_stream_to_str(FILE *fp)
{
	return file_read_stream_to_str_full(fp, TRUE);
}

gint copy_dir(const gchar *src, const gchar *dst)
{
	GDir *dir;
	const gchar *name;

	if ((dir = g_dir_open(src, 0, NULL)) == NULL) {
		warn("open %s", src);
		return -1;
	}

	if (mkdir(dst, 0700) < 0) {
		g_dir_close(dir);
		return -1;
	}

	while ((name = g_dir_read_name(dir)) != NULL) {
		gchar *old_file, *new_file;
		gint r = 0;
		old_file = g_strconcat(src, G_DIR_SEPARATOR_S, name, NULL);
		new_file = g_strconcat(dst, G_DIR_SEPARATOR_S, name, NULL);
		debug_print("copying: %s -> %s\n", old_file, new_file);
		if (g_file_test(old_file, G_FILE_TEST_IS_REGULAR)) {
			r = copy_file(old_file, new_file, TRUE);
		}
		else if (g_file_test(old_file, G_FILE_TEST_IS_SYMLINK)) {
			GError *error = NULL;
			gchar *target = g_file_read_link(old_file, &error);
			if (error) {
				g_warning("couldn't read link: %s", error->message);
				g_error_free(error);
			}
			if (target) {
				r = symlink(target, new_file);
				g_free(target);
			}
		}
		else if (g_file_test(old_file, G_FILE_TEST_IS_DIR)) {
			r = copy_dir(old_file, new_file);
		}
		g_free(old_file);
		g_free(new_file);
		if (r < 0) {
			g_dir_close(dir);
			return r;
		}
	}
	g_dir_close(dir);
	return 0;
}

FILE *my_tmpfile(void)
{
	const gchar suffix[] = ".XXXXXX";
	const gchar *tmpdir;
	guint tmplen;
	const gchar *progname;
	guint proglen;
	gchar *fname;
	gint fd;
	FILE *fp;
	gchar buf[2]="\0";

	tmpdir = get_tmp_dir();
	tmplen = strlen(tmpdir);
	progname = g_get_prgname();
	if (progname == NULL)
		progname = "claws-mail";
	proglen = strlen(progname);
	Xalloca(fname, tmplen + 1 + proglen + sizeof(suffix),
		return tmpfile());

	memcpy(fname, tmpdir, tmplen);
	fname[tmplen] = G_DIR_SEPARATOR;
	memcpy(fname + tmplen + 1, progname, proglen);
	memcpy(fname + tmplen + 1 + proglen, suffix, sizeof(suffix));

	fd = g_mkstemp(fname);
	if (fd < 0)
		return tmpfile();

	unlink(fname);

	/* verify that we can write in the file after unlinking */
	if (write(fd, buf, 1) < 0) {
		close(fd);
		return tmpfile();
	}

	fp = fdopen(fd, "w+b");
	if (!fp)
		close(fd);
	else {
		rewind(fp);
		return fp;
	}

	return tmpfile();
}

FILE *get_tmpfile_in_dir(const gchar *dir, gchar **filename)
{
	int fd;
	*filename = g_strdup_printf("%s%cclaws.XXXXXX", dir, G_DIR_SEPARATOR);
	fd = g_mkstemp(*filename);
	if (fd < 0)
		return NULL;
	return fdopen(fd, "w+");
}

FILE *str_open_as_stream(const gchar *str)
{
	FILE *fp;
	size_t len;

	cm_return_val_if_fail(str != NULL, NULL);

	fp = my_tmpfile();
	if (!fp) {
		FILE_OP_ERROR("str_open_as_stream", "my_tmpfile");
		return NULL;
	}

	len = strlen(str);
	if (len == 0) return fp;

	if (fwrite(str, 1, len, fp) != len) {
		FILE_OP_ERROR("str_open_as_stream", "fwrite");
		fclose(fp);
		return NULL;
	}

	rewind(fp);
	return fp;
}

gint prefs_chmod_mode(gchar *chmod_pref)
{
	gint newmode = 0;
	gchar *tmp;

	if (chmod_pref) {
		newmode = strtol(chmod_pref, &tmp, 8);
		if (!(*(chmod_pref) && !(*tmp)))
			newmode = 0;
	}

	return newmode;
}
