/*
 * Copyright (C) 2003, 2006 Free Software Foundation, Inc.
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

#include <glade/glade-xml.h>
#include <libguile.h>
#include <guile/gh.h>

#include "config.h"
#include "guile-gtk.h"

static void
try_merge (char* init, char* lib)
{
  scm_apply (scm_c_lookup ("merge-compiled-code"),
	     scm_list_2 (scm_makfrom0str (init),
			 scm_makfrom0str (lib)),
	     SCM_EOL);
}
  
#ifndef HAVE_GLADE_GNOME_INIT

void
glade_gnome_init (void)
{ try_merge ("glade_gnome_init", "libglade-gnome"); }
  
#endif

#ifndef HAVE_GLADE_BONOBO_INIT

void
glade_bonobo_init (void)
{ try_merge ("glade_bonobo_init", "libglade-bonobo"); }
  
#endif

#ifndef HAVE_GLADE_GNOME_DB_INIT

void
glade_gnome_db_init (void)
{ try_merge ("glade_gnome_db_init", "libglade-gnomedb"); }
  
#endif


static void
scm_glade_connect_dispatch (const gchar*	handler_name,
			    GObject*		object,
			    const char*		signal_name,
			    const char*		signal_data,
			    GObject*		connect_object,
			    gboolean		after,
			    gpointer		user_data)
{
  if (user_data)
    scm_apply (SCM_PACK (user_data),
	       scm_cons (scm_makfrom0str (handler_name),
			 scm_list_5 (
			   sgtk_wrap_gtkobj (object), 
			   scm_makfrom0str (signal_name), 
			   scm_makfrom0str (signal_data), 
			   sgtk_wrap_gtkobj (connect_object),
			   after ? SCM_BOOL_T : SCM_BOOL_F)),
	       SCM_EOL);
  else
    {
      SCM_STACKITEM  stack_item;
      SCM	     cb;
      GObject *c_object;
      sgtk_protshell *protshell;
      GClosure *closure;

      cb = scm_internal_cwdr ((scm_t_catch_body) gh_eval_str, (char*) handler_name,
			      scm_handle_by_message_noexit, "glade",
			      &stack_item);

      c_object = connect_object ? connect_object : object;
      protshell = sgtk_protect (SCM_BOOL_F, cb);
      closure = g_closure_new_simple (sizeof (GClosure), protshell);
      g_closure_set_marshal (closure, sgtk_closure_marshal);
      g_closure_add_finalize_notifier (closure, protshell, sgtk_closure_destroy);
      g_signal_connect_closure (c_object, signal_name, closure, after);
    }
}

void
glade_xml_signal_connect_full_hack (GladeXML*	self,
				    char*	handlername,
				    SCM		func)
{
  SCM_ASSERT (SCM_EQ_P (func, SCM_UNDEFINED) || SCM_EQ_P (func, SCM_BOOL_F) ||
	      SCM_NFALSEP (scm_procedure_p (func)), func, SCM_ARG3,
	      "glade-xml-signal-connect-full");

  glade_xml_signal_connect_full (self, handlername, scm_glade_connect_dispatch,
	 SCM_NFALSEP (scm_procedure_p (func)) ? ((gpointer) SCM_UNPACK (func)) : NULL);
}


void
glade_xml_signal_autoconnect_full_hack (GladeXML*	self,
					SCM		func)
{
  SCM_ASSERT (SCM_EQ_P (func, SCM_UNDEFINED) || SCM_EQ_P (func, SCM_BOOL_F) ||
	      SCM_NFALSEP (scm_procedure_p (func)), func, SCM_ARG2,
	      "glade-xml-signal-autoconnect-full");

  glade_xml_signal_autoconnect_full (self, scm_glade_connect_dispatch,
	 SCM_NFALSEP (scm_procedure_p (func)) ? (gpointer) SCM_UNPACK (func) : NULL);
}
