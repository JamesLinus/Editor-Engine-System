/*
 * Copyright (C) 1997, 1998, 1999, 2002, 2003, 2004, 2005, 2006, 2007 Free
 * Software Foundation, Inc.
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

#define GDK_ENABLE_BROKEN 1  /* ask for gdk_image_new_bitmap */

#include <config.h>
#include <libguile.h>
#include <guile-gtk.h>

#include <glib-object.h>
#include <gdk/gdkx.h> /* for GDK_WINDOW_XID */

#include "guile-gtk-compat.h"
#include "gtk-threads.h"
#include <string.h>



/* There's no g_object_get_type() (like say gtk_widget_get_type()) in gtk,
   so create this little helper for use in the define-object of GObject in
   gdk-2.0.defs.  */
GType
sgtk_g_object_get_type (void)
{
  return G_TYPE_OBJECT;
}

SCM
gdk_selection_property_get_interp (GdkWindow *requestor,
				   GdkAtom *prop_type, int *prop_format)
{
  guchar *data;
  int length;

  /* When the requestor window has been destroyed, it looks like
     gdk_selection_property_get doesn't store anything in its return
     locations.  Initialize here to be sure.  */
  data = NULL;
  *prop_type = GDK_NONE;
  *prop_format = 0;

  length = gdk_selection_property_get (requestor, &data, prop_type,
				       prop_format);
  /* Note: While we are returning a string, the data may actually be
   * anything and contain zeros. */
  if (data)
    return scm_take_locale_stringn ((char *) data, length);
  else
    return SCM_BOOL_F;
}

#define FUNCNAME "gdk-property-change"
#define BAD_FORMAT(format) \
  SCM_ASSERT (FALSE, scm_from_int(format), SCM_ARG6, FUNCNAME)
#define BAD_DATA(data) SCM_ASSERT (FALSE, data, SCM_ARG6, FUNCNAME)

static void
gdk_property_change_list (GdkWindow *window, GdkAtom property, GdkAtom type,
			  gint format, GdkPropMode mode, SCM data)
{
  guchar *duchar,*dc;
  gushort *dushort,*ds;
  gulong *dulong,*dl;
  int nelements = scm_ilength (data);
  if (nelements < 1) BAD_DATA (data);
  switch (format) {
  case 8:
    dc = duchar = g_new (guchar, nelements);
    for (; scm_is_pair (data); data = SCM_CDR (data))
      *dc++ = scm_num2long (SCM_CAR (data), SCM_ARG6, FUNCNAME);
    if (!scm_is_null (data)) BAD_DATA (data);
    gdk_property_change (window, property, type, format, mode,
			 duchar, nelements);
    g_free (duchar);
    break;
  case 16:
    ds = dushort = g_new (gushort, nelements);
    for (; scm_is_pair (data); data = SCM_CDR (data))
      *ds++ = scm_num2long (SCM_CAR (data), SCM_ARG6, FUNCNAME);
    if (!scm_is_null (data)) BAD_DATA (data);
    gdk_property_change (window, property, type, format, mode,
			 (guchar *) dushort, nelements);
    g_free (dushort);
    break;
  case 32:
    dl = dulong = g_new (gulong, nelements);
    for (; scm_is_pair (data); data = SCM_CDR (data))
      *dl++ = scm_num2long (SCM_CAR (data), SCM_ARG6, FUNCNAME);
    if (!scm_is_null (data)) BAD_DATA (data);
    gdk_property_change (window, property, type, format, mode,
			 (guchar *) dulong, nelements);
    g_free (dulong);
    break;
  default:
    BAD_FORMAT (format);
  }
}

static void
gdk_property_change_vector (GdkWindow *window, GdkAtom property, GdkAtom type,
			    gint format, GdkPropMode mode, SCM data)
{
  void *c_data;
  size_t len_data;
  SCM keep_data;
  int i;
  int nelements;

  if (! (format == 8 || format == 16 || format == 32))
    BAD_FORMAT (format);

  nelements = scm_c_vector_length (data);
  len_data = nelements * (format / 8);
  c_data = scm_malloc (len_data);
  keep_data = sgtk_make_cblk (c_data, len_data);

  switch (format) {
  case 8:
    {
      guchar *p = (guchar *) c_data;
      for (i = 0; i < nelements; i++)
        p[i] = scm_to_uint8 (scm_c_vector_ref (data, i));
    }
    break;
  case 16:
    {
      gushort *p = (gushort *) c_data;
      for (i = 0; i < nelements; i++)
        p[i] = scm_to_uint16 (scm_c_vector_ref (data, i));
    }
    break;
  default: /* 32 */
    {
      gulong *p = (gulong *) c_data;
      for (i = 0; i < nelements; i++)
        p[i] = scm_to_uint32 (scm_c_vector_ref (data, i));
    }
    break;
  }

  gdk_property_change (window, property, type, format, mode,
                       c_data, nelements);
  scm_remember_upto_here_1 (keep_data);
}

void
gdk_property_change_interp (GdkWindow *window, GdkAtom property, GdkAtom type,
			    gint format, GdkPropMode mode, SCM data)
{
  if (scm_is_null (data))
    gdk_property_change (window, property, type, format, mode, NULL, 0);
  else if (scm_is_string (data))
    {
      char *c_data;
      size_t c_len;
      if (format != 8) BAD_FORMAT (format);
      c_data = scm_to_locale_stringn (data, &c_len);
      gdk_property_change (window, property, type, format, mode,
                           (guchar *) c_data, c_len);
      free (c_data);
    }
  else if (scm_is_pair (data))
    gdk_property_change_list (window, property, type, format, mode, data);
  else if (scm_is_vector (data))
    gdk_property_change_vector (window, property, type, format, mode, data);
  else BAD_DATA (data);
}

#undef BAD_FORMAT
#undef BAD_DATA
#undef FUNCNAME


SCM_KEYWORD (kw_background,         "background");
SCM_KEYWORD (kw_cap_style,          "cap-style");
SCM_KEYWORD (kw_clip_mask,          "clip-mask");
SCM_KEYWORD (kw_clip_x_origin,      "clip-x-origin");
SCM_KEYWORD (kw_clip_y_origin,      "clip-y-origin");
SCM_KEYWORD (kw_fill,               "fill");
SCM_KEYWORD (kw_font,               "font");
SCM_KEYWORD (kw_foreground,         "foreground");
SCM_KEYWORD (kw_function,           "function");
SCM_KEYWORD (kw_graphics_exposures, "graphics-exposures");
SCM_KEYWORD (kw_join_style,         "join-style");
SCM_KEYWORD (kw_line_style,         "line-style");
SCM_KEYWORD (kw_line_width,         "line-width");
SCM_KEYWORD (kw_stipple,            "stipple");
SCM_KEYWORD (kw_subwindow_mode,     "subwindow-mode");
SCM_KEYWORD (kw_tile,               "tile");
SCM_KEYWORD (kw_ts_x_origin,        "ts-x-origin");
SCM_KEYWORD (kw_ts_y_origin,        "ts-y-origin");


/* This is a separate function so it can be shared in the future by
   gtk-gc-get.  */
GdkGCValuesMask
sgtk_gdk_gc_values_fill (char *func_name, int argnum,
                         GdkGCValues *values, SCM rest)
{
  GdkGCValuesMask mask;
  SCM key, val;

  mask = 0;
  for (;;)
    {
      if (! scm_is_pair (rest))
        break;
      key = SCM_CAR (rest);
      rest = SCM_CDR (rest);

      if (! scm_is_pair (rest))
	scm_misc_error (func_name, "missing argument to keyword ~A",
			scm_list_1 (key));
      argnum += 2;
      val = SCM_CAR (rest);
      rest = SCM_CDR (rest);

      if (scm_is_eq (key, kw_background))
	{
          val = sgtk_color_conversion (val);
          SCM_ASSERT (sgtk_valid_boxed (val, &sgtk_gdk_color_info),
                      val, argnum, func_name);
          values->background = * (GdkColor *) sgtk_scm2boxed (val);
	  mask |= GDK_GC_BACKGROUND;
	}
      else if (scm_is_eq (key, kw_cap_style))
	{
          values->cap_style = sgtk_scm2enum (val, &sgtk_gdk_cap_style_info,
                                             argnum, func_name);
	  mask |= GDK_GC_CAP_STYLE;
	}
      else if (scm_is_eq (key, kw_clip_mask))
	{
          /* this is a bitmap, but the code and docs give it as GdkPixmap */
          SCM_ASSERT (sgtk_is_a_gtkobj (GDK_TYPE_PIXMAP, val),
                      val, argnum, func_name);
          values->clip_mask = (GdkPixmap *) sgtk_get_gtkobj (val);
	  mask |= GDK_GC_CLIP_MASK;
	}
      else if (scm_is_eq (key, kw_clip_x_origin))
	{
	  values->clip_x_origin = scm_num2int (val, argnum, func_name);
	  mask |= GDK_GC_CLIP_X_ORIGIN;
	}
      else if (scm_is_eq (key, kw_clip_y_origin))
	{
	  values->clip_y_origin = scm_num2int (val, argnum, func_name);
	  mask |= GDK_GC_CLIP_Y_ORIGIN;
	}
      else if (scm_is_eq (key, kw_fill))
	{
          values->fill = sgtk_scm2enum (val, &sgtk_gdk_fill_info,
                                        argnum, func_name);
	  mask |= GDK_GC_FILL;
	}
      else if (scm_is_eq (key, kw_font))
	{
          val = sgtk_font_conversion (val);
          SCM_ASSERT (sgtk_valid_boxed (val, &sgtk_gdk_font_info),
                      val, argnum, func_name);
          values->font = (GdkFont *) sgtk_scm2boxed (val);
	  mask |= GDK_GC_FONT;
	}
      else if (scm_is_eq (key, kw_foreground))
	{
          val = sgtk_color_conversion (val);
          SCM_ASSERT (sgtk_valid_boxed (val, &sgtk_gdk_color_info),
                      val, argnum, func_name);
          values->foreground = * (GdkColor *) sgtk_scm2boxed (val);
	  mask |= GDK_GC_FOREGROUND;
	}
      else if (scm_is_eq (key, kw_function))
	{
          values->function = sgtk_scm2enum (val, &sgtk_gdk_function_info,
                                            argnum, func_name);
	  mask |= GDK_GC_FUNCTION;
	}
      else if (scm_is_eq (key, kw_graphics_exposures))
	{
          values->graphics_exposures = ! SCM_FALSEP (val);
	  mask |= GDK_GC_EXPOSURES;
	}
      else if (scm_is_eq (key, kw_join_style))
	{
          values->join_style = sgtk_scm2enum (val, &sgtk_gdk_join_style_info,
                                              argnum, func_name);
	  mask |= GDK_GC_JOIN_STYLE;
	}
      else if (scm_is_eq (key, kw_line_style))
	{
          values->line_style = sgtk_scm2enum (val, &sgtk_gdk_line_style_info,
                                              argnum, func_name);
	  mask |= GDK_GC_LINE_STYLE;
	}
      else if (scm_is_eq (key, kw_line_width))
	{
	  values->line_width = scm_num2int (val, argnum, func_name);
	  mask |= GDK_GC_LINE_WIDTH;
	}
      else if (scm_is_eq (key, kw_stipple))
	{
          SCM_ASSERT (sgtk_is_a_gtkobj (GDK_TYPE_PIXMAP, val),
                      val, argnum, func_name);
          values->stipple = (GdkPixmap *) sgtk_get_gtkobj (val);
	  mask |= GDK_GC_STIPPLE;
	}
      else if (scm_is_eq (key, kw_subwindow_mode))
	{
          values->subwindow_mode
            = sgtk_scm2enum (val, &sgtk_gdk_subwindow_mode_info,
                             argnum, func_name);
	  mask |= GDK_GC_SUBWINDOW;
	}
      else if (scm_is_eq (key, kw_tile))
	{
          SCM_ASSERT (sgtk_is_a_gtkobj (GDK_TYPE_PIXMAP, val),
                      val, argnum, func_name);
          values->tile = (GdkPixmap *) sgtk_get_gtkobj (val);
	  mask |= GDK_GC_TILE;
	}
      else if (scm_is_eq (key, kw_ts_x_origin))
	{
	  values->ts_x_origin = scm_num2int (val, argnum, func_name);
	  mask |= GDK_GC_TS_X_ORIGIN;
	}
      else if (scm_is_eq (key, kw_ts_y_origin))
	{
	  values->ts_y_origin = scm_num2int (val, argnum, func_name);
	  mask |= GDK_GC_TS_Y_ORIGIN;
	}
      else
	scm_misc_error (func_name, "unknown keyword ~A", scm_list_1 (key));
    }

  return mask;
}

GdkGC *
gdk_gc_new_with_values_interp (GdkWindow *window, SCM rest)
{
  GdkGCValues      values;
  GdkGCValuesMask  mask;
  GdkGC            *gc;
  
  mask = sgtk_gdk_gc_values_fill ("gdk-gc-new-with-values", SCM_ARG2,
                                  &values, rest);
  gc = gdk_gc_new_with_values (window, &values, mask);
  scm_remember_upto_here_1 (rest);
  return gc;
}


SCM_KEYWORD (kw_colormap,          "colormap");
SCM_KEYWORD (kw_cursor,            "cursor");
SCM_KEYWORD (kw_override_redirect, "override-redirect");
SCM_KEYWORD (kw_title,             "title");
SCM_KEYWORD (kw_visual,            "visual");
SCM_KEYWORD (kw_wmclass,           "wmclass");
SCM_KEYWORD (kw_x,                 "x");
SCM_KEYWORD (kw_y,                 "y");

/* The Gtk documentation doesn't seem to say whether NULL is allowed in the
   various fields of GdkWindowAttr.  The code looks like it's probably fine
   for the cursor, but not for the visual.  Let's insist on actual objects
   and strings for now.  */

GdkWindow *
gdk_window_new_interp (GdkWindow *parent, int width, int height,
		       GdkEventMask event_mask, GdkWindowClass window_class,
		       GdkWindowType window_type, SCM rest)
#define FUNC_NAME "gdk-window-new"
{
  GdkWindowAttr attr;
  gint mask;
  GdkWindow *w;
  SCM key, val;
  unsigned long argnum;
  SCM keep_lst = SCM_EOL;

  attr.event_mask = event_mask;
  attr.width = width;
  attr.height = height;
  attr.wclass = window_class;
  attr.window_type = window_type;

  mask = 0;
  argnum = 6;
  for (;;)
    {
      if (! scm_is_pair (rest))
        break;
      key = SCM_CAR (rest);
      rest = SCM_CDR (rest);

      if (! scm_is_pair (rest))
	scm_misc_error (FUNC_NAME, "missing argument to keyword ~A",
			scm_list_1 (key));
      argnum += 2;
      val = SCM_CAR (rest);
      rest = SCM_CDR (rest);

      if (scm_is_eq (key, kw_colormap))
	{
          SCM_ASSERT (sgtk_is_a_gtkobj (GDK_TYPE_COLORMAP, val),
                      val, argnum, FUNC_NAME);
          attr.colormap = (GdkColormap*) sgtk_get_gtkobj (val);
	  mask |= GDK_WA_COLORMAP;
	}
      else if (scm_is_eq (key, kw_cursor))
	{
          SCM_ASSERT (sgtk_valid_boxed (val, &sgtk_gdk_cursor_info),
                      val, argnum, FUNC_NAME);
          attr.cursor = (GdkCursor*) sgtk_scm2boxed (val);
	  mask |= GDK_WA_CURSOR;
	}
      else if (scm_is_eq (key, kw_override_redirect))
	{
          attr.override_redirect = ! SCM_FALSEP (val);
	  mask |= GDK_WA_NOREDIR;
	}
      else if (scm_is_eq (key, kw_title))
	{
          SCM cstr = sgtk_to_cstr (val);
          keep_lst = scm_cons (cstr, keep_lst);
	  attr.title = sgtk_cstr2ptr (cstr, argnum, FUNC_NAME);
	  mask |= GDK_WA_TITLE;
	}
      else if (scm_is_eq (key, kw_visual))
	{
          SCM_ASSERT (sgtk_is_a_gtkobj (GDK_TYPE_VISUAL, val),
                      val, argnum, FUNC_NAME);
          attr.visual = (GdkVisual*) sgtk_get_gtkobj (val);
	  mask |= GDK_WA_VISUAL;
	}
      else if (scm_is_eq (key, kw_wmclass))
	{
          SCM  cstr;

          cstr = sgtk_to_cstr (val);
          keep_lst = scm_cons (cstr, keep_lst);
          attr.wmclass_name = sgtk_cstr2ptr (cstr, argnum, FUNC_NAME);

          if (! scm_is_pair (rest))
            scm_misc_error (FUNC_NAME, "missing second argument to keyword ~A",
                            scm_list_1 (key));
          val = SCM_CAR (rest);
          rest = SCM_CDR (rest);
          argnum++;

          cstr = sgtk_to_cstr (val);
          keep_lst = scm_cons (cstr, keep_lst);
          attr.wmclass_class = sgtk_cstr2ptr (cstr, argnum, FUNC_NAME);
	  mask |= GDK_WA_WMCLASS;
	}
      else if (scm_is_eq (key, kw_x))
	{
	  attr.x = scm_num2short (val, argnum, FUNC_NAME);
	  mask |= GDK_WA_X;
	}
      else if (scm_is_eq (key, kw_y))
	{
	  attr.y = scm_num2short (val, argnum, FUNC_NAME);
	  mask |= GDK_WA_Y;
	}
      else
	scm_misc_error (FUNC_NAME, "unknown keyword ~A", scm_list_1 (key));
    }

  w = gdk_window_new (parent, &attr, mask);
  scm_remember_upto_here_1 (rest);
  scm_remember_upto_here_1 (keep_lst);
  return w;
}
#undef FUNC_NAME

void
gdk_window_destroy_interp (GdkWindow *window)
{
  /* gdk_window_destroy will unref the window, but we don't want that */
  gdk_window_ref (window);
  gdk_window_destroy (window);
}

/* Same as in gtk-glue.c, could share with that file if it exported this. */
static SCM
_sgtk_helper_toscm_copy_GdkWindow (void *mem)
{
  return sgtk_wrap_gtkobj ((GObject *) *(GdkWindow**) mem);
}

/* The list returned by gdk_window_get_children must be freed by the caller.
   Could do this sort of thing with a .defs file option, but easy enough to
   have explicit code while there's only a few such.  */
SCM
gdk_window_get_children_interp (GdkWindow *window)
{
  GList* children;
  SCM ret;
  children = gdk_window_get_children (window);
  ret = sgtk_list2scm (children, _sgtk_helper_toscm_copy_GdkWindow);
  g_list_free (children);
  return ret;
}


/* Event destructuring */

GdkEventType
gdk_event_type (GdkEvent *event)
{
  return event->any.type;
}

GdkWindow *
gdk_event_window (GdkEvent *event)
{
  return event->any.window;
}

gboolean
gdk_event_send_event (GdkEvent *event)
{
  return event->any.send_event;
}

GdkRectangle
gdk_event_area (GdkEvent *event)
{
  switch (event->any.type)
    {
    case GDK_EXPOSE:
      return event->expose.area;
    default:
      {
	GdkRectangle r = { 0, 0, 0, 0 };
	return r;
      }
    }
}

gint
gdk_event_count (GdkEvent *event)
{
  switch (event->type) {
  case GDK_EXPOSE:
    return event->expose.count;
  default:
    return 0;
  }
}

GdkVisibilityState
gdk_event_visibility_state (GdkEvent *event)
{
  switch (event->any.type)
    {
    case GDK_VISIBILITY_NOTIFY:
      return event->visibility.state;
    default:
      return GDK_VISIBILITY_UNOBSCURED; // XXX
    }
}

guint32
gdk_event_time (GdkEvent *event)
{
  switch (event->any.type)
    {
    case GDK_MOTION_NOTIFY:
      return event->motion.time;
    case GDK_BUTTON_PRESS:
    case GDK_BUTTON_RELEASE:
    case GDK_2BUTTON_PRESS:
    case GDK_3BUTTON_PRESS:
      return event->button.time;
    case GDK_ENTER_NOTIFY:
    case GDK_LEAVE_NOTIFY:
      return event->crossing.time;
    case GDK_KEY_PRESS:
    case GDK_KEY_RELEASE:
      return event->key.time;
    case GDK_PROPERTY_NOTIFY:
      return event->property.time;
    case GDK_SELECTION_CLEAR:
    case GDK_SELECTION_REQUEST:
    case GDK_SELECTION_NOTIFY:
      return event->selection.time;
    case GDK_PROXIMITY_IN:
    case GDK_PROXIMITY_OUT:
      return event->proximity.time;
    case GDK_DRAG_ENTER:
    case GDK_DRAG_LEAVE:
    case GDK_DRAG_MOTION:
    case GDK_DRAG_STATUS:
    case GDK_DROP_START:
    case GDK_DROP_FINISHED:
      return event->dnd.time;
    default:
      return 0;
    }
}

gdouble
gdk_event_x (GdkEvent *event)
{
  switch (event->any.type)
    {
    case GDK_MOTION_NOTIFY:
      return event->motion.x;
    case GDK_BUTTON_PRESS:
    case GDK_BUTTON_RELEASE:
    case GDK_2BUTTON_PRESS:
    case GDK_3BUTTON_PRESS:
      return event->button.x;
    case GDK_ENTER_NOTIFY:
    case GDK_LEAVE_NOTIFY:
      return event->crossing.x;
    default:
      return 0;
    }
}

gdouble
gdk_event_y (GdkEvent *event)
{
  switch (event->any.type)
    {
    case GDK_MOTION_NOTIFY:
      return event->motion.y;
    case GDK_BUTTON_PRESS:
    case GDK_BUTTON_RELEASE:
    case GDK_2BUTTON_PRESS:
    case GDK_3BUTTON_PRESS:
      return event->button.y;
    case GDK_ENTER_NOTIFY:
    case GDK_LEAVE_NOTIFY:
      return event->crossing.y;
    default:
      return 0;
    }
}

gint
gdk_event_button (GdkEvent *event)
{
  switch (event->any.type)
    {
    case GDK_BUTTON_PRESS:
    case GDK_BUTTON_RELEASE:
    case GDK_2BUTTON_PRESS:
    case GDK_3BUTTON_PRESS:
      return event->button.button;
    default:
      return 0;
    }
}

guint
gdk_event_state (GdkEvent *event)
{
  switch (event->any.type)
    {
    case GDK_MOTION_NOTIFY:
      return event->motion.state;
    case GDK_BUTTON_PRESS:
    case GDK_BUTTON_RELEASE:
    case GDK_2BUTTON_PRESS:
    case GDK_3BUTTON_PRESS:
      return event->button.state;
    case GDK_ENTER_NOTIFY:
    case GDK_LEAVE_NOTIFY:
      return event->crossing.state;
    case GDK_KEY_PRESS:
    case GDK_KEY_RELEASE:
      return event->key.state;
    default:
      return 0;
    }
}

gboolean
gdk_event_is_hint (GdkEvent *event)
{
  switch (event->any.type)
    {
    case GDK_MOTION_NOTIFY:
      return event->motion.is_hint;
    default:
      return 0;
    }
}

GdkInputSource
gdk_event_source (GdkEvent *event)
{
  switch (event->any.type)
    {
    case GDK_MOTION_NOTIFY:
      return event->motion.device->source;
    case GDK_BUTTON_PRESS:
    case GDK_BUTTON_RELEASE:
    case GDK_2BUTTON_PRESS:
    case GDK_3BUTTON_PRESS:
      return event->button.device->source;
    case GDK_PROXIMITY_IN:
    case GDK_PROXIMITY_OUT:
      return event->proximity.device->source;
    default:
      return GDK_SOURCE_MOUSE; /* XXX */
    }
}

/* This can probably turn into gdk_event_device returning GdkDevice */
/* guint32 */
/* gdk_event_deviceid (GdkEvent *event) */
/* { */
/*   switch (event->any.type) */
/*     { */
/*     case GDK_MOTION_NOTIFY: */
/*       return event->motion.deviceid; */
/*     case GDK_BUTTON_PRESS: */
/*     case GDK_BUTTON_RELEASE: */
/*     case GDK_2BUTTON_PRESS: */
/*     case GDK_3BUTTON_PRESS: */
/*       return event->button.deviceid; */
/*     case GDK_PROXIMITY_IN: */
/*     case GDK_PROXIMITY_OUT: */
/*       return event->proximity.deviceid; */
/*     default: */
/*       return 0; */
/*     } */
/* } */

gdouble
gdk_event_x_root (GdkEvent *event)
{
  switch (event->any.type)
    {
    case GDK_MOTION_NOTIFY:
      return event->motion.x_root;
    case GDK_BUTTON_PRESS:
    case GDK_BUTTON_RELEASE:
    case GDK_2BUTTON_PRESS:
    case GDK_3BUTTON_PRESS:
      return event->button.x_root;
    case GDK_DRAG_ENTER:
    case GDK_DRAG_LEAVE:
    case GDK_DRAG_MOTION:
    case GDK_DRAG_STATUS:
    case GDK_DROP_START:
    case GDK_DROP_FINISHED:
      return event->dnd.x_root;
    default:
      return 0;
    }
}

gdouble
gdk_event_y_root (GdkEvent *event)
{
  switch (event->any.type)
    {
    case GDK_MOTION_NOTIFY:
      return event->motion.y_root;
    case GDK_BUTTON_PRESS:
    case GDK_BUTTON_RELEASE:
    case GDK_2BUTTON_PRESS:
    case GDK_3BUTTON_PRESS:
      return event->button.y_root;
    case GDK_DRAG_ENTER:
    case GDK_DRAG_LEAVE:
    case GDK_DRAG_MOTION:
    case GDK_DRAG_STATUS:
    case GDK_DROP_START:
    case GDK_DROP_FINISHED:
      return event->dnd.y_root;
    default:
      return 0;
    }
}

guint
gdk_event_keyval (GdkEvent *event)
{
  switch (event->any.type)
    {
    case GDK_KEY_PRESS:
    case GDK_KEY_RELEASE:
      return event->key.keyval;
    default:
      return 0;
    }
}

/* Normally key.string is NULL in GDK_KEY_RELEASE and non-NULL in
   GDK_KEY_PRESS, but there's no need to worry about that, just return
   string or #f according to what the field contains.  */
SCM
gdk_event_string (GdkEvent *event)
{
  switch (event->any.type) {
  case GDK_KEY_PRESS:
  case GDK_KEY_RELEASE:
    if (event->key.string != NULL)
      return scm_from_locale_stringn (event->key.string, event->key.length);
    break;
  default:
    break;
  }
  return SCM_BOOL_F;
}

GdkWindow *
gdk_event_subwindow (GdkEvent *event)
{
  switch (event->any.type)
    {
    case GDK_ENTER_NOTIFY:
    case GDK_LEAVE_NOTIFY:
      return event->crossing.subwindow;
    default:
      return 0;
    }
}

GdkCrossingMode
gdk_event_crossing_mode (GdkEvent *event)
{
  switch (event->type) {
  case GDK_ENTER_NOTIFY:
  case GDK_LEAVE_NOTIFY:
    return event->crossing.mode;
  default:
    return 0;
  }
}

GdkNotifyType
gdk_event_notify_detail (GdkEvent *event)
{
  switch (event->any.type)
    {
    case GDK_ENTER_NOTIFY:
    case GDK_LEAVE_NOTIFY:
      return event->crossing.detail;
    default:
      return 0;
    }
}

int
gdk_event_focus (GdkEvent *event)
{
  switch (event->type) {
  case GDK_ENTER_NOTIFY:
  case GDK_LEAVE_NOTIFY:
    return event->crossing.focus;
  default:
    return 0;
  }
}

gboolean
gdk_event_in (GdkEvent *event)
{
  switch (event->any.type)
    {
    case GDK_FOCUS_CHANGE:
      return event->focus_change.in;
    default:
      return 0;
    }
}

gint16
gdk_event_configure_x (GdkEvent *event)
{
  switch (event->any.type)
    {
    case GDK_CONFIGURE:
      return event->configure.x;
    default:
      return 0;
    }
}

gint16
gdk_event_configure_y (GdkEvent *event)
{
  switch (event->any.type)
    {
    case GDK_CONFIGURE:
      return event->configure.y;
    default:
      return 0;
    }
}

gint16
gdk_event_configure_width (GdkEvent *event)
{
  switch (event->any.type)
    {
    case GDK_CONFIGURE:
      return event->configure.width;
    default:
      return 0;
    }
}

gint16
gdk_event_configure_height (GdkEvent *event)
{
  switch (event->any.type)
    {
    case GDK_CONFIGURE:
      return event->configure.height;
    default:
      return 0;
    }
}

GdkAtom
gdk_event_atom (GdkEvent *event)
{
  switch (event->type) {
  case GDK_PROPERTY_NOTIFY:
    return event->property.atom;
  default:
    return 0;
  }
}

GdkAtom
gdk_event_selection (GdkEvent *event)
{
  switch (event->any.type)
    {
    case GDK_SELECTION_CLEAR:
    case GDK_SELECTION_REQUEST:
    case GDK_SELECTION_NOTIFY:
      return event->selection.selection;
    default:
      return 0;
    }
}

GdkAtom
gdk_event_target (GdkEvent *event)
{
  switch (event->any.type)
    {
    case GDK_SELECTION_CLEAR:
    case GDK_SELECTION_REQUEST:
    case GDK_SELECTION_NOTIFY:
      return event->selection.target;
    default:
      return 0;
    }
}

GdkAtom
gdk_event_property (GdkEvent *event)
{
  switch (event->any.type)
    {
    case GDK_SELECTION_CLEAR:
    case GDK_SELECTION_REQUEST:
    case GDK_SELECTION_NOTIFY:
      return event->selection.property;
    default:
      return 0;
    }
}

guint32
gdk_event_requestor (GdkEvent *event)
{
  switch (event->any.type)
    {
    case GDK_SELECTION_CLEAR:
    case GDK_SELECTION_REQUEST:
    case GDK_SELECTION_NOTIFY:
      return event->selection.requestor;
    default:
      return 0;
    }
}

GdkDragContext*
gdk_event_drag_context (GdkEvent *event)
{
  switch (event->any.type)
    {
    case GDK_DRAG_ENTER:
    case GDK_DRAG_LEAVE:
    case GDK_DRAG_MOTION:
    case GDK_DRAG_STATUS:
    case GDK_DROP_START:
    case GDK_DROP_FINISHED:
      return event->dnd.context;
    default:
      return NULL;
    }
}

GdkAtom
gdk_event_message_type (GdkEvent *event)
{
  switch (event->type) {
  case GDK_CLIENT_EVENT:
    return event->client.message_type;
  default:
    return 0;
  }
}

SCM
gdk_event_message (GdkEvent *event)
{
  SCM vector;
  int i;
  switch (event->type) {
  case GDK_CLIENT_EVENT:
    switch (event->client.data_format) {
    case 8:
      vector = scm_c_make_vector (20, SCM_BOOL_F);
      for (i = 0; i < 20; i++)
	SCM_SIMPLE_VECTOR_SET (vector, i, scm_from_long ((long) event->client.data.b[i]));
      break;
    case 16:
      vector = scm_c_make_vector (10, SCM_BOOL_F);
      for (i = 0; i < 10; i++)
	SCM_SIMPLE_VECTOR_SET (vector, i, scm_from_long ((long) event->client.data.s[i]));
      break;
    case 32:
    default:
      vector = scm_c_make_vector (5, SCM_BOOL_F);
      for (i = 0; i < 5; i++)
	SCM_SIMPLE_VECTOR_SET (vector, i, scm_from_long (event->client.data.l[i]));
    }
    return vector;
  default:
    return SCM_BOOL_F;
  }
}


/* Doesn't work. */
#if 0
void
gdk_add_client_message_filter_interp (GdkAtom message_type, SCM filter_proc)
{
  static SCM client_message_filters = SCM_EOL;
  SCM_ASSERT (SCM_NFALSEP (scm_procedure_p (filter_proc)), filter_proc,
	      SCM_ARG2, "gdk-add-client-message-filter");
  if (scm_is_null (client_message_filters)) {
    client_message_filters = scm_list_1 (filter_proc);
    scm_permanent_object (client_message_filters);
  }
  else SCM_SETCDR (client_message_filters,
		   scm_cons (filter_proc,
			     SCM_CDR (client_message_filters)));
}
#endif

/* A guile-gtk extension, retained for compatibility, but probably of
   limited use.  Not sure what happens to GDK_WINDOW_XID on a non-X11
   system.
   In Gtk 2.0 there can be multiple displays and multiple leaders on one
   display, so the idea here is just to give the default one.  */
guint32
gdk_get_leader_window_id (void)
{
  return GDK_WINDOW_XID
    (gdk_display_get_default_group (gdk_display_get_default()));
}

/* A guile-gtk extension.  Not sure what happens to GDK_WINDOW_XID on a
   non-X11 system.  */
guint32
gdk_window_get_id (GdkWindow *window)
{
  return GDK_WINDOW_XID (window);
}

SCM_KEYWORD (kw_aspect,      "aspect");
SCM_KEYWORD (kw_base_size,   "base-size");
SCM_KEYWORD (kw_max_size,    "max-size");
SCM_KEYWORD (kw_min_size,    "min-size");
SCM_KEYWORD (kw_resize_inc,  "resize-inc");
SCM_KEYWORD (kw_pos,         "pos");
SCM_KEYWORD (kw_user_pos,    "user-pos");
SCM_KEYWORD (kw_user_size,   "user-size");
SCM_KEYWORD (kw_win_gravity, "win-gravity");

/* Fill "geom" with keyword argument values taken "rest".  The return is a
   GdkWindowHints mask showing which fields of "geom" have been set (plus
   hint settings which are just flags, without data fields in GdkGeometry).

   "argnum" is always 2 in current uses, but gdk_window_constrain_size in
   gtk 2 will be different.  */

GdkWindowHints
sgtk_gdk_geometry_fill (const char *func_name, int argnum,
                        GdkGeometry *geom, SCM rest)
{
  GdkWindowHints mask;
  SCM key, val, val2;

  mask = 0;
  for (;;)
    {
      if (! scm_is_pair (rest))
        break;
      key = SCM_CAR (rest);
      rest = SCM_CDR (rest);
      argnum++;

      if (scm_is_eq (key, kw_pos))
	{
	  mask |= GDK_HINT_POS;
	}
      else if (scm_is_eq (key, kw_user_pos))
	{
	  mask |= GDK_HINT_USER_POS;
	}
      else if (scm_is_eq (key, kw_user_size))
	{
	  mask |= GDK_HINT_USER_SIZE;
	}
      else
        {
          if (! scm_is_pair (rest))
            scm_misc_error (func_name, "missing argument to keyword ~A",
                            scm_list_1 (key));
          val = SCM_CAR (rest);
          rest = SCM_CDR (rest);
          argnum++;

          if (scm_is_eq (key, kw_win_gravity))
            {
              geom->win_gravity
                = sgtk_scm2enum (val, &sgtk_gdk_gravity_info,
                                 argnum, func_name);
              mask |= GDK_HINT_WIN_GRAVITY;
            }
          else
            {
              if (! scm_is_pair (rest))
                scm_misc_error (func_name, "missing second argument to keyword ~A",
                                scm_list_1 (key));
              val2 = SCM_CAR (rest);
              rest = SCM_CDR (rest);
              argnum++;

              if (scm_is_eq (key, kw_aspect))
                {
                  geom->min_aspect = scm_num2double (val, argnum-1, func_name);
                  geom->max_aspect = scm_num2double (val2, argnum, func_name);
                  mask |= GDK_HINT_ASPECT;
                }
              else if (scm_is_eq (key, kw_base_size))
                {
                  geom->base_width  = scm_num2int (val, argnum-1, func_name);
                  geom->base_height = scm_num2int (val2, argnum, func_name);
                  mask |= GDK_HINT_BASE_SIZE;
                }
              else if (scm_is_eq (key, kw_max_size))
                {
                  geom->max_width  = scm_num2int (val, argnum-1, func_name);
                  geom->max_height = scm_num2int (val2, argnum, func_name);
                  mask |= GDK_HINT_MAX_SIZE;
                }
              else if (scm_is_eq (key, kw_min_size))
                {
                  geom->min_width  = scm_num2int (val, argnum-1, func_name);
                  geom->min_height = scm_num2int (val2, argnum, func_name);
                  mask |= GDK_HINT_MIN_SIZE;
                }
              else if (scm_is_eq (key, kw_resize_inc))
                {
                  geom->width_inc  = scm_num2int (val, argnum-1, func_name);
                  geom->height_inc = scm_num2int (val2, argnum, func_name);
                  mask |= GDK_HINT_RESIZE_INC;
                }
              else
                scm_misc_error (func_name, "unknown keyword ~A",
                                scm_list_1 (key));
            }
        }
    }

  return mask;
}

void
gdk_window_set_geometry_hints_interp (GdkWindow *window, SCM rest)
{
  GdkGeometry geom;
  GdkWindowHints flags;
  flags = sgtk_gdk_geometry_fill ("gdk-window-set-geometry-hints", SCM_ARG2,
                                  &geom, rest);
  gdk_window_set_geometry_hints (window, &geom, flags);
}

SCM
gdk_window_get_position_interp (GdkWindow *window)
{
  gint x, y;
  gdk_window_get_position (window, &x, &y);
  return scm_cons (scm_from_int (x), scm_from_int (y));
}

SCM
gdk_window_get_root_origin_interp (GdkWindow *window)
{
  gint x, y;
  gdk_window_get_root_origin (window, &x, &y);
  return scm_cons (scm_from_int (x), scm_from_int (y));
}

SCM
gdk_window_get_deskrelative_origin_interp (GdkWindow *window)
{
  gint x, y;
  gdk_window_get_deskrelative_origin (window, &x, &y);
  return scm_cons (scm_from_int (x), scm_from_int (y));
}


SCM
gdk_window_get_size_interp (GdkWindow *window)
{
  gint width;
  gint height;
  gdk_window_get_size (window, &width, &height);
  return scm_cons (scm_from_int (width), scm_from_int (height));
}

SCM
gdk_window_get_origin_interp (GdkWindow *window)
{
  gint x;
  gint y;
  gdk_window_get_origin (window, &x, &y);
  return scm_cons (scm_from_int (x), scm_from_int (y));
}

#define FUNCNAME "gdk-draw-text"
void
gdk_draw_text_interp (GdkDrawable *drawable, GdkFont *font, GdkGC *gc,
		      gint x, gint y, SCM text)
{
  char *c_text;
  size_t c_len;
  c_text = scm_to_locale_stringn (text, &c_len);
  gdk_draw_text (drawable, font, gc, x, y, c_text, c_len);
  free (c_text);
}
#undef FUNCNAME

#define FUNCNAME "gdk-text-extents"
void
gdk_text_extents_interp (GdkFont *font, SCM text,
		      gint *lbearing, gint *rbearing, gint *width,
		      gint *ascent, gint *descent)
{
  char *c_text;
  size_t c_len;
  c_text = scm_to_locale_stringn (text, &c_len);
  /* What about 16-bit fonts ?!?! See spec. */
  gdk_text_extents (font, c_text, c_len,
                    lbearing, rbearing, width, ascent, descent);
  free (c_text);
}
#undef FUNCNAME

#define FUNCNAME "gdk-text-width"
gint
gdk_text_width_interp (GdkFont *font, SCM text)
{
  char *c_text;
  size_t c_len;
  gint ret;
  c_text = scm_to_locale_stringn (text, &c_len);
  ret = gdk_text_width (font, c_text, c_len);
  free (c_text);
  return ret;
}
#undef FUNCNAME

#define FUNCNAME "gdk-text-measure"
gint
gdk_text_measure_interp (GdkFont *font, SCM text)
{
  char *c_text;
  size_t c_len;
  gint ret;
  c_text = scm_to_locale_stringn (text, &c_len);
  ret = gdk_text_measure (font, c_text, c_len);
  free (c_text);
  return ret;
}
#undef FUNCNAME

#define FUNCNAME "gdk-text-height"
gint
gdk_text_height_interp (GdkFont *font, SCM text)
{
  char *c_text;
  size_t c_len;
  gint ret;
  c_text = scm_to_locale_stringn (text, &c_len);
  ret = gdk_text_height (font, c_text, c_len);
  free (c_text);
  return ret;
}
#undef FUNCNAME

char *
gdk_wcstombs_interp (GdkWChar src[], int count)
{
  return gdk_wcstombs (src);
}

SCM
gdk_mbstowcs_interp (const char *src)
{
  SCM vector;
  gint rv,i;
  GdkWChar *space;
  gint length = strlen (src);
  space = g_new (GdkWChar, length + 1);
  rv = gdk_mbstowcs (space, src, length + 1);
  if (rv < 0) {
    free (space);
    return SCM_BOOL_F;
  }
  for (i = 0; i < length && space[i]; i++) ;
  vector = scm_c_make_vector (i, SCM_UNSPECIFIED);
  for (i = 0; i < length && space[i]; i++)
    SCM_SIMPLE_VECTOR_SET (vector, i, scm_from_long (space[i]));
  g_free (space);
  return vector;
}

SCM
gdk_gc_get_values_interp (GdkGC *gc)
{
  GdkGCValues values;
  gdk_gc_get_values (gc, &values);
  return scm_list_n (sgtk_boxed2scm (&values.foreground,
				     &sgtk_gdk_color_info, TRUE),
		     sgtk_boxed2scm (&values.background,
				     &sgtk_gdk_color_info, TRUE),
		     sgtk_boxed2scm (values.font, &sgtk_gdk_font_info, TRUE),
		     sgtk_enum2scm (values.function, &sgtk_gdk_function_info),
		     sgtk_enum2scm (values.fill, &sgtk_gdk_fill_info),
		     sgtk_wrap_gtkobj ((GObject *) values.tile),
		     sgtk_wrap_gtkobj ((GObject *) values.stipple),
		     sgtk_wrap_gtkobj ((GObject *) values.clip_mask),
		     sgtk_enum2scm (values.subwindow_mode,
				     &sgtk_gdk_subwindow_mode_info),
		     scm_from_long (values.ts_x_origin),
		     scm_from_long (values.ts_y_origin),
		     scm_from_long (values.clip_x_origin),
		     scm_from_long (values.clip_y_origin),
		     values.graphics_exposures ? SCM_BOOL_T : SCM_BOOL_F,
		     scm_from_long (values.line_width),
		     sgtk_enum2scm (values.line_style,
				    &sgtk_gdk_line_style_info),
		     sgtk_enum2scm (values.cap_style,
				    &sgtk_gdk_cap_style_info),
		     sgtk_enum2scm (values.join_style,
				    &sgtk_gdk_join_style_info),
		     SCM_UNDEFINED);
}

SCM
gdk_rectangle_intersect_interp (GdkRectangle *rect1, GdkRectangle *rect2)
{
  GdkRectangle rout;
  if (gdk_rectangle_intersect (rect1, rect2, &rout))
    return sgtk_rect2scm (rout);
  else return SCM_BOOL_F;
}

GdkRectangle
gdk_rectangle_union_interp (GdkRectangle *rect1, GdkRectangle *rect2)
{
  GdkRectangle rout;
  gdk_rectangle_union (rect1, rect2, &rout);
  return rout;
}

GdkRectangle
gdk_region_get_clipbox_interp (GdkRegion *region)
{
  GdkRectangle r;
  gdk_region_get_clipbox (region, &r);
  return r;
}

/* Bitmap data as row bits packed into bytes, and each row is padded out to
   a byte boundary.  */
static void
check_bitmap_data_size (const char *func, int count, gint width, gint height)
{
  gint bytes_per_row = (width + 7) / 8;
  gint want_count = bytes_per_row * height;
  if (count != want_count)
    scm_misc_error (func,
		    "bitmap data wrong size, should be ~a bytes",
		    scm_list_1 (scm_from_int (want_count)));
}

GdkImage *
gdk_image_new_bitmap_interp (GdkVisual *visual, guchar data[], int count,
			     gint width, gint height)
{
  static const char func_name[] = "gdk-image-new-bitmap";
  gpointer  m_data;

  check_bitmap_data_size ("gdk-bitmap-create-from-data", count, width, height);

  m_data = malloc (count);
  if (m_data == NULL)
    scm_memory_error (func_name);
  memcpy (m_data, data, count);
  return gdk_image_new_bitmap (visual, m_data, width, height);
}

GdkBitmap *
gdk_bitmap_create_from_data_interp (GdkWindow *window, guchar data[],
				    int count, gint width, gint height)
{
  check_bitmap_data_size ("gdk-bitmap-create-from-data", count, width, height);
  return gdk_bitmap_create_from_data (window, data, width, height);
}

GdkPixmap *
gdk_pixmap_create_from_data_interp (GdkWindow *window, guchar data[],
				    int count, gint width, gint height,
				    gint depth, GdkColor *fg, GdkColor *bg)
{
  if (count * 8 < width * height)
    scm_misc_error ("gdk-pixmap-create-from-data",
		    "source pixmap is too small",
		    SCM_EOL);
  return gdk_pixmap_create_from_data (window, data, width, height,
				      depth, fg, bg);
}

GdkPixmap *
gdk_pixmap_create_from_xpm_d_interp (GdkWindow *window,
				     GdkBitmap **mask,
				     GdkColor *transparent_color,
				     char *data[],int count)
{
  return gdk_pixmap_create_from_xpm_d (window, mask, transparent_color, data);
}

GdkPixmap *
gdk_pixmap_colormap_create_from_xpm_d_interp (GdkWindow *window,
					      GdkColormap *colormap,
					      GdkBitmap **mask,
					      GdkColor *transparent_color,
					      char *data[],int count)
{
  return gdk_pixmap_colormap_create_from_xpm_d (window, colormap, mask,
						transparent_color, data);
}

static void
rgb_init ()
{
  static int initialized = FALSE;
  if (initialized) return;
  gdk_rgb_init();
  initialized = TRUE;
}

void
gdk_draw_rgb_image_interp (GdkDrawable *drawable, GdkGC *gc,
			   gint x, gint y, gint w, gint h,
			   GdkRgbDither dith,
			   guchar rgb_buf[], int count,
			   gint rowstride)
{
  if (count < (h - 1) * rowstride + w * 3)
    scm_misc_error ("gdk-draw-rgb-image",
		    "source RGB image is too small",
		    SCM_EOL);
  rgb_init();
  gdk_draw_rgb_image (drawable, gc, x, y, w, h, dith, rgb_buf, rowstride);
}

void
gdk_draw_rgb_image_dithalign_interp (GdkDrawable *drawable, GdkGC *gc,
				     gint x, gint y, gint w, gint h,
				     GdkRgbDither dith,
				     guchar rgb_buf[], int count,
				     gint rowstride,
				     gint xdith, gint ydith)
{
  if (count < (h - 1) * rowstride + w * 3)
    scm_misc_error ("gdk-draw-rgb-image-dithalign",
		    "source RGB image is too small",
		    SCM_EOL);
  rgb_init();
  gdk_draw_rgb_image_dithalign (drawable, gc, x, y, w, h, dith, rgb_buf,
				rowstride, xdith, ydith);
}

void
gdk_draw_indexed_image_interp (GdkDrawable *drawable, GdkGC *gc,
			       gint x, gint y, gint w, gint h,
			       GdkRgbDither dith,
			       guchar index_buf[], int i_count,
			       gint rowstride,
			       GdkRgbCmap *cmap)
{
  if (i_count < (h - 1) * rowstride + w)
    scm_misc_error ("gdk-draw-indexed-image", "source image is too small",
		    SCM_EOL);
  rgb_init();
  gdk_draw_indexed_image (drawable, gc, x, y, w, h, dith, index_buf,
			  rowstride, cmap);
}

void
gdk_draw_gray_image_interp (GdkDrawable *drawable, GdkGC *gc,
			    gint x, gint y, gint w, gint h,
			    GdkRgbDither dith,
			    guchar gray_buf[], int count,
			    gint rowstride)
{
  if (count < (h - 1) * rowstride + w)
    scm_misc_error ("gdk-draw-gray-image", "source image is too small",
		    SCM_EOL);
  rgb_init();
  gdk_draw_gray_image (drawable, gc, x, y, w, h, dith, gray_buf, rowstride);
}

void
gdk_draw_rgb_32_image_interp (GdkDrawable *drawable, GdkGC *gc,
			      gint x, gint y, gint w, gint h,
			      GdkRgbDither dith,
			      guchar rgb_32_buf[], int count,
			      gint rowstride)
{
  if (count < (h - 1) * rowstride + w * 4)
    scm_misc_error ("gdk-draw-rgb-32-image", "source image is too small",
		    SCM_EOL);
  rgb_init();
  gdk_draw_rgb_32_image (drawable, gc, x, y, w, h, dith, rgb_32_buf,
			 rowstride);
}

/* This has no equivalent within Gtk as such.  There might be some scope to
   take parameters specifying r/g/b and/or pixel values, but let's start
   just with object creation.  */
GdkColor *
gdk_color_new (void)
{
  static const GdkColor color;
  return gdk_color_copy (&color);
}

GdkRgbCmap *
gdk_rgb_cmap_new_interp (guint32 colors[], int count)
{
  rgb_init();
  return gdk_rgb_cmap_new (colors, count);
}

void
gdk_rgb_gc_set_foreground_interp (GdkGC *gc, guint32 rgb)
{
  rgb_init();
  gdk_rgb_gc_set_foreground (gc, rgb);
}

void
gdk_rgb_gc_set_background_interp (GdkGC *gc, guint32 rgb)
{
  rgb_init();
  gdk_rgb_gc_set_background (gc, rgb);
}

void
gdk_rgb_set_install_interp (int install)
{
  rgb_init();
  gdk_rgb_set_install (install);
}

void
gdk_rgb_set_min_colors_interp (int min_colors)
{
  rgb_init();
  gdk_rgb_set_min_colors (min_colors);
}

GdkColormap *
gdk_rgb_get_cmap_interp (void)
{
  rgb_init();
  return gdk_rgb_get_cmap ();
}

GdkVisual *
gdk_rgb_get_visual_interp (void)
{
  rgb_init();
  return gdk_rgb_get_visual ();
}

gboolean
gdk_rgb_ditherable_interp (void)
{
  rgb_init();
  return gdk_rgb_ditherable ();
}

void
gdk_rgb_set_verbose_interp (gboolean verbose)
{
  rgb_init();
  gdk_rgb_set_verbose (verbose);
}

static void
sgtk_color_copy (SCM color, void *ptr)
{
  memcpy (ptr,sgtk_scm2boxed (color),sizeof (GdkColor));
}

/* Like sgtk_color_conversion, but just the parse, don't allocate the color
   anywhere.  */
SCM
sgtk_string_parse_to_color (SCM obj)
{
  if (scm_is_string (obj))
    {
      GdkColor c;
      char *c_str;
      gint ret;
      c_str = scm_to_locale_string (obj);
      ret = gdk_color_parse (c_str, &c);
      free (c_str);
      if (! ret)
        scm_misc_error ("string->color", "no such color: ~S",
                        scm_list_1 (obj));
      return sgtk_boxed2scm (&c, &sgtk_gdk_color_info, 1);
    }
  else
    return obj;
}

/* must be an actual GdkColor, #f or string not accepted */
static int
sgtk_helper_valid_GdkColor (SCM obj)
{
  return sgtk_valid_boxed (obj, &sgtk_gdk_color_info);
}

gint
gdk_colormap_alloc_colors_interp (GdkColormap *colormap, SCM colors,
				  int writable, int best_match,
                                  SCM *s_success_ptr)
{
  sgtk_cvec cvec;
  gint failures;
  gboolean *c_success;
  SCM s_success;
  int i;
  GdkColor *cv;

  /* in "colors" change strings to GdkColors */
  sgtk_composite_outconversion (colors, sgtk_string_parse_to_color);
  /* now must have GdkColors */
  SCM_ASSERT (sgtk_valid_composite (colors, sgtk_helper_valid_GdkColor),
              colors, SCM_ARG2, "gdk-colormap-alloc-colors");

  cvec = sgtk_scm2cvec (colors, sgtk_color_copy, sizeof (GdkColor));
  c_success = g_new (gboolean, cvec.count);
  cv = (GdkColor *) cvec.vec;
  failures = gdk_colormap_alloc_colors (colormap, cv, cvec.count,
                                        writable, best_match, c_success);

  /* success flags as list of bools */
  s_success = SCM_EOL;
  for (i = cvec.count-1; i >= 0; i--)
    s_success = scm_cons (scm_from_bool (c_success[i]), s_success);
  g_free (c_success);
  *s_success_ptr = s_success;

  /* put modified "cv" GdkColors back into the "colors" GdkColor objects */
  if (scm_is_vector (colors))
    {
      for (i = 0; i < cvec.count; i++)
        memcpy (sgtk_scm2boxed (scm_c_vector_ref (colors, i)),
                cv+i, sizeof (GdkColor));
    }
  else
    {
      SCM  lst;
      for (i = 0, lst = colors; i < cvec.count; i++, lst = SCM_CDR (lst))
        memcpy (sgtk_scm2boxed (SCM_CAR (lst)),
                cv+i, sizeof (GdkColor));
    }
  scm_remember_upto_here_1 (colors);
  free (cvec.vec);

  return failures;
}

/* It really doesn't make sense to give strings for GdkColor's here, since
   that would merely mean allocating a color then immediately freeing it,
   hence we demand actual GdkColor objects.  */
void
gdk_colormap_free_colors_interp (GdkColormap *colormap, SCM colors)
{
  sgtk_cvec cvec;
  SCM_ASSERT (sgtk_valid_composite (colors, sgtk_helper_valid_GdkColor),
              colors, SCM_ARG2, "gdk-colormap-free-colors");
  cvec = sgtk_scm2cvec (colors, sgtk_color_copy, sizeof (GdkColor));
  gdk_colormap_free_colors (colormap, (GdkColor *) cvec.vec, cvec.count);
}

GdkColor *
gdk_color_white_interp (GdkColormap *colormap)
{
  GdkColor color;
  return gdk_color_white (colormap, &color) ? gdk_color_copy (&color) : NULL;
}

GdkColor *
gdk_color_black_interp (GdkColormap *colormap)
{
  GdkColor color;
  return gdk_color_black (colormap, &color) ? gdk_color_copy (&color) : NULL;
}

SCM
gdk_query_depths_interp ()
{
  gint *depths,count,i;
  SCM list;
  gdk_query_depths (&depths,&count);
  for (i = count, list = SCM_EOL;
       i >= 0;
       list = scm_cons (scm_from_int (depths[i--]), list))
    ;
  return list;
}

SCM
gdk_query_visual_types_interp ()
{
  GdkVisualType *types;
  gint count,i;
  SCM list;
  gdk_query_visual_types (&types,&count);
  for (i = count, list = SCM_EOL;
       i >= 0;
       list = scm_cons (sgtk_enum2scm (types[i--], &sgtk_gdk_visual_type_info),
			list))
    ;
  return list;
}

static SCM
_sgdk_helper_toscm_copy_GdkVisual (void *mem)
{
  return sgtk_wrap_gtkobj ((GObject *) * (GdkVisual**) mem);
}

/* The list returned by gdk_list_visuals be freed by the caller.
   Could do this sort of thing with a .defs file option, but easy enough to
   have explicit code while there's only a few such.  */
SCM
gdk_list_visuals_interp (void)
{
  GList* visuals;
  SCM ret;
  visuals = gdk_list_visuals ();
  ret = sgtk_list2scm (visuals, _sgdk_helper_toscm_copy_GdkVisual);
  g_list_free (visuals);
  return ret;
}

GList*
gdk_drag_context_targets (GdkDragContext* context)
{ return context->targets; }

#define FUNCNAME "gdk-text-property-to-text-list"
SCM
gdk_text_property_to_text_list_interp (GdkAtom encoding, gint format, SCM text)
{
  int count;
  gchar **list;
  SCM scmlist;
  char *c_text;
  size_t c_len;

  c_text = scm_to_locale_stringn (text, &c_len);
  count = gdk_text_property_to_text_list (encoding, format,
                                          (guchar *) c_text, c_len, &list);
  free (c_text);
  if (!count--)
    return SCM_BOOL_F; /* 0 is not valid; failed */

  scmlist = scm_makfromstrs (count, list);
  gdk_free_text_list (list);
  return scmlist;
}
#undef FUNCNAME

gint
gdk_string_to_compound_text_interp (char *str, GdkAtom *encoding, gint *format,
                                    SCM *textp)
{
  guchar *ctext;
  gint ret, length;
  ret = gdk_string_to_compound_text (str, encoding, format, &ctext, &length);
  if (ctext != NULL)
    {
      *textp = scm_from_locale_stringn ((char *) ctext, length);
      gdk_free_compound_text (ctext);
    }
  else
    *textp = SCM_BOOL_F;
  return ret;
}

SCM
gdk_property_get_interp (GdkWindow *window, GdkAtom property, GdkAtom type,
			 gulong offset, gulong length, int pdelete,
			 GdkAtom *actual_property_type, gint *actual_format)
{
  guchar *data;
  gint actual_length;
  if (!gdk_property_get (window, property, type, offset, length, pdelete,
			 actual_property_type, actual_format,
			 &actual_length, &data))
    return SCM_BOOL_F;

  return scm_take_locale_stringn ((char *) data, actual_length);
}



void
sgtk_init_gdk_support ()
{
#ifndef SCM_MAGIC_SNARFER
#ifndef MKDEP
#include "gdk-support.x"
#endif /* MKDEP */
#endif /* SCM_MAGIC_SNARFER */

  scm_c_define ("pango-scale", scm_from_int (PANGO_SCALE));
  scm_c_define ("gdk-priority-events", scm_from_int (GDK_PRIORITY_EVENTS));
  scm_c_define ("gdk-major-version", scm_from_int (gtk_major_version));
  scm_c_define ("gdk-minor-version", scm_from_int (gtk_minor_version));
}
