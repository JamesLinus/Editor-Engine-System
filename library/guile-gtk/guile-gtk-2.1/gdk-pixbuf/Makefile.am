# -*- Makefile -*-

# Copyright (C) 2002, 2003, 2006, 2007 Free Software Foundation, Inc.
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

moduledir = $(datadir)/guile/gtk-2.0
defsdir = $(datadir)/guile-gtk

INCLUDES = -I$(top_srcdir) $(GDK_PIXBUF_CFLAGS)
BUILD = guile -l $(top_builddir)/gtk-2.0/config.scm -s $(top_srcdir)/build-guile-gtk-2.0

if BUILD_GDK_PIXBUF
  defs_DATA = gdk-pixbuf.defs
  module_DATA = gdk-pixbuf.scm
  lib_LTLIBRARIES = libguilegdkpixbuf-2.0.la
endif

# guile-gtk  --version-info
#    2.0        0:0:0
#    2.1        0:1:0      same
#
libguilegdkpixbuf_2_0_la_SOURCES = gdk-pixbuf-glue.c \
  gdk-pixbuf-support.c gdk-pixbuf-support.h
libguilegdkpixbuf_2_0_la_LDFLAGS = --version-info 0:1:0 -export-dynamic
libguilegdkpixbuf_2_0_la_LIBADD = ../libguilegtk-2.0.la $(GDK_PIXBUF_LIBS)

BUILT_SOURCES = gdk-pixbuf-glue.c
CLEANFILES = gdk-pixbuf-glue.c

gdk-pixbuf-glue.c: gdk-pixbuf.defs $(top_srcdir)/build-guile-gtk-2.0
	$(BUILD) -I $(top_srcdir) -I $(srcdir) glue gdk-pixbuf.defs >tmpd && mv tmpd $@

EXTRA_DIST = gdk-pixbuf.defs gdk-pixbuf.scm

# Also link a small program that merely tests whether all external
# references of libguilegdkpixbuf-2.0 are satisfied.
#
if BUILD_GDK_PIXBUF
  check_PROGRAMS = linktest
endif
linktest_SOURCES = linktest.c
linktest_LDADD = libguilegdkpixbuf-2.0.la ../libguilegtk-2.0.la \
  $(GUILE_LDFLAGS)
