/*
 * Copyright (C) 1997, 1998, 1999, 2002, 2003, 2004, 2006, 2007 Free Software
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
#include <libguile.h>
#include <guile-gtk.h>
#include "guile-gtk-compat.h"
#include "gtk-threads.h"
#include <string.h>

#define numberof(x)  (sizeof (x) / sizeof ((x)[0]))



GdkColor*
gtk_style_white (GtkStyle *style)
{
  return &style->white;
}

GdkColor*
gtk_style_black (GtkStyle *style)
{
  return &style->black;
}

GdkColor *
gtk_style_fg (GtkStyle *style, GtkStateType state)
{
  if (state < 0 || state >= numberof (style->fg))
    return NULL;
  else
    return &style->fg[state];
}

GdkColor *
gtk_style_bg (GtkStyle *style, GtkStateType state)
{
  if (state < 0 || state >= numberof (style->bg))
    return NULL;
  else
    return &style->bg[state];
}

GdkColor *
gtk_style_light (GtkStyle *style, GtkStateType state)
{
  if (state < 0 || state >= numberof (style->light))
    return NULL;
  else
    return &style->light[state];
}

GdkColor *
gtk_style_dark (GtkStyle *style, GtkStateType state)
{
  if (state < 0 || state >= numberof (style->dark))
    return NULL;
  else
    return &style->dark[state];
}

GdkColor *
gtk_style_mid (GtkStyle *style, GtkStateType state)
{
  if (state < 0 || state >= numberof (style->mid))
    return NULL;
  else
    return &style->mid[state];
}

GdkColor *
gtk_style_text (GtkStyle *style, GtkStateType state)
{
  if (state < 0 || state >= numberof (style->text))
    return NULL;
  else
    return &style->text[state];
}

GdkColor *
gtk_style_base (GtkStyle *style, GtkStateType state)
{
  if (state < 0 || state >= numberof (style->base))
    return NULL;
  else
    return &style->base[state];
}

GdkGC *
gtk_style_fg_gc (GtkStyle *style, GtkStateType state)
{
  if (style == NULL || state < 0 || state >= 5)
    return NULL;

  return style->fg_gc[state];
}

GdkGC *
gtk_style_bg_gc (GtkStyle *style, GtkStateType state)
{
  if (style == NULL || state < 0 || state >= 5)
    return NULL;

  return style->bg_gc[state];
}

GdkGC *
gtk_style_light_gc (GtkStyle *style, GtkStateType state)
{
  if (style == NULL || state < 0 || state >= 5)
    return NULL;

  return style->light_gc[state];
}

GdkGC *
gtk_style_dark_gc (GtkStyle *style, GtkStateType state)
{
  if (style == NULL || state < 0 || state >= 5)
    return NULL;

  return style->dark_gc[state];
}

GdkGC *
gtk_style_mid_gc (GtkStyle *style, GtkStateType state)
{
  if (style == NULL || state < 0 || state >= 5)
    return NULL;

  return style->mid_gc[state];
}

GdkGC *
gtk_style_text_gc (GtkStyle *style, GtkStateType state)
{
  if (style == NULL || state < 0 || state >= 5)
    return NULL;

  return style->text_gc[state];
}

GdkGC *
gtk_style_base_gc (GtkStyle *style, GtkStateType state)
{
  if (style == NULL || state < 0 || state >= 5)
    return NULL;

  return style->base_gc[state];
}

int
gtk_editable_insert_text_scm (GtkEditable *editable,
			      gchar       *text,
			      int          position)
{
  gtk_editable_insert_text (editable, text, strlen (text), &position);
  return position;
}

void* gtk_fake_copy (void* ptr)
{
  return ptr;
}

void *gtk_no_copy (void *ptr)
{
  return NULL;
}

void gtk_no_free (void *ptr)
{
}

SCM
gtk_selection_data_data (GtkSelectionData* data)
{
  if (data->length >= 0)
    return scm_from_locale_stringn (data->data, data->length);
  else
    return SCM_BOOL_F;
}


static GList *
sgtk_glist_malloc (size_t len)
{
  GList *lst = scm_malloc (len * sizeof (GList));
  size_t i;
  for (i = 0; i < len; i++)
    {
      lst[i].next = &lst[i+1];
      lst[i].prev = &lst[i-1];
    }
  lst[0].prev = NULL;
  lst[len-1].next = NULL;
  return lst;
}


/* Same as in gtk-glue.c, could share with that file if it exported this. */
static SCM
_sgtk_helper_toscm_copy_GtkWidget (void *mem)
{
  return sgtk_wrap_gtkobj ((GObject*)(*(GtkWidget**)mem));
}

/* The list returned by gtk_container_children must be freed by the caller.
   Could do this sort of thing with a .defs file option, but easy enough to
   have explicit code while there's only a few such.  */
SCM
gtk_container_children_interp (GtkContainer *container)
{
  GList* children;
  SCM ret;
  children = gtk_container_children (container);
  ret = sgtk_list2scm (children, _sgtk_helper_toscm_copy_GtkWidget);
  g_list_free (children);
  return ret;
}

/* Return a list of strings. */
SCM
gtk_rc_get_default_files_interp (void)
{
  return scm_makfromstrs (-1, gtk_rc_get_default_files ());
}

SCM
gtk_widget_size_request_interp (GtkWidget *widget)
{
  GtkRequisition req;
  gtk_widget_size_request (widget, &req);
  return scm_cons (scm_from_int (req.width), scm_from_int (req.height));
}

/* No support for formatting bits in the message currently, so use a "%s" to
   prevent any escapes in the string getting expanded.  */
GtkWidget *
gtk_message_dialog_new_interp (GtkWindow *parent,
                               GtkDialogFlags flags,
                               GtkMessageType type,
                               GtkButtonsType buttons,
                               const char *message)
{
  return gtk_message_dialog_new (parent, flags, type, buttons, "%s", message);
}

/* This has no equivalent within Gtk as such. */
GtkTextIter *
gtk_text_iter_new_interp (void)
{
  static const GtkTextIter iter;
  return gtk_text_iter_copy (&iter);
}

/* The following three hook funcs are very similar.  In theory they could
   return the previously set function if it was a scheme level procedure,
   but that seems like more trouble than it's worth.  */

void
gtk_about_dialog_set_email_hook_interp (SCM func)
{
  if (scm_is_false (func))
    gtk_about_dialog_set_email_hook (NULL, NULL, NULL);
  else
    gtk_about_dialog_set_email_hook
      (sgtk_about_dialog_activate_link_marshal,
       (gpointer) SCM_UNPACK (scm_gc_protect_object (func)),
       (GDestroyNotify) scm_gc_unprotect_object);
}

void
gtk_about_dialog_set_url_hook_interp (SCM func)
{
  if (scm_is_false (func))
    gtk_about_dialog_set_url_hook (NULL, NULL, NULL);
  else
    gtk_about_dialog_set_url_hook
      (sgtk_about_dialog_activate_link_marshal,
       (gpointer) SCM_UNPACK (scm_gc_protect_object (func)),
       (GDestroyNotify) scm_gc_unprotect_object);
}

/* This is new in Gtk 2.10, see GtkLinkButton in gtk-2.0.defs for comments
   on that. */
#if 0
/* GtkLinkButtonUriFunc and GtkAboutDialogActivateLinkFunc are the same
   except for a pointer type, so can use
   sgtk_about_dialog_activate_link_marshal here.  */
void
gtk_link_button_set_uri_hook_interp (SCM func)
{
  if (scm_is_false (func))
    gtk_link_button_set_uri_hook (NULL, NULL, NULL);
  else
    gtk_link_button_set_uri_hook
      ((GtkLinkButtonUriFunc) sgtk_about_dialog_activate_link_marshal,
       (gpointer) SCM_UNPACK (scm_gc_protect_object (func)),
       (GDestroyNotify) scm_gc_unprotect_object);
}
#endif



/* These SCM_PROCs are here to have them initialized in
   sgtk_init_gtk_support.  Having them in sgtk_init_substrate is wrong
   because then they are not guaranteed to end up in the (gtk gtk)
   module. */

SCM_PROC (s_gtk_callback_trampoline, "gtk-callback-trampoline", 0, 1, 0, sgtk_callback_trampoline);
SCM_PROC (s_gtk_standalone_p, "gtk-standalone?", 0, 0, 0, sgtk_standalone_p);

SCM sgtk_gtk_object_new (SCM, SCM);
SCM sgtk_gtk_object_set (SCM, SCM);
SCM sgtk_gtk_object_get (SCM, SCM);

SCM_PROC (s_gtk_object_new, "gtk-object-new", 1, 0, 1, sgtk_gtk_object_new);
SCM_PROC (s_gtk_object_set, "gtk-object-set", 1, 0, 1, sgtk_gtk_object_set);
SCM_PROC (s_gtk_object_get, "gtk-object-get", 2, 0, 0, sgtk_gtk_object_get);
SCM_PROC (s_gtk_widget_new, "gtk-widget-new", 1, 0, 1, sgtk_gtk_object_new);
SCM_PROC (s_gtk_widget_set, "gtk-widget-set", 1, 0, 1, sgtk_gtk_object_set);
SCM_PROC (s_gtk_widget_get, "gtk-widget-get", 2, 0, 0, sgtk_gtk_object_get);

SCM_PROC (s_gtk_threads_update, "gtk-threads-update", 0, 0, 0, sgtk_threads_update);

void
sgtk_init_gtk_support ()
{
#ifndef SCM_MAGIC_SNARFER
#ifndef MKDEP
#include "gtk-support.x"
#endif /* MKDEP */
#endif /* SCM_MAGIC_SNARFER */

  /* Use the library variables so as to indicate what we're running with,
     not GTK_MAJOR_VERSION etc which would be what we compiled against.  */
  scm_c_define ("gtk-major-version", scm_from_uint (gtk_major_version));
  scm_c_define ("gtk-minor-version", scm_from_uint (gtk_minor_version));
  scm_c_define ("gtk-micro-version", scm_from_uint (gtk_micro_version));
  scm_c_define ("gtk-binary-age",    scm_from_uint (gtk_binary_age));
  scm_c_define ("gtk-interface-age", scm_from_uint (gtk_interface_age));

  scm_c_define ("gtk-priority-default",  scm_from_int (GTK_PRIORITY_DEFAULT));
  scm_c_define ("gtk-priority-high",     scm_from_int (GTK_PRIORITY_HIGH));
  scm_c_define ("gtk-priority-internal", scm_from_int (GTK_PRIORITY_INTERNAL));
  scm_c_define ("gtk-priority-low",      scm_from_int (GTK_PRIORITY_LOW));
  scm_c_define ("gtk-priority-redraw",   scm_from_int (GTK_PRIORITY_REDRAW));
  scm_c_define ("gtk-priority-resize",   scm_from_int (GTK_PRIORITY_RESIZE));

  /* GtkResponseType enum is only used as integer values, there's no GType
     for it, so don't bother with a define-enum */
  scm_c_define ("gtk-response-none",   scm_from_int (GTK_RESPONSE_NONE));
  scm_c_define ("gtk-response-reject", scm_from_int (GTK_RESPONSE_REJECT));
  scm_c_define ("gtk-response-accept", scm_from_int (GTK_RESPONSE_ACCEPT));
  scm_c_define ("gtk-response-delete-event", scm_from_int (GTK_RESPONSE_DELETE_EVENT));
  scm_c_define ("gtk-response-ok",     scm_from_int (GTK_RESPONSE_OK));
  scm_c_define ("gtk-response-cancel", scm_from_int (GTK_RESPONSE_CANCEL));
  scm_c_define ("gtk-response-close",  scm_from_int (GTK_RESPONSE_CLOSE));
  scm_c_define ("gtk-response-yes",    scm_from_int (GTK_RESPONSE_YES));
  scm_c_define ("gtk-response-no",     scm_from_int (GTK_RESPONSE_NO));
  scm_c_define ("gtk-response-apply",  scm_from_int (GTK_RESPONSE_APPLY));
  scm_c_define ("gtk-response-help",   scm_from_int (GTK_RESPONSE_HELP));
}
