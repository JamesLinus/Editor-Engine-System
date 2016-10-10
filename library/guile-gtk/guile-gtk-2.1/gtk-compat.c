/*
 * Copyright (C) 1997, 1998, 2000, 2001, 2002, 2003, 2004, 2006, 2007 Free Software
 * Foundation, Inc.
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

#include <config.h>
#include <gtk/gtk.h>
#include "guile-gtk.h"

/* This whole file is rated XXX. */

gchar*
gtk_label_get_interp (GtkLabel *label)
{
  gchar *str;
  gtk_label_get (label, &str);
  return str;
}

/* cheap cop-out. */

void
gtk_menu_popup_interp (GtkMenu *menu,
		       GtkWidget *parent_menu_shell,
		       GtkWidget *parent_menu_item,
		       gint button,
		       guint32 activate_time)
{
  gtk_menu_popup (menu, parent_menu_shell, parent_menu_item,
		  NULL, NULL, button, activate_time);
}

GdkColor*
gdk_color_parse_interp (char *spec)
{
  GdkColor color;
  return gdk_color_parse (spec, &color) ? gdk_color_copy (&color) : NULL;
}

GdkColor*
gtk_style_get_white_interp (GtkStyle *style)
{
  return &style->white;
}

void
gtk_list_append_item (GtkList *list, GtkListItem *item)
{
  GList *items = g_list_alloc ();
  items->data = item;
  gtk_list_append_items (list, items);
}

void
gtk_list_prepend_item (GtkList *list, GtkListItem *item)
{
  GList *items = g_list_alloc ();
  items->data = item;
  gtk_list_prepend_items (list, items);
}

#ifndef HAVE_GTK_SIGNAL_SET_CLASS_FUNCTION_FULL
void
gtk_signal_set_class_function_full (GtkType            type,
				    const gchar       *signal,
				    GtkSignalFunc      func,
				    GtkCallbackMarshal marshal,
				    gpointer           data,
				    GtkDestroyNotify   destroy_func)
{
  g_warning("Your version of Gtk+ does not support"
	    " gtk_signal_set_class_function_full");
}
#endif

void 
gtk_color_selection_set_color_interp (GtkColorSelection *selection, GdkColor *color)
{
  gdouble vals[4];
  
  /* get current opacity into vals[3], when in use, so we leave it unchanged */
  gtk_color_selection_get_color (selection, vals);

  vals[0] = color->red / 65535.0; 
  vals[1] = color->green / 65535.0; 
  vals[2] = color->blue / 65535.0; 

  gtk_color_selection_set_color (selection, vals);
}


GdkColor *
gtk_color_selection_get_color_interp (GtkColorSelection *selection)
{
  gdouble vals[4];
  GdkColor dummy, *color;

  gtk_color_selection_get_color (selection, vals);

  color = gdk_color_copy (&dummy);

  /* Since this color is not part of a colormap, the pixel value is
     pointless */
  color->pixel = 0; 
  color->red = (gushort) (65535.0 * vals[0]); 
  color->green = (gushort) (65535.0 * vals[1]); 
  color->blue = (gushort) (65535.0 * vals[2]); 

  return color;
}
