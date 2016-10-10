# -*- Makefile -*-
#
# Copyright (C) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2006, 2007 Free
# Software Foundation, Inc.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

SUBDIRS = . gdk-pixbuf glade gtk-gl gtk gtk-2.0 examples tests

INCLUDES = -I$(srcdir) -I. $(GTK_CFLAGS) $(GUILE_CFLAGS)

ACLOCAL = ./guile-gtk-aclocal.sh

bin_SCRIPTS     = build-guile-gtk-2.0
lib_LTLIBRARIES = libguilegtk-2.0.la

libguilegtk_2_0_la_SOURCES = guile-gtk.c \
	                     guile-gtk.h \
                             guile-compat.c \
			     guile-gtk-compat.h \
                             gtk-compat.c \
			     gtk-support.c \
			     gdk-support.c \
                             gtk-threads.c \
                             gtk-threads.h \
	                     gdk-glue.c \
			     gtk-glue.c

# Crib notes: the version tuple CURRENT:REVISION:AGE is updated for each
# release as follows.
# - If any incompatibility, then CURRENT+1:0:0.
# - Otherwise if interfaces added, then bump to CURRENT+1:0:AGE+1
#   (keeps same soname, increments the minor version number).
# - Otherwise bump to CURRENT:REVISION+1:AGE
#   (keeps same soname, increments the sub-minor version number).
# Remember to do this for the gdk-pixbuf, gtk-gl and glade subdirs too.
#
# guile-gtk  --version-info
#    2.0        0:0:0
#    2.1        1:0:1      misc additions
#
libguilegtk_2_0_la_LDFLAGS = -version-info 1:0:1 -export-dynamic $(GUILE_LDFLAGS)
libguilegtk_2_0_la_LIBADD = $(GTK_LIBS)

include_HEADERS = guile-gtk.h guile-gtk-compat.h

info_TEXINFOS = guile-gtk.texi
dist_man_MANS = build-guile-gtk.1

# Also link a small program that merely tests whether all external
# references of libguilegtk-2.0 are satisfied.
#
check_PROGRAMS = linktest

linktest_SOURCES = linktest.c
linktest_LDADD = libguilegtk-2.0.la $(GUILE_LDFLAGS)

gtkdefs = gtk-2.0.defs
gdkdefs = gdk-2.0.defs

defsdir = $(datadir)/guile-gtk/
defs_DATA = $(gtkdefs) $(gdkdefs) 

install-exec-local:
	rm -f $(DESTDIR)$(bindir)/build-guile-gtk
	rm -f $(DESTDIR)$(bindir)/guile-gtk
	ln $(DESTDIR)$(bindir)/build-guile-gtk-2.0 $(DESTDIR)$(bindir)/build-guile-gtk

BUILT_SOURCES = gdk-glue.c gtk-glue.c guile-gtk.x gtk-support.x gdk-support.x

CLEANFILES = gdk-glue.c gtk-glue.c guile-gtk.x gtk-support.x gdk-support.x

SUFFIXES = .x
.c.x:
	guile-snarf -o $@ $< $(DEFS) $(INCLUDES) $(CPPFLAGS) $(CFLAGS)

BUILD = guile -l ./gtk-2.0/config.scm -s $(srcdir)/build-guile-gtk-2.0

gtk-glue.c: $(gtkdefs) $(gdkdefs) build-guile-gtk-2.0
	$(BUILD) -I $(srcdir) glue $(gtkdefs) >tmpt && mv tmpt $@

gdk-glue.c: $(gdkdefs) build-guile-gtk-2.0
	$(BUILD) -I $(srcdir) glue $(gdkdefs) >tmpd && mv tmpd $@

EXTRA_DIST = gdk-2.0.defs gtk-2.0.defs build-guile-gtk-2.0	\
	     guile-gtk-aclocal.sh


# scm_c_define's explicitly in gtk-support.c and gdk-support.c
#
AM_ETAGSFLAGS = \
  --regex='/[ \t]*scm_c_define[ \t]*([ \t]*"\([^"]+\)"/\1/'
