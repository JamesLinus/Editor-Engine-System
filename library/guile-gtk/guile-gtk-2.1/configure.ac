dnl Process this file with autoconf to produce configure.

define(GUILE_GTK_COPYRIGHT,[[

Copyright 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007
Free Software Foundation, Inc.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or (at
your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
]])

AC_PREREQ(2.61)
AC_COPYRIGHT(GUILE_GTK_COPYRIGHT)
AH_TOP(/*GUILE_GTK_COPYRIGHT*/)

AC_INIT(guile-gtk, 2.1, guile-gtk-general@gnu.org)
AM_INIT_AUTOMAKE([no-dependencies])

AM_CONFIG_HEADER(config.h)

AC_PROG_CC
AM_PROG_LIBTOOL
AC_PROG_CPP

PKG_CHECK_MODULES([GTK], [gtk+-2.0 glib-2.0 gthread-2.0])
CFLAGS="$CFLAGS $GTK_CFLAGS"
LIBS="$CFLAGS $GTK_LIBS"


#--- Test for GdkPixbuf ... -------------------------------------------
 
AC_ARG_WITH(gdk-pixbuf,
  AC_HELP_STRING([--with-gdk-pixbuf], [Compile (gtk-2.0 gdk-pixbuf) module]))

if test "x$with_gdk_pixbuf" != "xno" ; then
  PKG_CHECK_MODULES([GDK_PIXBUF], [gdk-pixbuf-2.0], have_gdk_pixbuf=yes)
fi

if test "x$have_gdk_pixbuf" = "xyes" ; then
  gdk_pixbuf_msg=Yes
else
  gdk_pixbuf_msg=No
fi

AM_CONDITIONAL(BUILD_GDK_PIXBUF, test "x$have_gdk_pixbuf" = "xyes")

#--- Test for libGlade ... --------------------------------------------

AC_ARG_WITH(glade,
  AC_HELP_STRING([--with-libglade], [Compile (gtk-2.0 glade) module]))

if test "x$with_glade" != "xno" ; then
  AC_ARG_WITH(libglade_modules, 
    AC_HELP_STRING([--with-libglade-modules=LIST], [Comma separated list of libglade modules]),
    [glade_modules=`echo $withval | sed -e 's/,/ /g' `])

  PKG_CHECK_MODULES([LIBGLADE], [libglade-2.0], have_glade=yes)
fi

if test "x$have_glade" = "xyes"
then
  saved_LIBS="$LIBS"
  LIBS="$LIBGLADE_LIBS $LIBS"
  AC_CHECK_FUNCS(glade_gnome_init glade_bonobo_init glade_gnome_db_init)
  LIBS="$saved_LIBS"
  glade_msg=Yes
else
  glade_msg=No
fi

AM_CONDITIONAL(BUILD_GLADE, test "x$have_glade" = "xyes")

#--- Test for GtkGLArea ... -------------------------------------------

AC_ARG_WITH(gtkgl,
  AC_HELP_STRING([--with-gtkgl], [Compile (gtk-2.0 gdk-gl) and gtk-gl-area modules]))

if test "x$with_gtkgl" != "xno" ; then
  PKG_CHECK_MODULES([GTKGL], [gtkgl-2.0], have_gtkgl=yes)
fi

if test "x$have_gtkgl" = "xyes" ; then
  gtk_gl_msg=Yes
else
  gtk_gl_msg=No
fi

AM_CONDITIONAL(BUILD_GTK_GL, test "x$have_gtkgl" = "xyes")

#----------------------------------------------------------------------

AC_SUBST(INSTALL_OPTIONAL_SCM)

GUILE_PROGS
GUILE_FLAGS

AC_MSG_CHECKING(whether we have at least Guile 1.6.4)
if $GUILE -c '(if (> (string->number (major-version)) 1) (exit 0))
              (if (< (string->number (major-version)) 1) (exit 1))
              (if (> (string->number (minor-version)) 6) (exit 0))
              (if (< (string->number (minor-version)) 6) (exit 1))
              (if (>= (string->number (micro-version)) 4) (exit 0))
	      (exit 1)'; then
  AC_MSG_RESULT(found)
else
  AC_MSG_ERROR([Your Guile is too old.  You need Guile at least 1.6.4]) 
fi

AC_CACHE_CHECK([for thread_create in GThreadFunctions],
	       ac_cv_struct_thread_create,
               [saved_CFLAGS="$CFLAGS"
		saved_LIBS="$LIBS"
		CFLAGS="$CFLAGS $GTK_CFLAGS"
		LIBS="$LIB $GTK_LIBS"
		AC_TRY_COMPILE([#include <glib.h>],
                               [GThreadFunctions g; g.thread_create;],
			       ac_cv_struct_thread_create=yes,
			       ac_cv_struct_thread_create=no)
		CFLAGS="$saved_CFLAGS"
		LIBS="$saved_LIBS"])
if test $ac_cv_struct_thread_create = yes; then
  AC_DEFINE(HAVE_THREAD_CREATE, 1,
            [Define when you have the thread_create slot in GThreadFunctions])
fi

saved_LIBS="$LIBS"
saved_CPPFLAGS="$CPPFLAGS"

# Check for bleeding edge Guile functions
LIBS="$GUILE_LDFLAGS $LIBS"
CPPFLAGS="$GUILE_CFLAGS $CPPFLAGS"

### BEGIN compatibility checks ###

# Reasons for testing:
#     g_object_ref_sink - not in glib 2.6
#     mallinfo - not available on ancient systems
#
AC_CHECK_FUNCS(g_object_ref_sink mallinfo scm_c_issue_deprecation_warning)

AC_CHECK_HEADERS(malloc.h libguile/deprecation.h)

AC_CACHE_CHECK(for two argument scm_mutex_init,
ac_cv_func_scm_mutex_init_two_args,
AC_TRY_COMPILE([#include <libguile.h>],[scm_mutex_init (NULL, NULL);],
ac_cv_func_scm_mutex_init_two_args=yes,
ac_cv_func_scm_mutex_init_two_args=no))
if test "$ac_cv_func_scm_mutex_init_two_args" = "yes"; then
  AC_DEFINE(SCM_MUTEX_INIT_TWO_ARGS, 1,
	    [define if scm_mutex_init takes two arguments])
fi

### END compatibility checks ###

# Check for cutting edge Gtk functions
LIBS="$GTK_LIBS $saved_LIBS"
AC_CHECK_FUNCS(gtk_signal_set_class_function_full)

LIBS="$saved_LIBS"

AC_CONFIG_FILES(
	  Makefile
	  gdk-pixbuf/Makefile
	  glade/Makefile
          gtk/Makefile
          gtk-2.0/Makefile
	  gtk-gl/Makefile
          examples/Makefile
          tests/Makefile
          gtk-2.0/config.scm)

AC_OUTPUT

echo "

Configuration:

  GdkPixbuf support:	${gdk_pixbuf_msg}
  libGlade support:	${glade_msg}
  GtkGL support:	${gtk_gl_msg}

"
