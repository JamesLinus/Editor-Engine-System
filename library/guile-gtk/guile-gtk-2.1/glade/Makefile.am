# -*- Makefile -*-

# Copyright (C) 2002, 2006, 2007 Free Software Foundation, Inc.
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

INCLUDES = -I$(top_srcdir) $(LIBGLADE_CFLAGS)
BUILD = guile -l $(top_builddir)/gtk-2.0/config.scm -s $(top_srcdir)/build-guile-gtk-2.0

if BUILD_GLADE
  defs_DATA = glade.defs
  module_DATA = glade.scm
  lib_LTLIBRARIES = libguileglade-2.0.la
endif

# guile-gtk  --version-info
#    2.0        0:0:0
#    2.1        0:1:0      same
#
libguileglade_2_0_la_SOURCES = glade-glue.c glade-support.c glade-support.h 
libguileglade_2_0_la_LDFLAGS = --version-info 0:1:0 -export-dynamic
libguileglade_2_0_la_LIBADD = ../libguilegtk-2.0.la $(LIBGLADE_LIBS)

BUILT_SOURCES = glade-glue.c
CLEANFILES = glade-glue.c glade-support.x

glade-glue.c: glade.defs $(top_srcdir)/build-guile-gtk-2.0
	$(BUILD) -I $(top_srcdir) -I $(srcdir) glue glade.defs >tmpd && mv tmpd $@

SUFFIXES = .x
.c.x:
	guile-snarf -o $@ $< $(DEFS) $(INCLUDES) $(CPPFLAGS) $(CFLAGS)

EXTRA_DIST = glade.defs glade.scm

# Also link a small program that merely tests whether all external
# references of libguileglade-2.0 are satisfied.
#
if BUILD_GLADE
  check_PROGRAMS = linktest
endif
linktest_SOURCES = linktest.c
linktest_LDADD = libguileglade-2.0.la ../libguilegtk-2.0.la $(GUILE_LDFLAGS)
