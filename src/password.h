/*
 * Claws Mail -- a GTK based, lightweight, and fast e-mail client
 * Copyright (C) 2016-2023 The Claws Mail team
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

#ifndef __PASSWORD_H
#define __PASSWORD_H

#include <glib.h>

gchar *password_encrypt_gnutls(const gchar *password,
		const gchar *encryption_passphrase);
gchar *password_decrypt_gnutls(const gchar *password,
		const gchar *decryption_passphrase);
#define password_encrypt_real(n, m) password_encrypt_gnutls(n, m)
#define password_decrypt_real(n, m) password_decrypt_gnutls(n, m)

/* Wrapper function that will apply best encryption available,
 * and return a string ready to be saved as-is in preferences. */
gchar *password_encrypt(const gchar *password,
		const gchar *encryption_passphrase);

/* This is a wrapper function that looks at the whole string from
 * prefs (e.g. including the leading '!' for old implementation),
 * and tries to do the smart thing. */
gchar *password_decrypt(const gchar *password,
		const gchar *decryption_passphrase);

#endif /* __PASSWORD_H */
