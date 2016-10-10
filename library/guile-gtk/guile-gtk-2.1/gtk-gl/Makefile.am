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

INCLUDES = -I$(top_srcdir) $(GTKGL_CFLAGS)
BUILD = guile -l $(top_builddir)/gtk-2.0/config.scm -s $(top_srcdir)/build-guile-gtk-2.0

if BUILD_GTK_GL
  defs_DATA = gdk-gl.defs gtk-gl-area.defs
  module_DATA = gdk-gl.scm gtk-gl-area.scm
  lib_LTLIBRARIES = libguilegtkgl-2.0.la
endif

# guile-gtk  --version-info
#    2.0        0:0:0
#    2.1        0:1:0      same
#
libguilegtkgl_2_0_la_SOURCES = gdk-gl-glue.c gtk-gl-area-glue.c \
			       gdk-gl-support.c gdk-gl-support.h
libguilegtkgl_2_0_la_LDFLAGS = --version-info 0:1:0 -export-dynamic
libguilegtkgl_2_0_la_LIBADD = ../libguilegtk-2.0.la $(GTKGL_LIBS)

BUILT_SOURCES = gdk-gl-glue.c gtk-gl-area-glue.c
CLEANFILES = gdk-gl-glue.c gtk-gl-area-glue.c

gdk-gl-glue.c: gdk-gl.defs gdk-gl-types.scm $(top_srcdir)/build-guile-gtk-2.0
	$(BUILD) -I $(top_srcdir) -I $(srcdir) glue gdk-gl.defs >tmpd && mv tmpd $@

gtk-gl-area-glue.c: gtk-gl-area.defs gdk-gl-types.scm $(top_srcdir)/build-guile-gtk-2.0
	$(BUILD) -I $(top_srcdir) -I $(srcdir) glue gtk-gl-area.defs >tmpd && mv tmpd $@

EXTRA_DIST = gdk-gl.defs gdk-gl.scm gtk-gl-area.defs gtk-gl-area.scm gdk-gl-types.scm

# Also link a small program that merely tests whether all external
# references of libguilegtkgl-2.0 are satisfied.
#
if BUILD_GTK_GL
  check_PROGRAMS = linktest
endif
linktest_SOURCES = linktest.c
linktest_LDADD = libguilegtkgl-2.0.la ../libguilegtk-2.0.la $(GUILE_LDFLAGS)
