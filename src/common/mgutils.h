/*
 * Claws Mail -- a GTK based, lightweight, and fast e-mail client
 * Copyright (C) 2001-2012 Match Grun and the Claws Mail team
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

#ifndef __MGUTILS_H__
#define __MGUTILS_H__

#define MGU_SUCCESS        0
#define MGU_NO_FILE        -2
#define MGU_OPEN_FILE      -3
#define MGU_BAD_FORMAT     -7
#define MGU_ERROR_WRITE    -15
#define MGU_OPEN_DIRECTORY -16
#define MGU_NO_PATH        -17

char *mgu_replace_string	(char *old, const char *new);

#endif /* __MGUTILS_H__ */
