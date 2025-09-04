/*
 * Claws Mail -- a GTK based, lightweight, and fast e-mail client
 * Copyright (C) 2002-2014 by the Claws Mail Team and Hiroyuki Yamamoto
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

#ifndef MATCHER_H
#define MATCHER_H

#include <sys/types.h>
#include <regex.h>
#include <glib.h>
#include "proctypes.h"
#include "matchertypes.h"

struct _MatcherProp {
	int matchtype;
	int criteria;
	gchar *header;
	gchar *expr;
	int value;
	int error;
	gboolean result;
	gboolean done;
	/* Allows recompiling expr each time */
	regex_t *preg;
	/* Allows casefolding expr each time */
	gchar *casefold_expr;
};

struct _MatcherList {
	GSList *matchers;
	gboolean bool_and;
};


/* map MATCHCRITERIA_ to yacc's MATCHER_ */
#define MC_(name) \
	MATCHCRITERIA_ ## name = MATCHER_ ## name

/* map MATCHTYPE_ to yacc's MATCHER_ */
#define MT_(name) \
	MATCHTYPE_ ## name = MATCHER_ ## name

/* map MATCHACTION_ to yacc's MATCHER_ */
#define MA_(name) \
	MATCHACTION_ ## name = MATCHER_ ## name

/* map MATCHBOOL_ to yacc's MATCHER_ */
#define MB_(name) \
	MATCHERBOOL_ ## name = MATCHER_ ## name


void matcher_init(void);
void matcher_done(void);

MatcherProp *matcherprop_new		(gint		 criteria,
					 const gchar	*header,
					 gint		 matchtype,
					 const gchar	*expr,
					 int	         value);
void matcherprop_free			(MatcherProp *prop);

MatcherProp *matcherprop_parse		(gchar	**str);

MatcherProp *matcherprop_copy		(const MatcherProp *src);

MatcherList * matcherlist_new		(GSList		*matchers,
					 gboolean	bool_and);
MatcherList * matcherlist_new_from_lines(gchar		*lines,
					 gboolean	bool_and,
					 gboolean	case_sensitive);
void matcherlist_free			(MatcherList	*cond);

MatcherList *matcherlist_parse		(gchar		**str);

gboolean matcherlist_match		(MatcherList	*cond, 
					 MsgInfo	*info);

gint matcher_parse_keyword		(gchar		**str);
gint matcher_parse_number		(gchar		**str);
gboolean matcher_parse_boolean_op	(gchar		**str);
gchar *matcher_parse_regexp		(gchar		**str);
gchar *matcher_parse_str		(gchar		**str);
gchar *matcherprop_to_string		(MatcherProp	*matcher);
gchar *matcherlist_to_string		(const MatcherList	*matchers);
gchar *matching_build_command		(const gchar	*cmd, 
					 MsgInfo	*info);

void prefs_matcher_read_config		(void);
void prefs_matcher_write_config		(void);

gchar * matcher_quote_str(const gchar * src);

#endif
