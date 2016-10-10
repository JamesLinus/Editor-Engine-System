/*
 * Copyright 2003, 2006, 2007 Free Software Foundation, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk-pixbuf/gdk-pixbuf-loader.h>

#include <libguile.h>

#include "guile-gtk-compat.h"
#include "guile-gtk.h"
#include "gdk-pixbuf-support.h"


/* sgtk_scm2cvec puts a zero element (ie. NULL) at the end of "data".  This
   interp is just to discard the len parameter.  Maybe could do that with a
   change to build-guile-gtk-2.0.  */
GdkPixbuf *
gdk_pixbuf_new_from_xpm_data_interp (int len, char **data)
{
  return gdk_pixbuf_new_from_xpm_data ((const char **) data);
}

/* This is explicit code to allow for '\0' bytes in buf string.
   Using `raw-data-r' would be a possibility, but it'd be incompatible to
   drop the `count' parameter from the scheme level func.  (Even if most of
   the time a whole string would be typical.)  */
gboolean
gdk_pixbuf_loader_write_interp (GdkPixbufLoader *loader, SCM buf, SCM count,
                                GError **gerr)
{
  static const char func_name[] = "gdk-pixbuf-loader-write";

  gsize c_count;
  size_t c_buflen;
  char *c_buf;
  gboolean ret;

  c_count = scm_to_uint (count);
  c_buf = scm_to_locale_stringn (buf, &c_buflen);
  if (c_count > c_buflen)
    scm_out_of_range (func_name, count);
  ret = gdk_pixbuf_loader_write (loader, (guchar*) c_buf, c_count, gerr);
  free (c_buf);
  return ret;
}
